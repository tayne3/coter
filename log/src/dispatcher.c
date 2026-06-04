/**
 * @file dispatcher.c
 * @brief Internal log dispatcher.
 */
#include <stdlib.h>
#include <string.h>

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/msgqueue.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"
#include "log_internal.h"

typedef struct ct_log_job {
    ct_logger_t*    logger;
    ct_log_record_t record;
    char            payload[CT_LOG_RECORD_MAX];
} ct_log_job_t;

struct ct_log_dispatcher {
    ct_thread_t     thread;
    ct_msgqueue_t   queue;
    void*           q_buf;
    ct_atomic_int_t running;
    ct_atomic_int_t pending_jobs;
    ct_atomic_int_t high_watermark;
    ct_mutex_t      flush_mutex;
    ct_cond_t       flush_cond;
};

static int  dispatcher__routine(void* arg);
static void dispatcher__update_high_watermark(ct_log_dispatcher_t* self, int value);
static void dispatcher__finish_global_job(ct_log_dispatcher_t* self);
static void dispatcher__copy_payload(ct_log_job_t* job, const char* payload, size_t payload_len);

static void dispatcher__update_high_watermark(ct_log_dispatcher_t* self, int value) {
    int high = ct_atomic_int_load(&self->high_watermark);
    while (value > high) {
        int expected = high;
        if (ct_atomic_int_compare_exchange(&self->high_watermark, &expected, value)) { return; }
        high = expected;
    }
}

static void dispatcher__finish_global_job(ct_log_dispatcher_t* self) {
    ct_mutex_lock(&self->flush_mutex);
    if (ct_atomic_int_sub(&self->pending_jobs, 1) == 1) { ct_cond_broadcast(&self->flush_cond); }
    ct_mutex_unlock(&self->flush_mutex);
}

static void dispatcher__copy_payload(ct_log_job_t* job, const char* payload, size_t payload_len) {
    size_t copy_len = payload_len;
    if (copy_len >= CT_LOG_RECORD_MAX) {
        copy_len = CT_LOG_RECORD_MAX - 1;
        if (copy_len >= 3) {
            memcpy(job->payload, payload, copy_len);
            job->payload[copy_len - 3] = '.';
            job->payload[copy_len - 2] = '.';
            job->payload[copy_len - 1] = '.';
        } else {
            memcpy(job->payload, payload, copy_len);
        }
    } else {
        memcpy(job->payload, payload, copy_len);
    }
    job->payload[copy_len] = '\0';
    job->record.size       = copy_len;
}

static int dispatcher__routine(void* arg) {
    ct_log_dispatcher_t* self = (ct_log_dispatcher_t*)arg;
    ct_log_job_t         job;

    while (ct_atomic_int_load(&self->running) || ct_atomic_int_load(&self->pending_jobs) > 0) {
        if (ct_msgqueue_pop_for(&self->queue, &job, 100) != 0) { continue; }

        job.record.data = job.payload;
        if (job.logger && job.record.size > 0) {
            ct_list_foreach_entry(handler, &job.logger->handlers, ct_log_handler_t, node) {
                if (handler->vtable && handler->vtable->puts) { handler->vtable->puts(handler, &job.record, 1); }
            }
        }

        if (job.logger) { ct_logger_finish_pending_job(job.logger); }
        dispatcher__finish_global_job(self);
    }
    return 0;
}

ct_log_dispatcher_t* ct_log_dispatcher_create(void) {
    ct_log_dispatcher_t* self = (ct_log_dispatcher_t*)calloc(1, sizeof(ct_log_dispatcher_t));
    if (!self) { return NULL; }

    self->q_buf = malloc(sizeof(ct_log_job_t) * CT_LOG_QUEUE_SIZE);
    if (!self->q_buf) {
        free(self);
        return NULL;
    }
    ct_msgqueue_init(&self->queue, self->q_buf, sizeof(ct_log_job_t), CT_LOG_QUEUE_SIZE);

    self->running        = CT_ATOMIC_VAR_INIT(1);
    self->pending_jobs   = CT_ATOMIC_VAR_INIT(0);
    self->high_watermark = CT_ATOMIC_VAR_INIT(0);
    ct_mutex_init(&self->flush_mutex);
    ct_cond_init(&self->flush_cond);

    if (ct_thread_create(&self->thread, NULL, dispatcher__routine, self) != 0) {
        ct_mutex_destroy(&self->flush_mutex);
        ct_cond_destroy(&self->flush_cond);
        ct_msgqueue_destroy(&self->queue);
        free(self->q_buf);
        free(self);
        return NULL;
    }

    return self;
}

int ct_log_dispatcher_push_record(ct_log_dispatcher_t* self, ct_logger_t* logger, int level, const char* file, int line,
                                  uint32_t tid, ct_time64_t time, const char* payload, size_t payload_len) {
    if (!self || !logger || !payload || payload_len == 0 || !ct_atomic_int_load(&self->running)) { return -1; }

    ct_log_job_t job;
    memset(&job, 0, sizeof(job));
    job.logger       = logger;
    job.record.time  = time;
    job.record.tid   = tid;
    job.record.file  = file;
    job.record.line  = line;
    job.record.level = level;
    dispatcher__copy_payload(&job, payload, payload_len);

    ct_logger_add_pending_job(logger);
    int pending = ct_atomic_int_add(&self->pending_jobs, 1) + 1;
    dispatcher__update_high_watermark(self, pending);

    if (ct_msgqueue_try_push(&self->queue, &job) != 0) {
        dispatcher__finish_global_job(self);
        ct_logger_finish_pending_job(logger);
        return -1;
    }
    return 0;
}

void ct_log_dispatcher_flush(ct_log_dispatcher_t* self) {
    if (!self) { return; }
    ct_mutex_lock(&self->flush_mutex);
    while (ct_atomic_int_load(&self->pending_jobs) > 0) { ct_cond_wait(&self->flush_cond, &self->flush_mutex); }
    ct_mutex_unlock(&self->flush_mutex);
}
