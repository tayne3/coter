/**
 * @file thpool.c
 * @brief 线程池实现
 */
#include "coter/thread/thpool.h"

#include "coter/container/list.h"
#include "coter/event/msgqueue.h"
#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 工作任务
 */
typedef struct task {
    ct_thpool_routine_t routine;  // 执行函数
    void*               arg;      // 执行参数
} task_t;

#define TASK_INIT(routine, arg) {routine, arg}

/**
 * @brief 工作者
 */
typedef struct worker {
    ct_list_t     list[1];       // 链表节点
    ct_thread_t   thread;        // 线程
    ct_thpool_t*  thpool;        // 所属线程池
    task_t        task_buff[1];  // 任务缓存
    ct_msgqueue_t tasks[1];      // 任务队列
    ct_time64_t   last_use;      // 上次活动时间 (ms)
} worker_t;

/**
 * @brief 线程池状态
 */
typedef struct status {
    ct_atomic_long_t capacity;    // 最大线程数 (为0表示不限制)
    ct_atomic_long_t total_size;  // 总线程数
    ct_atomic_long_t wait_size;   // 等待任务数
    ct_atomic_long_t closed;      // 是否关闭
} status_t;

/**
 * @brief 线程池
 */
struct ct_thpool {
    ct_thpool_config_t config;  // 线程池属性
    status_t           status;  // 线程池状态

    ct_list_t  worker_head[1];   // 工作者队列
    ct_mutex_t worker_mutex[1];  // 互斥锁
    ct_cond_t  worker_cond[1];   // 条件变量

    ct_thread_t monitor_thread;  // 监视线程
};

#define ctl_is_closed(pool) ct_atomic_long_load(&(pool)->status.closed)

// 初始化 线程池属性
static void ctl_config_init(ct_thpool_t* self, ct_thpool_config_t* attr);
// 初始化 线程池状态
static void ctl_status_init(ct_thpool_t* self, size_t size);
// 获取工作者
static int ctl_retrieve_worker(ct_thpool_t* self, worker_t** worker);
// 回收工作者
static bool ctl_revert_worker(ct_thpool_t* self, worker_t* worker);

// 工作者-创建
static worker_t* ctl_worker_create(ct_thpool_t* self);
// 工作者-线程执行函数
static int ctl_worker_thread(void* arg);
// 工作者-输入任务
static void ctl_worker_input(worker_t* worker, ct_thpool_routine_t routine, void* arg);
// 工作者-结束
static void ctl_worker_finish(worker_t* worker);

// 监视器-动态调整线程数量
static int ctl_monitor_thread(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_thpool_t* ct_thpool_create(size_t size, ct_thpool_config_t* config) {
    ct_thpool_t* self = (ct_thpool_t*)malloc(sizeof(ct_thpool_t));
    if (!self) { return NULL; }

    ctl_config_init(self, config);
    ctl_status_init(self, size);
    ct_list_init(self->worker_head);
    ct_mutex_init(self->worker_mutex);
    ct_cond_init(self->worker_cond);

    if (self->config.idle_timeout > 0) {
        ct_thread_attr_t attr;
        ct_thread_attr_init(&attr);
        ct_thread_attr_set_stack_size(&attr, 1 * 1024);  // 设置堆栈大小: 1KB
        ct_thread_create(&self->monitor_thread, &attr, ctl_monitor_thread, self);
        ct_thread_attr_destroy(&attr);
    }
    return self;
}

void ct_thpool_close(ct_thpool_t* self) {
    if (!self) { return; }
    if (ctl_is_closed(self)) { return; }
    ct_atomic_long_store(&self->status.closed, 1);

    if (self->config.idle_timeout > 0) { ct_thread_join(self->monitor_thread, NULL); }

    ct_list_t stale_head[1];
    ct_list_init(stale_head);

    ct_mutex_lock(self->worker_mutex);
    ct_list_foreach_entry_safe(worker, self->worker_head, worker_t, list) {
        ct_list_remove(worker->list);
        ct_list_append(stale_head, worker->list);
        ctl_worker_finish(worker);
    }
    ct_mutex_unlock(self->worker_mutex);

    ct_list_foreach_entry_safe(worker, stale_head, worker_t, list) {
        ct_thread_join(worker->thread, NULL);
        ct_msgqueue_destroy(worker->tasks);
        free(worker);
    }

    ct_mutex_lock(self->worker_mutex);
    ct_cond_broadcast(self->worker_cond);
    ct_mutex_unlock(self->worker_mutex);
}

void ct_thpool_destroy(ct_thpool_t* self) {
    if (!self) { return; }
    ct_thpool_close(self);

    while (ct_atomic_long_load(&self->status.total_size) > 0) { ct_msleep(10); }
    ct_mutex_destroy(self->worker_mutex);
    ct_cond_destroy(self->worker_cond);

    if (self->config.thread_attr) {
        ct_thread_attr_destroy(self->config.thread_attr);
        free(self->config.thread_attr);
        self->config.thread_attr = NULL;
    }
    free(self);
}

int ct_thpool_submit(ct_thpool_t* self, ct_thpool_routine_t routine, void* arg) {
    if (!self) { return CTThPoolError_Closed; }
    if (!routine) { return CTThPoolError_TaskNull; }
    if (ctl_is_closed(self)) { return CTThPoolError_Closed; }

    worker_t* worker = NULL;
    const int ret    = ctl_retrieve_worker(self, &worker);
    if (ret == 0) { ctl_worker_input(worker, routine, arg); }
    return ret;
}

const char* ct_thpool_strerror(int error_code) {
    const char* CTThPoolErrorDescriptions[] = {
#define F(code, name, desc) desc,
        CT_THPOOL_ERROR_FOREACH(F)
#undef F
    };
    if (error_code >= CTThPoolError_Max) { return "unknown error"; }
    return CTThPoolErrorDescriptions[error_code];
}

void ct_thpool_default_config(ct_thpool_config_t* config) {
    if (!config) { return; }
    config->thread_attr  = NULL;
    config->idle_timeout = 1000;
    config->non_blocking = false;
    config->max_tasks    = 1000;
}

// -------------------------[STATIC DEFINITION]-------------------------

static void ctl_config_init(ct_thpool_t* self, ct_thpool_config_t* attr) {
    ct_thpool_config_t* sattr = &self->config;
    if (attr && attr->thread_attr) {
        sattr->thread_attr = (ct_thread_attr_t*)malloc(sizeof(ct_thread_attr_t));
        memcpy(sattr->thread_attr, attr->thread_attr, sizeof(ct_thread_attr_t));
    } else {
        sattr->thread_attr = NULL;
    }

    if (attr && attr->idle_timeout) {
        sattr->idle_timeout = CT_MIN(attr->idle_timeout, 50U);
    } else {
        sattr->idle_timeout = 1000;
    }

    if (attr) {
        sattr->non_blocking = attr->non_blocking;
    } else {
        sattr->non_blocking = false;
    }

    if (attr && attr->max_tasks) {
        sattr->max_tasks = attr->max_tasks;
    } else {
        sattr->max_tasks = 1000;
    }
}

static void ctl_status_init(ct_thpool_t* self, size_t size) {
    status_t* sstatus   = &self->status;
    sstatus->capacity   = CT_ATOMIC_VAR_INIT(size);
    sstatus->total_size = CT_ATOMIC_VAR_INIT(0);
    sstatus->wait_size  = CT_ATOMIC_VAR_INIT(0);
    sstatus->closed     = CT_ATOMIC_VAR_INIT(0);
}

static int ctl_retrieve_worker(ct_thpool_t* self, worker_t** worker) {
    if (!self || !worker) { return CTThPoolError_Closed; }
    ct_mutex_lock(self->worker_mutex);

Retry:
    if (!ct_list_isempty(self->worker_head)) {
        *worker = ct_list_first_entry(self->worker_head, worker_t, list);
        if (*worker) {
            ct_list_remove((*worker)->list);
            ct_mutex_unlock(self->worker_mutex);
            return 0;
        }
    }

    const long capacity = ct_atomic_long_load(&self->status.capacity);
    if (capacity == 0 || capacity > ct_atomic_long_load(&self->status.total_size)) {
        ct_mutex_unlock(self->worker_mutex);
        *worker = ctl_worker_create(self);
        if (!*worker) { return CTThPoolError_MemAlloc; }
        return 0;
    }

    if (self->config.non_blocking ||
        (self->config.max_tasks > 0 && ct_atomic_long_load(&self->status.wait_size) >= (long)self->config.max_tasks)) {
        ct_mutex_unlock(self->worker_mutex);
        return CTThPoolError_Overload;
    }

    ct_atomic_long_add(&self->status.wait_size, 1);
    ct_cond_wait(self->worker_cond, self->worker_mutex);
    ct_atomic_long_sub(&self->status.wait_size, 1);

    if (ctl_is_closed(self)) {
        ct_mutex_unlock(self->worker_mutex);
        return CTThPoolError_Closed;
    }

    goto Retry;
}

static bool ctl_revert_worker(ct_thpool_t* self, worker_t* worker) {
    if (ctl_is_closed(self)) { return false; }
    const long capacity = ct_atomic_long_load(&self->status.capacity);
    if ((capacity > 0 && capacity <= ct_atomic_long_load(&self->status.total_size))) {
        ct_mutex_lock(self->worker_mutex);
        ct_cond_broadcast(self->worker_cond);
        ct_mutex_unlock(self->worker_mutex);
        return false;
    }

    worker->last_use = ct_getuptime_ms();

    ct_mutex_lock(self->worker_mutex);
    if (ctl_is_closed(self)) {
        ct_mutex_unlock(self->worker_mutex);
        return false;
    }
    ct_list_append(self->worker_head, worker->list);
    ct_cond_signal(self->worker_cond);
    ct_mutex_unlock(self->worker_mutex);
    return true;
}

static worker_t* ctl_worker_create(ct_thpool_t* self) {
    worker_t* worker = (worker_t*)malloc(sizeof(worker_t));
    if (!worker) { return NULL; }
    ct_list_init(worker->list);
    worker->thpool   = self;
    worker->last_use = ct_getuptime_ms();
    ct_msgqueue_init(worker->tasks, worker->task_buff, sizeof(task_t), 1);

    const int ret = ct_thread_create(&worker->thread, self->config.thread_attr, ctl_worker_thread, worker);
    if (ret != 0) {
        ct_msgqueue_destroy(worker->tasks);
        free(worker);
        return NULL;
    }
    ct_atomic_long_add(&self->status.total_size, 1);
    return worker;
}

static int ctl_worker_thread(void* arg) {
    worker_t* worker = (worker_t*)arg;
    if (!worker) { return 0; }
    ct_thpool_t* pool = worker->thpool;
    if (!pool) { return 0; }

    task_t task;
    while (ct_msgqueue_pop(worker->tasks, &task) == 0) {
        task.routine(task.arg);
        if (!ctl_revert_worker(worker->thpool, worker)) {
            ct_thread_detach(worker->thread);
            ct_msgqueue_destroy(worker->tasks);
            free(worker);
            break;
        }
    }

    ct_atomic_long_sub(&pool->status.total_size, 1);
    ct_mutex_lock(pool->worker_mutex);
    ct_cond_signal(pool->worker_cond);
    ct_mutex_unlock(pool->worker_mutex);

    return 0;
}

static void ctl_worker_input(worker_t* worker, ct_thpool_routine_t routine, void* arg) {
    if (!worker || !routine) { return; }
    const task_t task = TASK_INIT(routine, arg);
    ct_msgqueue_push(worker->tasks, &task);
}

static void ctl_worker_finish(worker_t* worker) {
    if (!worker) { return; }
    ct_msgqueue_close(worker->tasks);
}

static int ctl_monitor_thread(void* arg) {
    ct_thpool_t* pool = (ct_thpool_t*)arg;
    ct_time64_t  now, last = ct_getuptime_ms();

    while (!ctl_is_closed(pool)) {
        now = ct_getuptime_ms();

        if (now >= last + (ct_time64_t)pool->config.idle_timeout) {
            last = now;

            const ct_time64_t expiry_time = now - pool->config.idle_timeout;
            const size_t      total_size  = ct_atomic_long_load(&pool->status.total_size);

            ct_list_t stale_head[1];
            ct_list_init(stale_head);

            // 遍历工作者队列，找出空闲超时的工作者
            ct_mutex_lock(pool->worker_mutex);
            ct_list_foreach_entry_safe(worker, pool->worker_head, worker_t, list) {
                if (worker->last_use < expiry_time) {
                    ct_list_remove(worker->list);
                    ct_list_append(stale_head, worker->list);
                    ctl_worker_finish(worker);
                }
            }
            ct_mutex_unlock(pool->worker_mutex);

            size_t stale_size = 0;
            ct_list_foreach_entry_safe(worker, stale_head, worker_t, list) {
                ct_thread_join(worker->thread, NULL);
                ct_msgqueue_destroy(worker->tasks);
                free(worker);
                stale_size++;
            }

            // 如果所有线程都是过期的且有等待任务，则通知工作者
            if ((total_size == 0 || total_size == stale_size) && ct_atomic_long_load(&pool->status.wait_size) > 0) {
                ct_cond_broadcast(pool->worker_cond);
            }
        }

        ct_msleep(50);
    }

    return 0;
}
