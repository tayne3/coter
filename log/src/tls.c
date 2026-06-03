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
#include "coter/log/handler/base.h"
#include "coter/log/log.h"
#include "coter/sync/atomic.h"
#include "coter/sync/mutex.h"
#include "coter/thread/once.h"
#include "coter/thread/thread.h"
#include "coter/thread/tls.h"
#include "log_internal.h"

#if defined(CT_OS_WIN)
#include <windows.h>
#elif defined(CT_OS_LINUX)
#include <sys/syscall.h>
#endif

// -------------------------[GLOBAL STATE & TYPES]-------------------------

struct ct_log_tls {
    ct_log_block_t* current_block;   // Current accumulating block
    ct_logger_t*    last_logger;     // Belonging logger
    ct_time64_t     first_log_time;  // First log time of this block
    uint32_t        tid;             // Cached thread ID

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

// Platform helpers
static uint32_t tls__gettid(void);

// Core log operations
static ct_log_tls_t* ct_log_tls_get(void);
static void          tls__submit_block(ct_log_tls_t* self);
static int           tls__ensure_block(ct_log_tls_t* self, ct_logger_t* logger);

// -------------------------[STATIC IMPLEMENTATION]-------------------------

static ct_log_tls_t* ct_log_tls_get(void) {
    ct_once_exec(&g_log_tls_key_once, tls__thread_create_key);
    ct_once_exec(&g_log_tls_init_once, tls__init_registry);

    ct_log_tls_t* self = ct_tls_get(g_log_tls_key);
    if (!self) {
        self = calloc(1, sizeof(ct_log_tls_t));
        if (!self) { return NULL; }

        self->tid = tls__gettid();
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

static uint32_t tls__gettid(void) {
#ifdef CT_OS_WIN
    return (uint32_t)GetCurrentThreadId();
#elif defined(CT_OS_LINUX)
    return (uint32_t)syscall(SYS_gettid);
#elif defined(CT_OS_DARWIN)
    uint64_t tid = 0;
    pthread_threadid_np(NULL, &tid);
    return (uint32_t)tid;
#else
    return 0;
#endif
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

// -------------------------[CT_LOG_SUBMIT CORE]-------------------------

static void ct_log_submit__impl(ct_logger_t* logger, int level, const char* file, int line, const char* payload,
                                size_t payload_len) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return; }
    if (!logger) { logger = ct_logger_default(); }
    if (!logger || !payload || payload_len == 0) { return; }
    int logger_state = ct_atomic_int_load(&logger->state);
    if (logger_state >= (int)CT_LOGGER_STATE_DESTROYING) { return; }

    ct_log_dispatcher_t* dispatcher = NULL;
    if (logger_state == (int)CT_LOGGER_STATE_RUNNING && ct_log_system_ensure() == 0) {
        dispatcher = ct_log_get_dispatcher();
    }

    // Async path
    ct_log_tls_t* self = ct_log_tls_get();
    if (!self) { return; }

    ct_time64_t now_wall = ct_gettimeofday_us() / 1000;

    // Sync fallback
    if (logger_state == (int)CT_LOGGER_STATE_INIT || !dispatcher) {
        ct_log_record_t record = {
            .time  = now_wall,
            .tid   = self->tid,
            .file  = file,
            .line  = line,
            .data  = payload,
            .size  = payload_len,
            .level = level,
        };
        ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
            if (handler->vtable && handler->vtable->puts) {
                handler->vtable->puts(handler, &record, 1);
                if (handler->vtable->flush) handler->vtable->flush(handler);
            }
        }
        return;
    }

    ct_mutex_lock(&self->lock);
    if (ct_atomic_int_load(&logger->state) != (int)CT_LOGGER_STATE_RUNNING) {
        ct_mutex_unlock(&self->lock);
        return;
    }

    if (tls__ensure_block(self, logger) != 0) {
        ct_mutex_unlock(&self->lock);
        return;
    }

    ct_log_block_t* block = self->current_block;

    uint32_t header_offset = block->used;
    uint32_t available     = block->capacity - block->used;

    // Reserve header space
    if (available < sizeof(ct_log_record_header_t)) {
        tls__submit_block(self);
        if (tls__ensure_block(self, logger) != 0) {
            ct_mutex_unlock(&self->lock);
            return;
        }
        block         = self->current_block;
        header_offset = block->used;
        available     = block->capacity - block->used;
    }
    block->used += (uint32_t)sizeof(ct_log_record_header_t);
    available -= (uint32_t)sizeof(ct_log_record_header_t);

    // Copy payload directly into block
    uint32_t copy_len = (uint32_t)payload_len < available ? (uint32_t)payload_len : available;

    if (copy_len > 0) { memcpy(block->data + block->used, payload, copy_len); }

    // Handle truncation
    if ((uint32_t)payload_len > copy_len) {
        if (copy_len >= 3) {
            block->data[block->used + copy_len - 3] = '.';
            block->data[block->used + copy_len - 2] = '.';
            block->data[block->used + copy_len - 1] = '.';
        }
        ct_log_add_dropped_bytes((uint32_t)payload_len - copy_len);
    }

    block->used += copy_len;

    // Fill record header with raw metadata
    ct_log_record_header_t* header = (ct_log_record_header_t*)(block->data + header_offset);
    header->time                   = now_wall;
    header->level                  = level;
    header->tid                    = self->tid;
    header->line                   = line;
    header->file                   = file;
    header->size                   = block->used - header_offset - (uint32_t)sizeof(ct_log_record_header_t);
    ++block->rec_count;

    // Auto-submit threshold
    ct_time64_t now_up = ct_getuptime_ms();
    if (block->used > block->capacity * 7 / 8 || (now_up - self->first_log_time >= 100)) { tls__submit_block(self); }

    ct_mutex_unlock(&self->lock);
}

// -------------------------[PUBLIC API IMPLEMENTATION]-------------------------

void ct_log_submit_fmt(ct_logger_t* logger, int level, const char* file, int line, const char* fmt, ...) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return; }
    if (!logger) { logger = ct_logger_default(); }
    if (!logger || !fmt) { return; }
    if (level < ct_atomic_int_load(&logger->level)) { return; }
    const int logger_state = ct_atomic_int_load(&logger->state);
    if (logger_state >= (int)CT_LOGGER_STATE_DESTROYING) { return; }

    ct_log_dispatcher_t* dispatcher = NULL;
    if (logger_state == (int)CT_LOGGER_STATE_RUNNING && ct_log_system_ensure() == 0) {
        dispatcher = ct_log_get_dispatcher();
    }

    ct_log_tls_t* self = ct_log_tls_get();
    if (!self) { return; }

    // Async path: try direct formatting into block
    do {
        if (!dispatcher) { break; }

        ct_time64_t now_wall = ct_gettimeofday_us() / 1000;

        if (logger_state == (int)CT_LOGGER_STATE_INIT) { break; }

        ct_mutex_lock(&self->lock);
        if (ct_atomic_int_load(&logger->state) != (int)CT_LOGGER_STATE_RUNNING) {
            ct_mutex_unlock(&self->lock);
            return;
        }
        if (tls__ensure_block(self, logger) != 0) {
            ct_mutex_unlock(&self->lock);
            break;
        }

        ct_log_block_t* block = self->current_block;

        // Check for minimum space (header + small safety margin)
        if (block->capacity - block->used < sizeof(ct_log_record_header_t) + 32) {
            tls__submit_block(self);
            if (tls__ensure_block(self, logger) != 0) {
                ct_mutex_unlock(&self->lock);
                break;
            }
            block = self->current_block;
        }

        uint32_t header_offset = block->used;
        block->used += (uint32_t)sizeof(ct_log_record_header_t);

        va_list args;
        va_start(args, fmt);
        int available = (int)(block->capacity - block->used);
        int len       = vsnprintf(block->data + block->used, (size_t)available, fmt, args);
        va_end(args);

        if (len < 0) {
            block->used = header_offset;
            ct_mutex_unlock(&self->lock);
            return;
        }

        uint32_t written = (uint32_t)len;
        if (written >= (uint32_t)available) {
            written = (uint32_t)available - 1;
            if (written >= 3) { memcpy(block->data + block->used + written - 3, "...", 3); }
            ct_log_add_dropped_bytes((uint32_t)len - written);
        }
        block->used += written;

        ct_log_record_header_t* header = (ct_log_record_header_t*)(block->data + header_offset);
        header->time                   = now_wall;
        header->level                  = level;
        header->tid                    = self->tid;
        header->line                   = line;
        header->file                   = file;
        header->size                   = written;
        ++block->rec_count;

        ct_time64_t now_up = ct_getuptime_ms();
        if (block->used > block->capacity * 7 / 8 || (now_up - self->first_log_time >= 100)) {
            tls__submit_block(self);
        }
        ct_mutex_unlock(&self->lock);
        return;
    } while (0);

    // fallback
    {
        char    buf[1024];
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        if (len <= 0) { return; }
        ct_log_submit_payload(logger, level, file, line, buf, (size_t)len);
    }
}

void ct_log_submit_payload(ct_logger_t* logger, int level, const char* file, int line, const char* payload,
                           size_t payload_len) {
    ct_log_submit__impl(logger, level, file, line, payload, payload_len);
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
