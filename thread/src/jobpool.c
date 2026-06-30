/**
 * @file jobpool.c
 * @brief 任务池实现
 */
#include "coter/thread/jobpool.h"

#include <stdlib.h>
#include <string.h>

#include "coter/container/list.h"
#include "coter/core/macro.h"
#include "coter/sync/msgqueue.h"
#include "coter/thread/thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 任务
 */
typedef struct job {
    ct_jobpool_routine_t routine;  // 执行函数
    void*                arg;      // 执行参数
} job_t;

#define CT_JOBPOOL_JOB_INIT(_routine, _arg) {.routine = _routine, .arg = _arg}

typedef struct unit {
    ct_list_t      list[1];      // 链表节点
    ct_msgqueue_t* job_queue;    // 工作队列（指向共享队列）
    ct_thread_t    thread;       // 线程
    size_t         yield_every;  // 连续执行 N 个任务后让出 CPU（0=禁用）
} unit_t;

/**
 * @brief 任务池
 */
struct ct_jobpool {
    job_t*        job_buffer;       // 工作队列缓冲区
    ct_msgqueue_t job_queue[1];     // 工作队列（所有线程共享）
    ct_list_t     regular_list[1];  // 工作线程链表
    size_t        thread_max;       // 线程数
    size_t        job_max;          // 工作队列容量
};

static int ct_jobpool_thread_do_regular(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_jobpool_t* ct_jobpool_create(size_t thread_max, size_t job_max, const ct_jobpool_config_t* config) {
    if (!thread_max || !job_max) { return NULL; }

    ct_jobpool_t* self = (ct_jobpool_t*)malloc(sizeof(ct_jobpool_t));
    if (!self) { return NULL; }

    self->thread_max = thread_max;
    self->job_max    = job_max;

    self->job_buffer = (job_t*)calloc(job_max, sizeof(job_t));
    if (!self->job_buffer) {
        free(self);
        return NULL;
    }
    ct_msgqueue_init(self->job_queue, self->job_buffer, sizeof(job_t), job_max);
    ct_list_init(self->regular_list);

    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    if (config && config->thread_attr) {
        memcpy(&attr, config->thread_attr, sizeof(ct_thread_attr_t));
    } else {
        ct_thread_attr_set_stack_size(&attr, 1 * 1024 * 1024);
    }

    for (size_t i = 0; i < thread_max; ++i) {
        unit_t* unit = (unit_t*)malloc(sizeof(unit_t));
        if (!unit) {
            ct_thread_attr_destroy(&attr);
            ct_jobpool_destroy(self);
            return NULL;  // 内存申请失败，视为整体失败
        }

        unit->job_queue   = self->job_queue;
        unit->yield_every = config ? config->yield_every : 0;
        ct_list_init(unit->list);

        const int ret = ct_thread_create(&unit->thread, &attr, ct_jobpool_thread_do_regular, unit);
        if (ret != 0) {
            free(unit);
            ct_thread_attr_destroy(&attr);
            ct_jobpool_destroy(self);
            return NULL;  // 线程创建失败，视为整体失败
        }
        ct_list_append(self->regular_list, unit->list);
    }

    ct_thread_attr_destroy(&attr);
    return self;
}

void ct_jobpool_destroy(ct_jobpool_t* self) {
    if (!self) { return; }

    // 关闭队列，唤醒所有阻塞中的工作线程
    ct_msgqueue_close(self->job_queue);

    // 等待所有线程退出
    ct_list_foreach_entry_safe(unit, self->regular_list, unit_t, list) {
        ct_thread_join(&unit->thread, NULL);
        ct_list_remove(unit->list);
        free(unit);
    }

    ct_msgqueue_destroy(self->job_queue);
    if (self->job_buffer) { free(self->job_buffer); }
    free(self);
}

int ct_jobpool_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg) {
    if (!self || !routine) { return -1; }
    const job_t job = CT_JOBPOOL_JOB_INIT(routine, arg);
    return ct_msgqueue_push(self->job_queue, &job) != 0 ? -1 : 0;
}

int ct_jobpool_try_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg) {
    if (!self || !routine) { return -1; }
    const job_t job = CT_JOBPOOL_JOB_INIT(routine, arg);
    return ct_msgqueue_try_push(self->job_queue, &job) != 0 ? -1 : 0;
}

int ct_jobpool_submit_for(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg, ct_time64_t timeout_ms) {
    if (!self || !routine) { return -1; }
    const job_t job = CT_JOBPOOL_JOB_INIT(routine, arg);
    return ct_msgqueue_push_for(self->job_queue, &job, timeout_ms) != 0 ? -1 : 0;
}

size_t ct_jobpool_pending(ct_jobpool_t* self) {
    if (!self) { return 0; }
    return ct_msgqueue_size(self->job_queue);
}

// -------------------------[STATIC DEFINITION]-------------------------

static int ct_jobpool_thread_do_regular(void* arg) {
    unit_t* unit   = (unit_t*)arg;
    size_t  consec = 0;

    job_t job;
    for (;;) {
        if (ct_msgqueue_try_pop(unit->job_queue, &job) != 0) {
            consec = 0;
            if (ct_msgqueue_pop(unit->job_queue, &job) != 0) { break; }
        } else {
            ++consec;
        }

        if (job.routine) { job.routine(job.arg); }

        if (unit->yield_every > 0 && consec >= unit->yield_every) {
            consec = 0;
            ct_thread_yield();
        }
    }

    return 0;
}
