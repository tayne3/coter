/**
 * @file dispatcher.c
 * @brief Internal log dispatcher.
 */
#include <string.h>

#include "coter/sync/msgqueue.h"
#include "coter/thread/thread.h"
#include "log_internal.h"

enum ct_log_dispatcher_state {
    CT_LOG_DISPATCHER_INIT     = 0,
    CT_LOG_DISPATCHER_STARTING = 1,
    CT_LOG_DISPATCHER_READY    = 2,
    CT_LOG_DISPATCHER_FAILED   = -1,
};

static struct dispatcher {
    ct_atomic_int_t state;
    ct_thread_t     worker;
    ct_msgqueue_t   queue;
    ct_log_job_t    q_buf[CT_LOG_QUEUE_SIZE];
} mgr[1] = {{
    .state = CT_ATOMIC_VAR_INIT(CT_LOG_DISPATCHER_INIT),
}};

static int  dispatcher__routine(void* arg);
static int  dispatcher__start(void);
static bool dispatcher__is_ready(void);
static void dispatcher__dispatch_record(ct_logger_t* logger, const ct_log_record_job_t* job);

static int dispatcher__routine(void* arg) {
    CT_UNUSED(arg);

    ct_log_job_t job;
    while (ct_msgqueue_pop(&mgr->queue, &job) == 0) {
        switch (job.type) {
            case CT_LOG_JOB_RECORD: dispatcher__dispatch_record(job.logger, &job.record); break;

            case CT_LOG_JOB_FLUSH:
                if (job.logger) {
                    ct_list_foreach_entry(handler, &job.logger->handlers, ct_log_handler_t, node) {
                        if (handler->vtable && handler->vtable->flush) { handler->vtable->flush(handler); }
                    }
                }
                CT_FALLTHROUGH;

            case CT_LOG_JOB_BARRIER: {
                ct_log_barrier_t* b = job.barrier;
                ct_mutex_lock(&b->mtx);
                b->done = true;
                ct_cond_signal(&b->cond);
                ct_mutex_unlock(&b->mtx);
                break;
            }

            default: break;
        }
    }
    return 0;
}

static void dispatcher__dispatch_record(ct_logger_t* logger, const ct_log_record_job_t* job) {
    if (!logger || !job || job->size == 0) { return; }

    ct_log_record_t record;
    record.time  = job->time;
    record.tid   = job->tid;
    record.file  = job->file;
    record.line  = job->line;
    record.level = job->level;
    record.size  = job->size;
    record.data  = job->payload;

    ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
        if (handler->vtable && handler->vtable->write) { handler->vtable->write(handler, &record); }
    }
}

static int dispatcher__start(void) {
    ct_msgqueue_init(&mgr->queue, mgr->q_buf, sizeof(ct_log_job_t), CT_LOG_QUEUE_SIZE);

    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    attr.stack_size       = 1 * 1024 * 1024;  // stack size: 1MB
    if (ct_thread_create(&mgr->worker, &attr, dispatcher__routine, NULL) != 0) {
        ct_msgqueue_destroy(&mgr->queue);
        return -1;
    }

    ct_atomic_int_store(&mgr->state, CT_LOG_DISPATCHER_READY);
    return 0;
}

static bool dispatcher__is_ready(void) {
    return ct_atomic_int_load(&mgr->state) == CT_LOG_DISPATCHER_READY;
}

int ct_log_dispatcher_start(void) {
    int state = ct_atomic_int_load(&mgr->state);
    if (state == CT_LOG_DISPATCHER_READY) { return 0; }
    if (state == CT_LOG_DISPATCHER_FAILED) { return -1; }

    int expected = CT_LOG_DISPATCHER_INIT;
    if (ct_atomic_int_compare_exchange(&mgr->state, &expected, CT_LOG_DISPATCHER_STARTING)) {
        if (dispatcher__start() == 0) { return 0; }

        ct_atomic_int_store(&mgr->state, CT_LOG_DISPATCHER_FAILED);
        return -1;
    }

    while (ct_atomic_int_load(&mgr->state) == CT_LOG_DISPATCHER_STARTING) { ct_thread_yield(); }
    return ct_atomic_int_load(&mgr->state) == CT_LOG_DISPATCHER_READY ? 0 : -1;
}

int ct_log_dispatcher_submit(const ct_log_job_t* job) {
    if (!job || !job->logger || !dispatcher__is_ready()) { return -1; }
    if (ct_log_dispatcher_is_worker()) { return -1; }
    return ct_msgqueue_push(&mgr->queue, job);
}

int ct_log_dispatcher_sync(ct_logger_t* logger, int job_type) {
    if (!logger || !dispatcher__is_ready()) { return -1; }
    if (ct_log_dispatcher_is_worker()) { return -1; }
    if (job_type != CT_LOG_JOB_BARRIER && job_type != CT_LOG_JOB_FLUSH) { return -1; }

    ct_log_barrier_t b;
    ct_mutex_init(&b.mtx);
    ct_cond_init(&b.cond);
    b.done = false;

    ct_log_job_t job;
    memset(&job, 0, sizeof(job));
    job.type    = job_type;
    job.logger  = logger;
    job.barrier = &b;

    if (ct_log_dispatcher_submit(&job) != 0) {
        ct_cond_destroy(&b.cond);
        ct_mutex_destroy(&b.mtx);
        return -1;
    }

    ct_mutex_lock(&b.mtx);
    while (!b.done) { ct_cond_wait(&b.cond, &b.mtx); }
    ct_mutex_unlock(&b.mtx);

    ct_cond_destroy(&b.cond);
    ct_mutex_destroy(&b.mtx);
    return 0;
}

bool ct_log_dispatcher_is_worker(void) {
    return dispatcher__is_ready() && ct_thread_is_self(mgr->worker);
}
