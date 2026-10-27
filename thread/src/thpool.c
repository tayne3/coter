/**
 * @file thpool.c
 * @brief Dynamically sized thread pool.
 */
#include "coter/thread/thpool.h"

#include <stdlib.h>
#include <string.h>

#include "coter/container/list.h"
#include "coter/sync/atomic.h"
#include "coter/sync/msgqueue.h"
#include "coter/thread/thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief Work task.
 */
typedef struct task {
    ct_thpool_routine_t routine_;  ///< Routine to execute.
    void*               arg_;      ///< Routine argument.
} task_t;

#define THPOOL_TASK_INIT(routine_, arg_) {(routine_), (arg_)}

/**
 * @brief Worker thread state.
 */
typedef struct worker {
    ct_list_t     list_[1];       ///< Intrusive list node.
    ct_thread_t   thread_;        ///< Worker thread handle.
    ct_thpool_t*  thpool_;        ///< Owning thread pool.
    task_t        task_buff_[1];  ///< Queue storage.
    ct_msgqueue_t tasks_[1];      ///< Task queue.
    ct_time64_t   last_use_;      ///< Last activity timestamp in milliseconds.
} worker_t;

/**
 * @brief Thread pool status.
 */
typedef struct status {
    ct_atomic_long_t capacity_;    ///< Maximum workers; 0 means unlimited.
    ct_atomic_long_t total_size_;  ///< Total workers.
    ct_atomic_long_t wait_size_;   ///< Blocked submitters.
    ct_atomic_long_t closed_;      ///< Non-zero once the pool is closed.
} status_t;

/**
 * @brief Thread pool.
 */
struct ct_thpool {
    status_t         status_;           ///< Thread pool status.
    ct_thread_attr_t thread_attr_;      ///< Worker thread attributes.
    size_t           idle_timeout_;     ///< Idle timeout in milliseconds; 0 keeps workers.
    size_t           max_tasks_;        ///< Maximum blocked submitters; 0 means unlimited.
    bool             non_blocking_;     ///< Reject submissions when overloaded.
    bool             has_thread_attr_;  ///< Non-zero when thread_attr_ is configured.

    ct_list_t  worker_head_[1];   ///< Idle worker list.
    ct_mutex_t worker_mutex_[1];  ///< Protects the worker list.
    ct_cond_t  worker_cond_[1];   ///< Signals worker availability.

    ct_thread_t monitor_thread_;  ///< Idle-worker monitor.
};

#define THPOOL_IS_CLOSED(pool_) ct_atomic_long_load(&(pool_)->status_.closed_)

/* Initialize pool configuration. */
static void thpool_config_init(ct_thpool_t* self, ct_thpool_config_t* attr);
/* Initialize pool status. */
static void thpool_status_init(ct_thpool_t* self, size_t size);
/* Acquire an idle worker or create one. */
static int thpool_retrieve_worker(ct_thpool_t* self, worker_t** worker);
/* Return a worker to the idle list or retire it. */
static bool thpool_revert_worker(ct_thpool_t* self, worker_t* worker);

/* Create a worker. */
static worker_t* thpool_worker_create(ct_thpool_t* self);
/* Worker thread entry point. */
static int thpool_worker_thread(void* arg);
/* Queue a task on a worker. */
static void thpool_worker_input(worker_t* worker, ct_thpool_routine_t routine, void* arg);
/* Stop a worker's queue. */
static void thpool_worker_finish(worker_t* worker);

/* Monitor thread: retire idle workers. */
static int thpool_monitor_thread(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_thpool_t* ct_thpool_create(size_t size, ct_thpool_config_t* config) {
    ct_thpool_t* self = (ct_thpool_t*)malloc(sizeof(ct_thpool_t));
    if (!self) { return NULL; }

    thpool_config_init(self, config);
    thpool_status_init(self, size);
    ct_list_init(self->worker_head_);
    ct_mutex_init(self->worker_mutex_);
    ct_cond_init(self->worker_cond_);

    if (self->idle_timeout_ > 0) {
        ct_thread_attr_t attr;
        ct_thread_attr_init(&attr);
        ct_thread_attr_set_stack_size(&attr, 1 * 1024);  // Monitor thread stack: 1 KiB.
        ct_thread_create(&self->monitor_thread_, &attr, thpool_monitor_thread, self);
        ct_thread_attr_destroy(&attr);
    }
    return self;
}

void ct_thpool_close(ct_thpool_t* self) {
    if (!self) { return; }
    if (THPOOL_IS_CLOSED(self)) { return; }
    ct_atomic_long_store(&self->status_.closed_, 1);

    if (self->idle_timeout_ > 0) { ct_thread_join(&self->monitor_thread_, NULL); }

    ct_list_t stale_head[1];
    ct_list_init(stale_head);

    ct_mutex_lock(self->worker_mutex_);
    ct_list_foreach_entry_safe(worker, self->worker_head_, worker_t, list_) {
        ct_list_remove(worker->list_);
        ct_list_append(stale_head, worker->list_);
        thpool_worker_finish(worker);
    }
    ct_mutex_unlock(self->worker_mutex_);

    ct_list_foreach_entry_safe(worker, stale_head, worker_t, list_) {
        ct_thread_join(&worker->thread_, NULL);
        ct_msgqueue_destroy(worker->tasks_);
        free(worker);
    }

    ct_mutex_lock(self->worker_mutex_);
    ct_cond_broadcast(self->worker_cond_);
    ct_mutex_unlock(self->worker_mutex_);
}

void ct_thpool_destroy(ct_thpool_t* self) {
    if (!self) { return; }
    ct_thpool_close(self);

    // Wait for all worker threads to exit.
    ct_mutex_lock(self->worker_mutex_);
    while (ct_atomic_long_load(&self->status_.total_size_) > 0) {
        ct_cond_wait_for(self->worker_cond_, self->worker_mutex_, 1000);
    }
    ct_mutex_unlock(self->worker_mutex_);
    ct_mutex_destroy(self->worker_mutex_);
    ct_cond_destroy(self->worker_cond_);

    if (self->has_thread_attr_) { ct_thread_attr_destroy(&self->thread_attr_); }
    free(self);
}

int ct_thpool_submit(ct_thpool_t* self, ct_thpool_routine_t routine, void* arg) {
    if (!self) { return CT_THPOOL_ERROR_CLOSED; }
    if (!routine) { return CT_THPOOL_ERROR_TASK_NULL; }
    if (THPOOL_IS_CLOSED(self)) { return CT_THPOOL_ERROR_CLOSED; }

    worker_t* worker = NULL;
    const int ret    = thpool_retrieve_worker(self, &worker);
    if (ret == 0) { thpool_worker_input(worker, routine, arg); }
    return ret;
}

const char* ct_thpool_strerror(int error_code) {
    const char* descriptions[] = {
#define F(code, name, description) description,
        CT_THPOOL_ERROR_FOREACH(F)
#undef F
    };
    if (error_code >= CT_THPOOL_ERROR_MAX) { return "unknown error"; }
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

static void thpool_config_init(ct_thpool_t* self, ct_thpool_config_t* attr) {
    self->idle_timeout_    = 1000;
    self->non_blocking_    = false;
    self->max_tasks_       = 1000;
    self->has_thread_attr_ = false;

    if (attr) {
        if (attr->thread_attr_) {
            memcpy(&self->thread_attr_, attr->thread_attr_, sizeof(ct_thread_attr_t));
            self->has_thread_attr_ = true;
        }
        self->idle_timeout_ = attr->idle_timeout_ == 0 ? 0 : CT_MAX(attr->idle_timeout_, 50U);
        self->non_blocking_ = attr->non_blocking_;
        self->max_tasks_    = attr->max_tasks_;
    }
}

static void thpool_status_init(ct_thpool_t* self, size_t size) {
    status_t* status    = &self->status_;
    status->capacity_   = CT_ATOMIC_VAR_INIT(size);
    status->total_size_ = CT_ATOMIC_VAR_INIT(0);
    status->wait_size_  = CT_ATOMIC_VAR_INIT(0);
    status->closed_     = CT_ATOMIC_VAR_INIT(0);
}

static int thpool_retrieve_worker(ct_thpool_t* self, worker_t** worker) {
    if (!self || !worker) { return CT_THPOOL_ERROR_CLOSED; }
    ct_mutex_lock(self->worker_mutex_);

Retry:
    if (!ct_list_isempty(self->worker_head_)) {
        *worker = ct_list_first_entry(self->worker_head_, worker_t, list_);
        if (*worker) {
            ct_list_remove((*worker)->list_);
            ct_mutex_unlock(self->worker_mutex_);
            return 0;
        }
    }

    const long capacity = ct_atomic_long_load(&self->status_.capacity_);
    if (capacity == 0 || capacity > ct_atomic_long_load(&self->status_.total_size_)) {
        ct_mutex_unlock(self->worker_mutex_);
        *worker = thpool_worker_create(self);
        if (!*worker) { return CT_THPOOL_ERROR_MEM_ALLOC; }
        return 0;
    }

    if (self->non_blocking_ ||
        (self->max_tasks_ > 0 && ct_atomic_long_load(&self->status_.wait_size_) >= (long)self->max_tasks_)) {
        ct_mutex_unlock(self->worker_mutex_);
        return CT_THPOOL_ERROR_OVERLOAD;
    }

    ct_atomic_long_add(&self->status_.wait_size_, 1);
    ct_cond_wait(self->worker_cond_, self->worker_mutex_);
    ct_atomic_long_sub(&self->status_.wait_size_, 1);

    if (THPOOL_IS_CLOSED(self)) {
        ct_mutex_unlock(self->worker_mutex_);
        return CT_THPOOL_ERROR_CLOSED;
    }

    goto Retry;
}

static bool thpool_revert_worker(ct_thpool_t* self, worker_t* worker) {
    if (THPOOL_IS_CLOSED(self)) { return false; }
    const long capacity = ct_atomic_long_load(&self->status_.capacity_);
    if ((capacity > 0 && capacity <= ct_atomic_long_load(&self->status_.total_size_))) {
        ct_mutex_lock(self->worker_mutex_);
        ct_cond_broadcast(self->worker_cond_);
        ct_mutex_unlock(self->worker_mutex_);
        return false;
    }

    worker->last_use_ = ct_gettick_ms();

    ct_mutex_lock(self->worker_mutex_);
    if (THPOOL_IS_CLOSED(self)) {
        ct_mutex_unlock(self->worker_mutex_);
        return false;
    }
    ct_list_append(self->worker_head_, worker->list_);
    ct_cond_signal(self->worker_cond_);
    ct_mutex_unlock(self->worker_mutex_);
    return true;
}

static worker_t* thpool_worker_create(ct_thpool_t* self) {
    worker_t* worker = (worker_t*)malloc(sizeof(worker_t));
    if (!worker) { return NULL; }
    ct_list_init(worker->list_);
    worker->thpool_   = self;
    worker->last_use_ = ct_gettick_ms();
    ct_msgqueue_init(worker->tasks_, worker->task_buff_, sizeof(task_t), 1);

    const int ret = ct_thread_create(&worker->thread_, self->has_thread_attr_ ? &self->thread_attr_ : NULL,
                                     thpool_worker_thread, worker);
    if (ret != 0) {
        ct_msgqueue_destroy(worker->tasks_);
        free(worker);
        return NULL;
    }
    ct_atomic_long_add(&self->status_.total_size_, 1);
    return worker;
}

static int thpool_worker_thread(void* arg) {
    worker_t* worker = (worker_t*)arg;
    if (!worker) { return 0; }
    ct_thpool_t* pool = worker->thpool_;
    if (!pool) { return 0; }

    task_t task;
    while (ct_msgqueue_pop(worker->tasks_, &task) == 0) {
        task.routine_(task.arg_);
        if (!thpool_revert_worker(worker->thpool_, worker)) {
            ct_thread_detach(&worker->thread_);
            ct_msgqueue_destroy(worker->tasks_);
            free(worker);
            break;
        }
    }

    ct_atomic_long_sub(&pool->status_.total_size_, 1);
    ct_mutex_lock(pool->worker_mutex_);
    ct_cond_signal(pool->worker_cond_);
    ct_mutex_unlock(pool->worker_mutex_);

    return 0;
}

static void thpool_worker_input(worker_t* worker, ct_thpool_routine_t routine, void* arg) {
    if (!worker || !routine) { return; }
    const task_t task = THPOOL_TASK_INIT(routine, arg);
    ct_msgqueue_push(worker->tasks_, &task);
}

static void thpool_worker_finish(worker_t* worker) {
    if (!worker) { return; }
    ct_msgqueue_close(worker->tasks_);
}

static int thpool_monitor_thread(void* arg) {
    ct_thpool_t* pool = (ct_thpool_t*)arg;
    ct_time64_t  now, last = ct_gettick_ms();

    while (!THPOOL_IS_CLOSED(pool)) {
        now = ct_gettick_ms();

        if (now >= last + (ct_time64_t)pool->idle_timeout_) {
            last = now;

            const ct_time64_t expiry_time = now - pool->idle_timeout_;
            const size_t      total_size  = ct_atomic_long_load(&pool->status_.total_size_);

            ct_list_t stale_head[1];
            ct_list_init(stale_head);

            // Collect workers that have been idle past the timeout.
            ct_mutex_lock(pool->worker_mutex_);
            ct_list_foreach_entry_safe(worker, pool->worker_head_, worker_t, list_) {
                if (worker->last_use_ < expiry_time) {
                    ct_list_remove(worker->list_);
                    ct_list_append(stale_head, worker->list_);
                    thpool_worker_finish(worker);
                }
            }
            ct_mutex_unlock(pool->worker_mutex_);

            size_t stale_size = 0;
            ct_list_foreach_entry_safe(worker, stale_head, worker_t, list_) {
                ct_thread_join(&worker->thread_, NULL);
                ct_msgqueue_destroy(worker->tasks_);
                free(worker);
                ++stale_size;
            }

            // Wake a worker if every thread expired while tasks are waiting.
            if ((total_size == 0 || total_size == stale_size) && ct_atomic_long_load(&pool->status_.wait_size_) > 0) {
                ct_cond_broadcast(pool->worker_cond_);
            }
        }

        ct_msleep(50);
    }

    return 0;
}
