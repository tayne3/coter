/**
 * @file thpool.c
 * @brief Dynamically sized thread pool.
 */
#include "coter/thread/thpool.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "coter/container/list.h"
#include "coter/core/abort.h"
#include "coter/core/time.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief Queued task.
 *
 * Allocated on submission and freed by the worker that executes it.
 */
typedef struct task {
    ct_thpool_routine_t routine_;  ///< Routine to execute.
    void*               arg_;      ///< Routine argument.
    ct_list_t           link_[1];  ///< Task queue node.
} task_t;

/**
 * @brief Worker thread node.
 *
 * Lives on the pool's live list until its thread finishes the exit section,
 * then on the retired list until another thread reaps it.
 */
typedef struct worker {
    ct_list_t         link_[1];  ///< Live/retired list node.
    ct_thread_t       thread_;   ///< Worker thread handle.
    struct ct_thpool* pool_;     ///< Owning thread pool.
} worker_t;

/**
 * @brief Thread pool.
 *
 * All state is protected by one mutex. Workers block on task_cond_ waiting
 * for a queued task (or idle timeout). Submitters blocked on a full queue and
 * destroy() block on slot_cond_. Every wait re-checks its predicate in a
 * loop, so wakeups can only be spurious, never lost.
 */
struct ct_thpool {
    ct_mutex_t lock_[1];       ///< Protects all state below.
    ct_cond_t  task_cond_[1];  ///< Workers: task queued, or pool closed.
    ct_cond_t  slot_cond_[1];  ///< Submitters/destroy: queue space, closed, or fully stopped.

    ct_list_t queue_head_[1];    ///< Queued tasks.
    ct_list_t live_head_[1];     ///< Workers whose threads have not exited yet.
    ct_list_t retired_head_[1];  ///< Exited workers awaiting join and free.

    size_t capacity_;       ///< Maximum workers; 0 means unlimited.
    size_t live_count_;     ///< Live workers, including slots being created.
    size_t waiting_count_;  ///< Workers blocked on task_cond_.
    size_t blocked_count_;  ///< Submitters blocked on slot_cond_.
    size_t queued_count_;   ///< Tasks currently queued.
    bool   closed_;         ///< Non-zero once the pool is closed.

    ct_thread_attr_t thread_attr_;      ///< Worker thread attributes.
    bool             has_thread_attr_;  ///< Non-zero when thread_attr_ is configured.
    size_t           idle_timeout_;     ///< Idle timeout in milliseconds; 0 keeps workers.
    size_t           max_tasks_;        ///< Maximum queued tasks; 0 means unlimited.
    bool             non_blocking_;     ///< Reject submissions when the queue is full.
};

/* Initialize pool configuration. */
static void thpool_config_init(ct_thpool_t* self, ct_thpool_config_t* config);
/* Create a worker while the lock is held; reserves the live slot first. */
static int thpool_worker_create(ct_thpool_t* self);
/* Worker thread entry point. */
static int thpool_worker_thread(void* arg);
/* Worker exit section; called with the lock held. */
static void thpool_worker_retire(ct_thpool_t* self, worker_t* worker);
/* Join a worker thread; abort because continuing would risk double-free. */
static void thpool_thread_join(ct_thread_t* thread);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_thpool_t* ct_thpool_create(size_t size, ct_thpool_config_t* config) {
    ct_thpool_t* self = (ct_thpool_t*)malloc(sizeof(ct_thpool_t));
    if (!self) { return NULL; }

    thpool_config_init(self, config);
    self->capacity_      = size;
    self->live_count_    = 0;
    self->waiting_count_ = 0;
    self->blocked_count_ = 0;
    self->queued_count_  = 0;
    self->closed_        = false;

    ct_list_init(self->queue_head_);
    ct_list_init(self->live_head_);
    ct_list_init(self->retired_head_);

    if (ct_mutex_init(self->lock_) != 0) {
        if (self->has_thread_attr_) { ct_thread_attr_destroy(&self->thread_attr_); }
        free(self);
        return NULL;
    }
    if (ct_cond_init(self->task_cond_) != 0) {
        ct_mutex_destroy(self->lock_);
        if (self->has_thread_attr_) { ct_thread_attr_destroy(&self->thread_attr_); }
        free(self);
        return NULL;
    }
    if (ct_cond_init(self->slot_cond_) != 0) {
        ct_cond_destroy(self->task_cond_);
        ct_mutex_destroy(self->lock_);
        if (self->has_thread_attr_) { ct_thread_attr_destroy(&self->thread_attr_); }
        free(self);
        return NULL;
    }
    return self;
}

void ct_thpool_close(ct_thpool_t* self) {
    if (!self) { return; }

    // Already-queued tasks keep running; workers drain the queue before exit.
    ct_mutex_lock(self->lock_);
    self->closed_ = true;
    ct_cond_broadcast(self->task_cond_);
    ct_cond_broadcast(self->slot_cond_);
    ct_mutex_unlock(self->lock_);
}

void ct_thpool_destroy(ct_thpool_t* self) {
    if (!self) { return; }
    ct_thpool_close(self);

    // Wait until every worker finished its exit section and every blocked
    // submitter returned. All paths that drop these counters broadcast
    // slot_cond_ under the lock, so a plain wait cannot miss a wakeup.
    ct_mutex_lock(self->lock_);
    while (self->live_count_ > 0 || self->blocked_count_ > 0) { ct_cond_wait(self->slot_cond_, self->lock_); }
    ct_mutex_unlock(self->lock_);

    // live_count_ == 0: every worker moved itself to the retired list and no
    // longer touches the pool, so joining and freeing them is safe.
    ct_list_foreach_entry_safe(worker, self->retired_head_, worker_t, link_) {
        ct_list_remove(worker->link_);
        thpool_thread_join(&worker->thread_);
        free(worker);
    }

    ct_mutex_destroy(self->lock_);
    ct_cond_destroy(self->task_cond_);
    ct_cond_destroy(self->slot_cond_);

    if (self->has_thread_attr_) { ct_thread_attr_destroy(&self->thread_attr_); }
    free(self);
}

int ct_thpool_submit(ct_thpool_t* self, ct_thpool_routine_t routine, void* arg) {
    if (!self) { return CT_THPOOL_ERROR_CLOSED; }
    if (!routine) { return CT_THPOOL_ERROR_TASK_NULL; }

    task_t* task = (task_t*)malloc(sizeof(task_t));
    if (!task) { return CT_THPOOL_ERROR_MEM_ALLOC; }
    task->routine_ = routine;
    task->arg_     = arg;
    ct_list_init(task->link_);

    int rc = 0;
    ct_mutex_lock(self->lock_);
    for (;;) {
        if (self->closed_) {
            rc = CT_THPOOL_ERROR_CLOSED;
            break;
        }

        if (self->max_tasks_ == 0 || self->queued_count_ < self->max_tasks_) {
            // No idle worker will pick this task up: grow while capacity allows.
            // Creating the thread under the lock keeps the new worker visible
            // to close/destroy from the start; it queues on the mutex anyway.
            if (self->waiting_count_ == 0 && (self->capacity_ == 0 || self->live_count_ < self->capacity_)) {
                rc = thpool_worker_create(self);
                if (rc != 0) { break; }
            }
            ct_list_append(self->queue_head_, task->link_);
            self->queued_count_++;
            ct_cond_signal(self->task_cond_);
            break;
        }

        // The queue is full.
        if (self->non_blocking_) {
            rc = CT_THPOOL_ERROR_OVERLOAD;
            break;
        }
        self->blocked_count_++;
        ct_cond_wait(self->slot_cond_, self->lock_);
        self->blocked_count_--;
        if (self->blocked_count_ == 0) { ct_cond_broadcast(self->slot_cond_); }
    }
    ct_mutex_unlock(self->lock_);

    if (rc != 0) { free(task); }
    return rc;
}

const char* ct_thpool_strerror(int error_code) {
    const char* descriptions[] = {
#define F(code, name, description) description,
        CT_THPOOL_ERROR_FOREACH(F)
#undef F
    };
    if (error_code < 0 || error_code >= CT_THPOOL_ERROR_MAX) { return "unknown error"; }
    return descriptions[error_code];
}

void ct_thpool_default_config(ct_thpool_config_t* config) {
    if (!config) { return; }
    config->thread_attr_  = NULL;
    config->idle_timeout_ = 1000;
    config->non_blocking_ = false;
    config->max_tasks_    = 1000;
}

// -------------------------[STATIC DEFINITION]-------------------------

static void thpool_config_init(ct_thpool_t* self, ct_thpool_config_t* config) {
    self->idle_timeout_    = 1000;
    self->non_blocking_    = false;
    self->max_tasks_       = 1000;
    self->has_thread_attr_ = false;

    if (config) {
        if (config->thread_attr_) {
            memcpy(&self->thread_attr_, config->thread_attr_, sizeof(ct_thread_attr_t));
            self->has_thread_attr_ = true;
        }
        self->idle_timeout_ = config->idle_timeout_;
        self->non_blocking_ = config->non_blocking_;
        self->max_tasks_    = config->max_tasks_;
    }
}

static int thpool_worker_create(ct_thpool_t* self) {
    worker_t* worker = (worker_t*)malloc(sizeof(worker_t));
    if (!worker) { return CT_THPOOL_ERROR_MEM_ALLOC; }

    worker->pool_ = self;
    ct_list_init(worker->link_);

    // Reserve the live slot before creating the thread so close/destroy can
    // see a worker is coming even if creation is still in progress.
    self->live_count_++;
    const int ret = ct_thread_create(&worker->thread_, self->has_thread_attr_ ? &self->thread_attr_ : NULL,
                                     thpool_worker_thread, worker);
    if (ret != 0) {
        self->live_count_--;
        ct_cond_broadcast(self->slot_cond_);  // destroy() may wait on live_count_.
        free(worker);
        return CT_THPOOL_ERROR_MEM_ALLOC;
    }
    ct_list_append(self->live_head_, worker->link_);
    return 0;
}

static int thpool_worker_thread(void* arg) {
    worker_t* worker = (worker_t*)arg;
    if (!worker) { return 0; }
    ct_thpool_t* self = worker->pool_;
    if (!self) { return 0; }

    ct_mutex_lock(self->lock_);
    for (;;) {
        while (ct_list_is_empty(self->queue_head_) && !self->closed_) {
            self->waiting_count_++;
            int timed_out = 0;
            if (self->idle_timeout_ > 0) {
                timed_out =
                    ct_cond_wait_for(self->task_cond_, self->lock_, (ct_time64_t)self->idle_timeout_) == ETIMEDOUT;
            } else {
                ct_cond_wait(self->task_cond_, self->lock_);
            }
            self->waiting_count_--;
            // Idle past the timeout with no work and not closing: retire.
            if (timed_out && !self->closed_ && ct_list_is_empty(self->queue_head_)) { break; }
        }

        if (!ct_list_is_empty(self->queue_head_)) {
            task_t* task = ct_list_first_entry(self->queue_head_, task_t, link_);
            ct_list_remove(task->link_);
            self->queued_count_--;
            // Popping freed a queue slot for a blocked submitter. Broadcast so
            // destroy() cannot consume a wakeup meant for a submitter.
            if (self->blocked_count_ > 0) { ct_cond_broadcast(self->slot_cond_); }
            ct_mutex_unlock(self->lock_);

            task->routine_(task->arg_);
            free(task);

            ct_mutex_lock(self->lock_);
            continue;
        }

        break;
    }

    thpool_worker_retire(self, worker);
    return 0;
}

static void thpool_worker_retire(ct_thpool_t* self, worker_t* worker) {
    // Reap predecessors first. Their threads already finished this section,
    // so the joins only reclaim resources and cannot block meaningfully.
    if (!ct_list_is_empty(self->retired_head_)) {
        ct_list_t stale[1];
        ct_list_init(stale);
        ct_list_splice_prev(stale, self->retired_head_);
        ct_mutex_unlock(self->lock_);
        ct_list_foreach_entry_safe(done, stale, worker_t, link_) {
            thpool_thread_join(&done->thread_);
            free(done);
        }
        ct_mutex_lock(self->lock_);
    }

    // The worker's only list move and its last access to the pool. live_count_
    // is still above zero while reaping above, so destroy() cannot be sweeping.
    ct_list_remove(worker->link_);
    ct_list_append(self->retired_head_, worker->link_);
    self->live_count_--;
    ct_cond_broadcast(self->slot_cond_);  // destroy() waits for live_count_ == 0.
    ct_mutex_unlock(self->lock_);
}

static void thpool_thread_join(ct_thread_t* thread) {
    if (ct_thread_join(thread, NULL) != 0) { CT_ABORT(); }
}
