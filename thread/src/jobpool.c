/**
 * @file jobpool.c
 * @brief Fixed-size job pool with a shared work queue.
 */
#include "coter/thread/jobpool.h"

#include <stdlib.h>
#include <string.h>

#include "coter/container/list.h"
#include "coter/core/macro.h"
#include "coter/sync/msgqueue.h"
#include "coter/thread/thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define JOBPOOL_YIELD_SLICE_US     ((ct_time64_t)10 * 1000)
#define JOBPOOL_YIELD_BATCH_MIN_US (JOBPOOL_YIELD_SLICE_US / 16)
#define JOBPOOL_YIELD_BATCH_MAX_US (JOBPOOL_YIELD_SLICE_US / 4)
#define JOBPOOL_YIELD_SAMPLE_MAX   ((size_t)512)

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

// Adaptive time-slice state for one worker.
typedef struct jobpool_yield {
    size_t      sample_;       // Jobs to execute before the next time check.
    size_t      consec_;       // Jobs executed since the last time check.
    ct_time64_t slice_start_;  // Start of the current yield time slice.
    ct_time64_t batch_start_;  // Start of the current sampling batch.
} jobpool_yield_t;

// Job pool.
struct ct_jobpool {
    job_t*        job_buffer_;       // Work queue storage.
    ct_msgqueue_t job_queue_[1];     // Shared work queue.
    ct_list_t     regular_list_[1];  // Worker list.
    size_t        thread_max_;       // Worker count.
    size_t        job_max_;          // Queue capacity.
};

static int jobpool_worker_thread(void* arg);

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

    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    if (config && config->thread_attr_) {
        memcpy(&attr, config->thread_attr_, sizeof(ct_thread_attr_t));
    } else {
        ct_thread_attr_set_stack_size(&attr, 1 * 1024 * 1024);
    }

    for (size_t i = 0; i < thread_max; ++i) {
        unit_t* unit = (unit_t*)malloc(sizeof(unit_t));
        if (!unit) {
            ct_thread_attr_destroy(&attr);
            ct_jobpool_destroy(self);
            return NULL;  // Treat allocation failure as total failure.
        }

        unit->job_queue_ = self->job_queue_;
        ct_list_init(unit->list_);

        const int ret = ct_thread_create(&unit->thread_, &attr, jobpool_worker_thread, unit);
        if (ret != 0) {
            free(unit);
            ct_thread_attr_destroy(&attr);
            ct_jobpool_destroy(self);
            return NULL;  // Treat thread creation failure as total failure.
        }
        ct_list_append(self->regular_list_, unit->list_);
    }

    ct_thread_attr_destroy(&attr);
    return self;
}

void ct_jobpool_destroy(ct_jobpool_t* self) {
    if (!self) { return; }

    // Close the queue and wake all blocked workers.
    ct_msgqueue_close(self->job_queue_);

    // Wait for every worker to exit.
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

static void jobpool_yield_init(jobpool_yield_t* state) {
    const ct_time64_t now = ct_gethrtime_us();

    state->sample_      = 1;
    state->consec_      = 0;
    state->slice_start_ = now;
    state->batch_start_ = now;
}

static void jobpool_yield_after_pop(jobpool_yield_t* state, ct_time64_t wait_start) {
    const ct_time64_t wait_end = ct_gethrtime_us();

    // Only a real block means the worker already gave up the CPU. A pop that
    // returns immediately must not reset the slice or the sampling batch.
    if (wait_end - wait_start >= JOBPOOL_YIELD_SLICE_US / 2) {
        state->slice_start_ = wait_end;
        state->batch_start_ = wait_end;
        state->sample_      = 1;
        state->consec_      = 0;
    }
}

static void jobpool_yield_after_job(jobpool_yield_t* state) {
    if (state->consec_ < state->sample_) { return; }

    const ct_time64_t now      = ct_gethrtime_us();
    const ct_time64_t batch_us = now - state->batch_start_;
    state->consec_             = 0;

    if (now - state->slice_start_ >= JOBPOOL_YIELD_SLICE_US) {
        ct_thread_yield();
        state->slice_start_ = ct_gethrtime_us();
        state->batch_start_ = state->slice_start_;
    } else {
        state->batch_start_ = now;
    }

    // Aim for a sampling batch of slice/16..slice/4 to amortize clock reads.
    // A step change in job duration is only visible at the next sample.
    if (batch_us >= JOBPOOL_YIELD_SLICE_US) {
        state->sample_ = 1;
    } else if (batch_us < JOBPOOL_YIELD_BATCH_MIN_US) {
        if (state->sample_ < JOBPOOL_YIELD_SAMPLE_MAX) { state->sample_ *= 2; }
    } else if (batch_us > JOBPOOL_YIELD_BATCH_MAX_US) {
        if (state->sample_ > 1) { state->sample_ /= 2; }
    }
}

static int jobpool_worker_thread(void* arg) {
    unit_t* unit = (unit_t*)arg;
    job_t   job;

    jobpool_yield_t yield_state;
    jobpool_yield_init(&yield_state);

    for (;;) {
        if (ct_msgqueue_try_pop(unit->job_queue_, &job) != 0) {
            const ct_time64_t wait_start = ct_gethrtime_us();
            if (ct_msgqueue_pop(unit->job_queue_, &job) != 0) { break; }
            jobpool_yield_after_pop(&yield_state, wait_start);
        }
        ++yield_state.consec_;

        if (job.routine_) { job.routine_(job.arg_); }

        jobpool_yield_after_job(&yield_state);
    }

    return 0;
}
