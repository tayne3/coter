/**
 * @file submit.c
 * @brief Log submit path and job snapshot builder.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coter/core/platform.h"
#include "coter/core/time.h"
#include "coter/log/logger.h"
#include "coter/thread/thread.h"
#include "log_internal.h"

#if defined(CT_OS_WIN)
#include <windows.h>
#elif defined(CT_OS_LINUX)
#include <sys/syscall.h>
#elif defined(CT_OS_DARWIN)
#include <pthread.h>
#endif

static uint32_t submit__gettid(void);
static void     submit__init_job(ct_log_job_t* job, ct_logger_t* logger, int level, const char* file, int line);
static size_t   submit__format_payload(ct_log_record_job_t* record_job, const char* fmt, va_list args);
static size_t   submit__copy_payload(ct_log_record_job_t* record_job, const char* payload, size_t payload_len);

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

static void submit__init_job(ct_log_job_t* job, ct_logger_t* logger, int level, const char* file, int line) {
    ct_log_record_job_t* record_job = &job->record;

    job->type              = CT_LOG_JOB_RECORD;
    job->logger            = logger;
    record_job->time       = ct_gettimeofday_us() / 1000;
    record_job->tid        = submit__gettid();
    record_job->file       = file;
    record_job->line       = line;
    record_job->level      = level;
    record_job->size       = 0;
    record_job->payload[0] = '\0';
}

static size_t submit__format_payload(ct_log_record_job_t* record_job, const char* fmt, va_list args) {
    if (!record_job || !fmt) { return 0; }

    char*        buf = record_job->payload;
    const size_t cap = sizeof(record_job->payload);

    int len = vsnprintf(buf, cap, fmt, args);
    if (len <= 0) { return 0; }

    size_t written = (size_t)len;
    if (written >= cap) {
        written = cap - 1;
        if (written >= 3) { memcpy(buf + written - 3, "...", 3); }
    }
    buf[written]     = '\0';
    record_job->size = written;
    return written;
}

static size_t submit__copy_payload(ct_log_record_job_t* record_job, const char* payload, size_t payload_len) {
    if (!record_job || !payload || payload_len == 0) { return 0; }

    char*        buf = record_job->payload;
    const size_t cap = sizeof(record_job->payload);

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
    buf[written]     = '\0';
    record_job->size = written;
    return written;
}

void ct_log_submit_fmt(ct_logger_t* logger, int level, const char* file, int line, const char* fmt, ...) {
    if (ct_log_dispatcher_is_worker()) { return; }
    if (!CT_LOG_LEVEL_IS_VALID(level) || !fmt) { return; }
    if (!logger) { logger = ct_logger_get_default(); }
    if (!logger) { return; }
    if (level < ct_atomic_int_load(&logger->level)) { return; }
    if (ct_logger_acquire_writer(logger) != 0) { return; }

    ct_log_job_t job;
    memset(&job, 0, sizeof(job));
    submit__init_job(&job, logger, level, file, line);

    va_list args;
    va_start(args, fmt);
    size_t len = submit__format_payload(&job.record, fmt, args);
    va_end(args);

    if (len > 0) { (void)ct_log_dispatcher_submit(&job); }

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

    ct_log_job_t job;
    memset(&job, 0, sizeof(job));
    submit__init_job(&job, logger, level, file, line);

    size_t len = submit__copy_payload(&job.record, payload, payload_len);
    if (len > 0) { (void)ct_log_dispatcher_submit(&job); }

    ct_logger_release_writer(logger);
}
