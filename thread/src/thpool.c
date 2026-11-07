/**
 * @file thpool.c
 * @brief Dynamically sized thread pool.
 */
#include "coter/thread/thpool.h"

#include <errno.h>
#include <stdlib.h>

#include "coter/container/list.h"
#include "coter/core/abort.h"
#include "coter/core/time.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

// Queued task.
typedef struct task {
    ct_list_t           list_[1];
    ct_thpool_routine_t routine_;
    void*               arg_;
} task_t;

// Worker thread node.
typedef struct worker {
    ct_list_t         list_[1];
    ct_thread_t       thread_;
    struct ct_thpool* pool_;
} worker_t;

// Thread pool.
struct ct_thpool {
    ct_mutex_t lock_;
    ct_cond_t  task_cond_;
    ct_cond_t  slot_cond_;

    ct_list_t queue_head_[1];
    ct_list_t live_head_[1];
    ct_list_t retired_head_[1];

    size_t capacity_;
    size_t live_count_;
    size_t waiting_count_;
    size_t blocked_count_;
    size_t queued_count_;
    bool   closed_;

    ct_thpool_config_t cfg_;
};

// Create a worker while the lock is held; reserves the live slot first.
static int thpool_worker_create(ct_thpool_t* self);
// Worker thread entry point.
static int thpool_worker_thread(void* arg);
// Worker exit section; called with the lock held.
static void thpool_worker_retire(ct_thpool_t* self, worker_t* worker);
// Join a worker thread; abort because continuing would risk double-free.
static void thpool_thread_join(ct_thread_t* thread);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_thpool_t* ct_thpool_create(size_t size, const ct_thpool_config_t* cfg) {
    ct_thpool_t* self = (ct_thpool_t*)malloc(sizeof(ct_thpool_t));
    if (!self) { return NULL; }
    if (ct_mutex_init(&self->lock_) != 0) {
        free(self);
        return NULL;
    }
    if (ct_cond_init(&self->task_cond_) != 0) {
        ct_mutex_destroy(&self->lock_);
        free(self);
        return NULL;
    }
    if (ct_cond_init(&self->slot_cond_) != 0) {
        ct_cond_destroy(&self->task_cond_);
        ct_mutex_destroy(&self->lock_);
        free(self);
        return NULL;
    }

    ct_list_init(self->queue_head_);
    ct_list_init(self->live_head_);
    ct_list_init(self->retired_head_);

    self->capacity_      = size;
    self->live_count_    = 0;
    self->waiting_count_ = 0;
    self->blocked_count_ = 0;
    self->queued_count_  = 0;
    self->closed_        = false;

    if (cfg) {
        self->cfg_ = *cfg;
    } else {
        ct_thpool_default_config(&self->cfg_);
    }
    return self;
}

void ct_thpool_close(ct_thpool_t* self) {
    if (!self) { return; }

    ct_mutex_lock(&self->lock_);
    self->closed_ = true;
    ct_cond_broadcast(&self->task_cond_);
    ct_cond_broadcast(&self->slot_cond_);
    ct_mutex_unlock(&self->lock_);
}

void ct_thpool_destroy(ct_thpool_t* self) {
    if (!self) { return; }
    ct_thpool_close(self);

    ct_mutex_lock(&self->lock_);
    while (self->live_count_ > 0 || self->blocked_count_ > 0) { ct_cond_wait(&self->slot_cond_, &self->lock_); }
    ct_mutex_unlock(&self->lock_);

    ct_list_foreach_entry_safe(worker, self->retired_head_, worker_t, list_) {
        ct_list_remove(worker->list_);
        thpool_thread_join(&worker->thread_);
        free(worker);
    }

    ct_mutex_destroy(&self->lock_);
    ct_cond_destroy(&self->task_cond_);
    ct_cond_destroy(&self->slot_cond_);

    free(self);
}

int ct_thpool_submit(ct_thpool_t* self, ct_thpool_routine_t routine, void* arg) {
    if (!self) { return CT_THPOOL_ERROR_CLOSED; }
    if (!routine) { return CT_THPOOL_ERROR_INVALID_ARG; }

    task_t* task = (task_t*)malloc(sizeof(task_t));
    if (!task) { return CT_THPOOL_ERROR_MEM_ALLOC; }
    task->routine_ = routine;
    task->arg_     = arg;
    ct_list_init(task->list_);

    int rc = 0;
    ct_mutex_lock(&self->lock_);
    for (;;) {
        if (self->closed_) {
            rc = CT_THPOOL_ERROR_CLOSED;
            break;
        }

        if (self->cfg_.max_tasks_ == 0 || self->queued_count_ < self->cfg_.max_tasks_) {
            if (self->waiting_count_ == 0 && (self->capacity_ == 0 || self->live_count_ < self->capacity_)) {
                rc = thpool_worker_create(self);
                if (rc != 0) { break; }
            }
            ct_list_append(self->queue_head_, task->list_);
            ++self->queued_count_;
            ct_cond_signal(&self->task_cond_);
            break;
        }

        if (self->cfg_.non_blocking_) {
            rc = CT_THPOOL_ERROR_OVERLOAD;
            break;
        }
        ++self->blocked_count_;
        ct_cond_wait(&self->slot_cond_, &self->lock_);
        --self->blocked_count_;
        if (self->blocked_count_ == 0) { ct_cond_broadcast(&self->slot_cond_); }
    }
    ct_mutex_unlock(&self->lock_);

    if (rc != 0) { free(task); }
    return rc;
}

const char* ct_thpool_strerror(int errcode) {
    const char* descriptions[] = {
#define F(code, name, description) description,
        CT_THPOOL_ERROR_FOREACH(F)
#undef F
    };
    if (errcode < 0 || errcode >= CT_THPOOL_ERROR_MAX) { return "unknown error"; }
    return descriptions[errcode];
}

// -------------------------[STATIC DEFINITION]-------------------------

static int thpool_worker_create(ct_thpool_t* self) {
    worker_t* worker = (worker_t*)malloc(sizeof(worker_t));
    if (!worker) { return CT_THPOOL_ERROR_MEM_ALLOC; }

    worker->pool_ = self;
    ct_list_init(worker->list_);

    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    attr.stack_size_      = self->cfg_.stack_size_;

    ++self->live_count_;
    const int ret = ct_thread_create(&worker->thread_, &attr, thpool_worker_thread, worker);
    if (ret != 0) {
        --self->live_count_;
        ct_cond_broadcast(&self->slot_cond_);
        free(worker);
        return CT_THPOOL_ERROR_MEM_ALLOC;
    }
    ct_list_append(self->live_head_, worker->list_);
    return 0;
}

static int thpool_worker_thread(void* arg) {
    worker_t* worker = (worker_t*)arg;
    if (!worker) { return 0; }
    ct_thpool_t* self = worker->pool_;
    if (!self) { return 0; }

    ct_mutex_lock(&self->lock_);
    for (;;) {
        while (ct_list_is_empty(self->queue_head_) && !self->closed_) {
            ++self->waiting_count_;
            int timed_out = 0;
            if (self->cfg_.idle_timeout_ > 0) {
                timed_out = ct_cond_wait_for(&self->task_cond_, &self->lock_, (ct_time64_t)self->cfg_.idle_timeout_) ==
                            ETIMEDOUT;
            } else {
                ct_cond_wait(&self->task_cond_, &self->lock_);
            }
            --self->waiting_count_;
            if (timed_out && !self->closed_ && ct_list_is_empty(self->queue_head_)) { break; }
        }

        if (!ct_list_is_empty(self->queue_head_)) {
            task_t* task = ct_list_first_entry(self->queue_head_, task_t, list_);
            ct_list_remove(task->list_);
            --self->queued_count_;
            if (self->blocked_count_ > 0) { ct_cond_broadcast(&self->slot_cond_); }
            ct_mutex_unlock(&self->lock_);

            task->routine_(task->arg_);
            free(task);

            ct_mutex_lock(&self->lock_);
            continue;
        }

        break;
    }

    thpool_worker_retire(self, worker);
    return 0;
}

static void thpool_worker_retire(ct_thpool_t* self, worker_t* worker) {
    if (!ct_list_is_empty(self->retired_head_)) {
        ct_list_t stale[1];
        ct_list_init(stale);
        ct_list_splice_prev(stale, self->retired_head_);
        ct_mutex_unlock(&self->lock_);
        ct_list_foreach_entry_safe(done, stale, worker_t, list_) {
            thpool_thread_join(&done->thread_);
            free(done);
        }
        ct_mutex_lock(&self->lock_);
    }

    ct_list_remove(worker->list_);
    ct_list_append(self->retired_head_, worker->list_);
    --self->live_count_;
    ct_cond_broadcast(&self->slot_cond_);
    ct_mutex_unlock(&self->lock_);
}

static void thpool_thread_join(ct_thread_t* thread) {
    if (ct_thread_join(thread, NULL) != 0) { CT_ABORT(); }
}
