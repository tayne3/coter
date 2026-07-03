/**
 * @file submit.c
 * @brief Log submit path — 路径 C：直接调用 handler，无全局 dispatcher。
 *
 * 日志提交流程：
 *   1. 检查 logger 状态 + level 过滤
 *   2. 构造 ct_log_record_t（栈上，含 payload 拷贝）
 *   3. 遍历 logger->handlers 调用每个 handler->write()
 *   4. 每个 handler 自行保证并发安全（Phase 0 已完成 console/file 加锁）
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coter/core/time.h"
#include "coter/log/logger.h"
#include "internal.h"

#if defined(CT_OS_WIN)
#include <windows.h>
#elif defined(CT_OS_LINUX)
#include <sys/syscall.h>
#elif defined(CT_OS_DARWIN)
#include <pthread.h>
#endif

static uint32_t submit__gettid(void);
static void     submit__dispatch(ct_logger_t* logger, const ct_log_record_t* record);
static size_t   submit__format_payload(char* buf, size_t cap, const char* fmt, va_list args);
static size_t   submit__copy_payload(char* buf, size_t cap, const char* payload, size_t payload_len);

static uint32_t submit__gettid(void) {
#ifdef CT_OS_WIN
    return (uint32_t)GetCurrentThreadId();
#elif defined(CT_OS_LINUX)
    return (uint32_t)syscall(SYS_gettid);
#elif defined(CT_OS_DARWIN)
    uint64_t tid = 0;
    pthread_threadid_np(NULL, &tid);
    return (uint32_t)tid;
#else
    return (uint32_t)ct_thread_current_id();
#endif
}

/* 直接遍历 handlers 调用 write()，无队列、无跨线程传递 */
static void submit__dispatch(ct_logger_t* logger, const ct_log_record_t* record) {
    /* 防止同步 handler 内部再次调用日志系统（如 CT_LOGGER_TRACE 或 ct_logger_close）
     * 导致无限递归或死锁。在此期间将当前线程标记为内部 worker。 */
    ct_log_register_worker();

    ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
        if (handler->vtable && handler->vtable->write) { handler->vtable->write(handler, record); }
    }

    ct_log_unregister_worker();
}

static size_t submit__format_payload(char* buf, size_t cap, const char* fmt, va_list args) {
    if (!buf || !fmt || cap == 0) { return 0; }

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

static size_t submit__copy_payload(char* buf, size_t cap, const char* payload, size_t payload_len) {
    if (!buf || !payload || payload_len == 0 || cap == 0) { return 0; }

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
    if (ct_log_dispatcher_is_worker()) { return; }
    if (!CT_LOG_LEVEL_IS_VALID(level) || !fmt) { return; }
    if (!logger) { logger = ct_logger_get_default(); }
    if (!logger) { return; }
    if (level < ct_atomic_int_load(&logger->level)) { return; }
    if (ct_logger_acquire_writer(logger) != 0) { return; }

    /* payload 和 record 均在栈上构造，无堆分配 */
    char payload[CT_LOG_RECORD_MAX];

    va_list args;
    va_start(args, fmt);
    size_t len = submit__format_payload(payload, sizeof(payload), fmt, args);
    va_end(args);

    if (len > 0) {
        ct_log_record_t record;
        record.time  = ct_gettimeofday_us() / 1000;
        record.tid   = submit__gettid();
        record.file  = file;
        record.line  = line;
        record.level = level;
        record.size  = len;
        record.data  = payload;

        submit__dispatch(logger, &record);
    }

    ct_logger_release_writer(logger);
}

void ct_log_submit_payload(ct_logger_t* logger, int level, const char* file, int line, const char* payload,
                           size_t payload_len) {
    if (ct_log_dispatcher_is_worker()) { return; }
    if (!CT_LOG_LEVEL_IS_VALID(level) || !payload || payload_len == 0) { return; }
    if (!logger) { logger = ct_logger_get_default(); }
    if (!logger) { return; }
    if (level < ct_atomic_int_load(&logger->level)) { return; }
    if (ct_logger_acquire_writer(logger) != 0) { return; }

    char   buf[CT_LOG_RECORD_MAX];
    size_t len = submit__copy_payload(buf, sizeof(buf), payload, payload_len);

    if (len > 0) {
        ct_log_record_t record;
        record.time  = ct_gettimeofday_us() / 1000;
        record.tid   = submit__gettid();
        record.file  = file;
        record.line  = line;
        record.level = level;
        record.size  = len;
        record.data  = buf;

        submit__dispatch(logger, &record);
    }

    ct_logger_release_writer(logger);
}
