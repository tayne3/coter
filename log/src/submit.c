/**
 * @file submit.c
 * @brief Log submit path and thread local scratch buffer.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coter/core/platform.h"
#include "coter/core/time.h"
#include "coter/log/logger.h"
#include "coter/sync/atomic.h"
#include "coter/thread/once.h"
#include "coter/thread/tls.h"
#include "log_internal.h"

#if defined(CT_OS_WIN)
#include <windows.h>
#elif defined(CT_OS_LINUX)
#include <sys/syscall.h>
#endif

typedef struct ct_log_tls {
    uint32_t tid;
    char     payload[CT_LOG_RECORD_MAX];
} ct_log_tls_t;

static ct_tls_key_t g_log_tls_key;
static ct_once_t    g_log_tls_key_once = CT_ONCE_INIT;

static void          tls__thread_destroy(void* ptr);
static void          tls__thread_create_key(void);
static uint32_t      tls__gettid(void);
static ct_log_tls_t* tls__get(void);
static size_t        tls__format_payload(char* buf, size_t cap, const char* fmt, va_list args);
static size_t        tls__copy_payload(char* buf, size_t cap, const char* payload, size_t payload_len);

static ct_log_tls_t* tls__get(void) {
    ct_once_exec(&g_log_tls_key_once, tls__thread_create_key);

    ct_log_tls_t* self = (ct_log_tls_t*)ct_tls_get(g_log_tls_key);
    if (!self) {
        self = (ct_log_tls_t*)calloc(1, sizeof(ct_log_tls_t));
        if (!self) { return NULL; }
        self->tid = tls__gettid();
        ct_tls_set(g_log_tls_key, self);
    }
    return self;
}

static void tls__thread_destroy(void* ptr) {
    free(ptr);
}

static void tls__thread_create_key(void) {
    ct_tls_create(&g_log_tls_key, tls__thread_destroy);
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

static size_t tls__format_payload(char* buf, size_t cap, const char* fmt, va_list args) {
    if (!buf || cap == 0 || !fmt) { return 0; }

    int len = vsnprintf(buf, cap, fmt, args);
    if (len <= 0) { return 0; }

    size_t written = (size_t)len;
    if (written >= cap) {
        written = cap - 1;
        if (written >= 3) { memcpy(buf + written - 3, "...", 3); }
    }
    buf[written] = '\0';
    return written;
}

static size_t tls__copy_payload(char* buf, size_t cap, const char* payload, size_t payload_len) {
    if (!buf || cap == 0 || !payload || payload_len == 0) { return 0; }

    size_t written = payload_len;
    if (written >= cap) {
        written = cap - 1;
        if (written >= 3) {
            memcpy(buf, payload, written);
            memcpy(buf + written - 3, "...", 3);
        } else {
            memcpy(buf, payload, written);
        }
    } else {
        memcpy(buf, payload, written);
    }
    buf[written] = '\0';
    return written;
}

void ct_log_submit_fmt(ct_logger_t* logger, int level, const char* file, int line, const char* fmt, ...) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return; }
    if (!logger) { logger = ct_logger_default(); }
    if (!logger || !fmt) { return; }
    if (level < ct_atomic_int_load(&logger->level)) { return; }
    if (ct_logger_acquire_writer(logger) != 0) { return; }

    ct_log_tls_t* tls = tls__get();
    if (!tls) {
        ct_logger_release_writer(logger);
        return;
    }

    va_list args;
    va_start(args, fmt);
    size_t len = tls__format_payload(tls->payload, sizeof(tls->payload), fmt, args);
    va_end(args);

    if (len > 0) {
        ct_log_dispatcher_t* dispatcher = ct_log_get_dispatcher();
        if (dispatcher) {
            (void)ct_log_dispatcher_push_record(dispatcher, logger, level, file, line, tls->tid,
                                                ct_gettimeofday_us() / 1000, tls->payload, len);
        }
    }

    ct_logger_release_writer(logger);
}

void ct_log_submit_payload(ct_logger_t* logger, int level, const char* file, int line, const char* payload,
                           size_t payload_len) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return; }
    if (!logger) { logger = ct_logger_default(); }
    if (!logger || !payload || payload_len == 0) { return; }
    if (level < ct_atomic_int_load(&logger->level)) { return; }
    if (ct_logger_acquire_writer(logger) != 0) { return; }

    ct_log_tls_t* tls = tls__get();
    if (!tls) {
        ct_logger_release_writer(logger);
        return;
    }

    size_t len = tls__copy_payload(tls->payload, sizeof(tls->payload), payload, payload_len);
    if (len > 0) {
        ct_log_dispatcher_t* dispatcher = ct_log_get_dispatcher();
        if (dispatcher) {
            (void)ct_log_dispatcher_push_record(dispatcher, logger, level, file, line, tls->tid,
                                                ct_gettimeofday_us() / 1000, tls->payload, len);
        }
    }
    ct_logger_release_writer(logger);
}
