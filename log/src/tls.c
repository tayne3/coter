/**
 * @file tls.c
 * @brief Log thread local storage cache.
 */
#include "coter/log/tls.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coter/container/list.h"
#include "coter/core/platform.h"
#include "coter/core/strings.h"
#include "coter/core/time.h"
#include "coter/log/handler.h"
#include "coter/log/log.h"
#include "coter/sync/atomic.h"
#include "coter/sync/mutex.h"
#include "coter/thread/once.h"
#include "coter/thread/thread.h"
#include "coter/thread/tls.h"
#include "log_internal.h"

#ifdef CT_OS_LINUX
#include <sys/syscall.h>
#endif

// -------------------------[GLOBAL STATE & TYPES]-------------------------

/**
 * @brief Log TLS cache
 */
struct ct_log_tls {
    ct_time_t cached_time_sec;  // Last access time (seconds)

    const char* last_file;        // Last accessed file path
    const char* filename;         // Last accessed file name
    int         filename_length;  // Last accessed file name length

    char tm_str[24];   // Cached time string
    char tid_str[19];  // Thread ID string

    ct_log_block_t* current_block;   // Current accumulating block
    ct_logger_t*    last_logger;     // Belonging logger
    ct_time64_t     first_log_time;  // First log time of this block

    ct_list_t  node;  // Node in mutex-protected registry
    ct_mutex_t lock;  // Protection lock for current_block
};

// TLS key
static ct_tls_key_t g_log_tls_key;
// TLS key initialization flag
static ct_once_t g_log_tls_key_once = CT_ONCE_INIT;

// TLS registry
static ct_list_t g_log_tls_registry;
// TLS registry lock
static ct_mutex_t g_log_tls_lock;
// TLS registry initialization flag
static ct_once_t g_log_tls_init_once = CT_ONCE_INIT;

// -------------------------[STATIC DECLARATION]-------------------------

// Thread lifecycle
static void tls__thread_destroy(void* ptr);
static void tls__thread_create_key(void);
static void tls__init_registry(void);

// Time & String formatting
static void        tls__fmt_digits_2(char** p, int value);
static void        tls__fmt_digits_3(char** p, int value);
static void        tls__fmt_digits_4(char** p, int value);
static void        tls__update_tmstr(ct_log_tls_t* self);
static void        tls__gettid_str(char* str, size_t max);
static inline void tls__update_filename(ct_log_tls_t* self, const char* file);

// Core log operations
static ct_log_tls_t* ct_log_tls_get(void);
static void          tls__submit_block(ct_log_tls_t* self);
static int           tls__ensure_block(ct_log_tls_t* self, ct_logger_t* logger);
static void tls__write_record_va(ct_log_tls_t* self, ct_logger_t* logger, int level, const char* file, int line,
                                 const char* prefix_fmt, const char* fmt, va_list args);
static void tls__write_sync(ct_logger_t* logger, int level, const char* file, int line, const char* prefix_fmt,
                            const char* fmt, va_list args);

// -------------------------[STATIC IMPLEMENTATION]-------------------------

static ct_log_tls_t* ct_log_tls_get(void) {
    ct_once_exec(&g_log_tls_key_once, tls__thread_create_key);
    ct_once_exec(&g_log_tls_init_once, tls__init_registry);

    ct_log_tls_t* self = ct_tls_get(g_log_tls_key);
    if (!self) {
        self = calloc(1, sizeof(ct_log_tls_t));
        if (!self) { return NULL; }
        tls__gettid_str(self->tid_str, sizeof(self->tid_str));

        ct_mutex_init(&self->lock);
        ct_list_init(&self->node);

        ct_mutex_lock(&g_log_tls_lock);
        ct_list_append(&g_log_tls_registry, &self->node);
        ct_mutex_unlock(&g_log_tls_lock);

        ct_tls_set(g_log_tls_key, self);
    }
    return self;
}

static void tls__thread_destroy(void* ptr) {
    ct_log_tls_t* self = (ct_log_tls_t*)ptr;
    if (!self) { return; }

    tls__submit_block(self);

    ct_mutex_lock(&g_log_tls_lock);
    ct_list_remove(&self->node);
    ct_mutex_unlock(&g_log_tls_lock);

    ct_mutex_destroy(&self->lock);
    free(self);
}

static void tls__thread_create_key(void) {
    ct_tls_create(&g_log_tls_key, tls__thread_destroy);
}

static void tls__init_registry(void) {
    ct_list_init(&g_log_tls_registry);
    ct_mutex_init(&g_log_tls_lock);
}

static void tls__fmt_digits_2(char** p, int value) {
    *(*p)++ = '0' + value / 10;
    *(*p)++ = '0' + value % 10;
}

static void tls__fmt_digits_3(char** p, int value) {
    *(*p)++ = '0' + value / 100;
    *(*p)++ = '0' + (value / 10) % 10;
    *(*p)++ = '0' + value % 10;
}

static void tls__fmt_digits_4(char** p, int value) {
    *(*p)++ = '0' + value / 1000;
    *(*p)++ = '0' + (value / 100) % 10;
    *(*p)++ = '0' + (value / 10) % 10;
    *(*p)++ = '0' + value % 10;
}

static void tls__update_tmstr(ct_log_tls_t* self) {
    const ct_time64_t now_us   = ct_gettimeofday_us();
    const ct_time_t   now_sec  = (ct_time_t)(now_us / INT64_C(1000000));
    const int         now_usec = (int)(now_us % INT64_C(1000000));

    if (self->cached_time_sec == now_sec) {
        char* p = &self->tm_str[20];
        tls__fmt_digits_3(&p, now_usec / 1000);
        return;
    }

    self->cached_time_sec = now_sec;
    struct tm tm;
    ct_localtime_r(&now_sec, &tm);

    char* p = self->tm_str;
    tls__fmt_digits_4(&p, tm.tm_year + 1900);
    *p++ = '-';
    tls__fmt_digits_2(&p, tm.tm_mon + 1);
    *p++ = '-';
    tls__fmt_digits_2(&p, tm.tm_mday);
    *p++ = ' ';
    tls__fmt_digits_2(&p, tm.tm_hour);
    *p++ = ':';
    tls__fmt_digits_2(&p, tm.tm_min);
    *p++ = ':';
    tls__fmt_digits_2(&p, tm.tm_sec);
    *p++ = '.';
    tls__fmt_digits_3(&p, now_usec / 1000);
}

static void tls__gettid_str(char* str, size_t max) {
#ifdef CT_OS_WIN
    const DWORD tid = GetCurrentThreadId();
#if ULONG_MAX == 0xFFFFFFFF
    snprintf(str, max, "0x%08lX", tid);
#else
    snprintf(str, max, "0x%016lX", tid);
#endif
#elif defined(CT_OS_LINUX)
    const long int tid = syscall(SYS_gettid);
#if ULONG_MAX == 0xFFFFFFFF
    snprintf(str, max, "0x%08lX", tid);
#else
    snprintf(str, max, "0x%016lX", tid);
#endif
#elif defined(CT_OS_DARWIN)
    uint64_t tid = 0;
    pthread_threadid_np(NULL, &tid);
    snprintf(str, max, "0x%016llX", tid);
#else
#error "Unsupported platform!"
#endif
}

static inline void tls__update_filename(ct_log_tls_t* self, const char* file) {
    if (self->last_file != file) {
        const char* _filename = file;
#ifdef CT_OS_WIN
        const char* slash     = strrchr(file, '/');
        const char* backslash = strrchr(file, '\\');
        if (slash || backslash) { _filename = (slash > backslash ? slash : backslash) + 1; }
#else
        const char* slash = strrchr(file, '/');
        if (slash) { _filename = slash + 1; }
#endif
        self->last_file       = file;
        self->filename_length = (int)strlen(_filename);
        self->filename        = _filename;
    }
}

static void tls__submit_block(ct_log_tls_t* self) {
    ct_log_dispatcher_t* dispatcher = ct_log_get_dispatcher();
    if (self->current_block && dispatcher) {
        ct_log_dispatcher_push_block(dispatcher, self->last_logger, self->current_block);
        self->current_block = NULL;
    }
}

static int tls__ensure_block(ct_log_tls_t* self, ct_logger_t* logger) {
    if (self->current_block && self->last_logger != logger) { tls__submit_block(self); }

    if (!self->current_block) {
        ct_log_dispatcher_t* dispatcher = ct_log_get_dispatcher();
        if (!dispatcher) return -1;

        self->current_block = ct_log_block_pool_acquire(ct_log_dispatcher_get_pool(dispatcher));
        if (!self->current_block) { return -1; }

        self->last_logger    = logger;
        self->first_log_time = ct_getuptime_ms();
    }
    return 0;
}

static void tls__write_record_va(ct_log_tls_t* self, ct_logger_t* logger, int level, const char* file, int line,
                                 const char* prefix_fmt, const char* fmt, va_list args) {
    if (tls__ensure_block(self, logger) != 0) return;

    ct_log_block_t* block    = self->current_block;
    ct_time64_t     now_wall = ct_gettimeofday_us() / 1000;
    ct_time64_t     now_up   = ct_getuptime_ms();

    uint32_t header_offset = block->used;
    uint32_t available     = block->capacity - block->used;

    // Check if we have enough space for just the header
    if (available < sizeof(ct_log_record_header_t)) {
        tls__submit_block(self);
        if (tls__ensure_block(self, logger) != 0) return;
        block         = self->current_block;
        header_offset = block->used;
        available     = block->capacity - block->used;
    }

    block->used += (uint32_t)sizeof(ct_log_record_header_t);
    available -= (uint32_t)sizeof(ct_log_record_header_t);

    int prefix_len = 0;
    if (prefix_fmt) {
        tls__update_tmstr(self);
        if (file) {
            tls__update_filename(self, file);
            prefix_len = snprintf(block->data + block->used, available, prefix_fmt, self->tm_str, self->tid_str,
                                  self->filename_length, self->filename, line);
        } else {
            prefix_len = snprintf(block->data + block->used, available, prefix_fmt, self->tm_str, self->tid_str);
        }
    }

    int     msg_len = 0;
    va_list args_copy;
    va_copy(args_copy, args);
    if (prefix_len >= 0 && (uint32_t)prefix_len < available) {
        msg_len = vsnprintf(block->data + block->used + prefix_len, available - prefix_len, fmt, args_copy);
    } else {
        msg_len = available + 1;  // Force overflow condition
    }
    va_end(args_copy);

    if (prefix_len < 0 || msg_len < 0 || (uint32_t)(prefix_len + msg_len) >= available) {
        block->used = header_offset;  // rollback
        tls__submit_block(self);

        if (tls__ensure_block(self, logger) == 0) {
            block         = self->current_block;
            header_offset = block->used;
            available     = block->capacity - block->used;

            if (available < sizeof(ct_log_record_header_t)) return;  // Should never happen on fresh block
            block->used += (uint32_t)sizeof(ct_log_record_header_t);
            available -= (uint32_t)sizeof(ct_log_record_header_t);

            if (prefix_fmt) {
                if (file) {
                    prefix_len = snprintf(block->data + block->used, available, prefix_fmt, self->tm_str, self->tid_str,
                                          self->filename_length, self->filename, line);
                } else {
                    prefix_len =
                        snprintf(block->data + block->used, available, prefix_fmt, self->tm_str, self->tid_str);
                }
            }

            va_list args_retry;
            va_copy(args_retry, args);
            if (prefix_len >= 0 && (uint32_t)prefix_len < available) {
                msg_len = vsnprintf(block->data + block->used + prefix_len, available - prefix_len, fmt, args_retry);
            }
            va_end(args_retry);

            // If it STILL overflows, we must truncate it. We cannot allocate an infinite block.
            if (prefix_len > 0 && (uint32_t)prefix_len < available) {
                block->used += (uint32_t)prefix_len;
                available -= (uint32_t)prefix_len;
            }
            if (msg_len > 0) {
                uint32_t written_msg = (uint32_t)msg_len < available ? (uint32_t)msg_len : available - 1;
                if ((uint32_t)msg_len >= available) {
                    if (written_msg >= 3) {
                        block->data[block->used + written_msg - 3] = '.';
                        block->data[block->used + written_msg - 2] = '.';
                        block->data[block->used + written_msg - 1] = '.';
                    }
                    ct_log_add_dropped_bytes((uint32_t)msg_len - written_msg);
                }
                block->used += written_msg;
            }
        } else {
            return;
        }
    } else {
        block->used += (uint32_t)(prefix_len + msg_len);
    }

    ct_log_record_header_t* header = (ct_log_record_header_t*)(block->data + header_offset);
    header->time                   = now_wall;
    header->level                  = level;
    header->size                   = block->used - header_offset - (uint32_t)sizeof(ct_log_record_header_t);
    block->rec_count++;

    // Check for threshold using monotonic clock
    if (block->used > block->capacity * 7 / 8 || (now_up - self->first_log_time >= 100)) { tls__submit_block(self); }
}

static void tls__write_sync(ct_logger_t* logger, int level, const char* file, int line, const char* prefix_fmt,
                            const char* fmt, va_list args) {
    char buf[4096];
    int  prefix_len = 0;
    if (prefix_fmt) {
        ct_log_tls_t* self = ct_log_tls_get();
        if (self) {
            ct_mutex_lock(&self->lock);
            tls__update_tmstr(self);
            if (file) {
                tls__update_filename(self, file);
                prefix_len = snprintf(buf, sizeof(buf), prefix_fmt, self->tm_str, self->tid_str, self->filename_length,
                                      self->filename, line);
            } else {
                prefix_len = snprintf(buf, sizeof(buf), prefix_fmt, self->tm_str, self->tid_str);
            }
            ct_mutex_unlock(&self->lock);
        }
    }

    if (prefix_len < 0) prefix_len = 0;
    if ((size_t)prefix_len >= sizeof(buf)) prefix_len = sizeof(buf) - 1;

    int msg_len = vsnprintf(buf + prefix_len, sizeof(buf) - prefix_len, fmt, args);

    if (msg_len > 0) {
        uint32_t final_msg_len = (uint32_t)msg_len;
        if (prefix_len + final_msg_len >= sizeof(buf)) {
            final_msg_len = sizeof(buf) - prefix_len - 1;
            if (final_msg_len >= 3) {
                buf[prefix_len + final_msg_len - 3] = '.';
                buf[prefix_len + final_msg_len - 2] = '.';
                buf[prefix_len + final_msg_len - 1] = '.';
            }
            ct_log_add_dropped_bytes((uint32_t)msg_len - final_msg_len);
        }

        ct_log_record_t record = {
            .time  = ct_gettimeofday_us() / 1000,
            .data  = buf,
            .size  = (size_t)(prefix_len + final_msg_len),
            .level = level,
        };
        ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
            if (handler->vtable && handler->vtable->write_batch) {
                handler->vtable->write_batch(handler, &record, 1);
                if (handler->vtable->flush) handler->vtable->flush(handler);
            }
        }
    }
}

// -------------------------[INTERNAL API IMPLEMENTATION]-------------------------

void ct_log_tls_foreach(void (*fn)(ct_log_tls_t* tc, void* arg, bool force), void* arg, bool force) {
    if (!fn) { return; }
    ct_once_exec(&g_log_tls_init_once, tls__init_registry);

    ct_mutex_lock(&g_log_tls_lock);
    ct_list_foreach_entry(tc, &g_log_tls_registry, ct_log_tls_t, node) {
        fn(tc, arg, force);
    }
    ct_mutex_unlock(&g_log_tls_lock);
}

void ct_log_tls_lock(ct_log_tls_t* self) {
    if (self) { ct_mutex_lock(&self->lock); }
}

void ct_log_tls_unlock(ct_log_tls_t* self) {
    if (self) { ct_mutex_unlock(&self->lock); }
}

int ct_log_tls_trylock(ct_log_tls_t* self) {
    return self ? ct_mutex_trylock(&self->lock) : -1;
}

void ct_log_tls_flush_pending(ct_log_tls_t* self) {
    if (!self) { return; }
    tls__submit_block(self);
}

// -------------------------[PUBLIC API IMPLEMENTATION]-------------------------

void ct_log_tls_output(ct_logger_t* logger, int level, const char* file, int line, const char* prefix_fmt,
                       const char* fmt, ...) {
    if (!logger || !fmt) { return; }
    int logger_state = ct_atomic_int_load(&logger->state);
    if (logger_state >= (int)CT_LOGGER_STATE_DESTROYING) { return; }

    ct_log_dispatcher_t* dispatcher = ct_log_get_dispatcher();

    // If dispatcher is dead or logger is unregistered (INIT),
    // we MUST bypass the async queue to prevent UAF or data loss. We fallback to
    // a synchronous, lock-protected, stack-allocated write directly to the handlers.
    if (logger_state == (int)CT_LOGGER_STATE_INIT || !dispatcher) {
        va_list args;
        va_start(args, fmt);
        tls__write_sync(logger, level, file, line, prefix_fmt, fmt, args);
        va_end(args);
        return;
    }

    ct_log_tls_t* self = ct_log_tls_get();
    if (!self) { return; }

    va_list args;
    va_start(args, fmt);
    ct_mutex_lock(&self->lock);
    tls__write_record_va(self, logger, level, file, line, prefix_fmt, fmt, args);
    ct_mutex_unlock(&self->lock);
    va_end(args);
}

void ct_log_tls_output_hex(ct_logger_t* logger, int level, const void* buf, size_t len) {
    if (!logger || !buf || len == 0) { return; }

    int logger_state = ct_atomic_int_load(&logger->state);
    if (logger_state >= (int)CT_LOGGER_STATE_DESTROYING) { return; }
    char           hex_buf[128];
    const uint8_t* src       = (const uint8_t*)buf;
    const char*    hex_table = "0123456789ABCDEF";

    for (size_t i = 0; i < len;) {
        size_t chunk = (len - i) > 32 ? 32 : (len - i);
        char*  dst   = hex_buf;
        for (size_t j = 0; j < chunk; ++j) {
            uint8_t b = src[i + j];
            *dst++    = hex_table[b >> 4];
            *dst++    = hex_table[b & 0x0F];
            if (i + j < len - 1) *dst++ = ' ';
        }
        *dst = '\0';
        ct_log_tls_output(logger, level, NULL, 0, NULL, "%s", hex_buf);
        i += chunk;
    }
}
