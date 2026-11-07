/**
 * @file jobpool.c
 * @brief Fixed-size job pool with a shared work queue.
 */
#include "coter/thread/jobpool.h"

#include <stdlib.h>

#include "coter/container/list.h"
#include "coter/core/macro.h"
#include "coter/sync/msgqueue.h"
#include "coter/thread/thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

// 10ms
#define JOBPOOL_YIELD_SLICE_US 10000

// Work item.
typedef struct job {
    ct_jobpool_routine_t routine_;  // Routine to execute.
    void*                arg_;      // Routine argument.
} job_t;

#define CT_JOBPOOL_JOB_INIT(routine_value_, arg_value_) {.routine_ = (routine_value_), .arg_ = (arg_value_)}

// Worker thread state.
typedef struct unit {
    ct_list_t      list_[1];    // Intrusive list node.
    ct_msgqueue_t* job_queue_;  // Shared work queue.
    ct_thread_t    thread_;     // Worker thread handle.
} unit_t;

// Job pool.
struct ct_jobpool {
    job_t*        job_buffer_;       // Work queue storage.
    ct_msgqueue_t job_queue_[1];     // Shared work queue.
    ct_list_t     regular_list_[1];  // Worker list.
    size_t        thread_max_;       // Worker count.
    size_t        job_max_;          // Queue capacity.
};

static int m_worker_thread(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_jobpool_t* ct_jobpool_create(size_t thread_max, size_t job_max, const ct_jobpool_config_t* config) {
    if (!thread_max || !job_max) { return NULL; }

    ct_jobpool_t* self = (ct_jobpool_t*)malloc(sizeof(ct_jobpool_t));
    if (!self) { return NULL; }

    self->thread_max_ = thread_max;
    self->job_max_    = job_max;

    self->job_buffer_ = (job_t*)calloc(job_max, sizeof(job_t));
    if (!self->job_buffer_) {
        free(self);
        return NULL;
    }
    if (ct_msgqueue_init(self->job_queue_, self->job_buffer_, sizeof(job_t), job_max) != 0) {
        free(self->job_buffer_);
        free(self);
        return NULL;
    }
    ct_list_init(self->regular_list_);

    ct_jobpool_config_t cfg;
    if (config) {
        cfg = *config;
    } else {
        ct_jobpool_default_config(&cfg);
    }

    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    attr.stack_size_      = cfg.stack_size_;

    for (size_t i = 0; i < thread_max; ++i) {
        unit_t* unit = (unit_t*)malloc(sizeof(unit_t));
        if (!unit) {
            ct_jobpool_destroy(self);
            return NULL;
        }

        unit->job_queue_ = self->job_queue_;
        ct_list_init(unit->list_);

        const int ret = ct_thread_create(&unit->thread_, &attr, m_worker_thread, unit);
        if (ret != 0) {
            free(unit);
            ct_jobpool_destroy(self);
            return NULL;
        }
        ct_list_append(self->regular_list_, unit->list_);
    }

    return self;
}

void ct_jobpool_destroy(ct_jobpool_t* self) {
    if (!self) { return; }

    ct_msgqueue_close(self->job_queue_);

    ct_list_foreach_entry_safe(unit, self->regular_list_, unit_t, list_) {
        ct_thread_join(&unit->thread_, NULL);
        ct_list_remove(unit->list_);
        free(unit);
    }

    ct_msgqueue_destroy(self->job_queue_);
    if (self->job_buffer_) { free(self->job_buffer_); }
    free(self);
}

int ct_jobpool_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg) {
    if (!self || !routine) { return -1; }
    const job_t job = CT_JOBPOOL_JOB_INIT(routine, arg);
    return ct_msgqueue_push(self->job_queue_, &job) != 0 ? -1 : 0;
}

int ct_jobpool_try_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg) {
    if (!self || !routine) { return -1; }
    const job_t job = CT_JOBPOOL_JOB_INIT(routine, arg);
    return ct_msgqueue_try_push(self->job_queue_, &job) != 0 ? -1 : 0;
}

int ct_jobpool_submit_for(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg, ct_time64_t timeout_ms) {
    if (!self || !routine) { return -1; }
    const job_t job = CT_JOBPOOL_JOB_INIT(routine, arg);
    return ct_msgqueue_push_for(self->job_queue_, &job, timeout_ms) != 0 ? -1 : 0;
}

size_t ct_jobpool_pending(ct_jobpool_t* self) {
    if (!self) { return 0; }
    return ct_msgqueue_size(self->job_queue_);
}

// -------------------------[STATIC DEFINITION]-------------------------

static int m_worker_thread(void* arg) {
    unit_t*     unit    = (unit_t*)arg;
    ct_time64_t next_us = ct_gethrtime_us() + JOBPOOL_YIELD_SLICE_US;

    for (;;) {
        job_t job;
        if (ct_msgqueue_try_pop(unit->job_queue_, &job) != 0) {
            if (ct_msgqueue_pop(unit->job_queue_, &job) != 0) { break; }
        }
        if (job.routine_) { job.routine_(job.arg_); }

        if (ct_gethrtime_us() >= next_us) {
            ct_thread_yield();
            next_us = ct_gethrtime_us() + JOBPOOL_YIELD_SLICE_US;
        }
    }

    return 0;
}
