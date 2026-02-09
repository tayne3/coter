/**
 * @file jobpool.h
 * @brief 任务池实现
 */
#ifndef COTER_THREAD_JOBPOOL_H
#define COTER_THREAD_JOBPOOL_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// 任务池执行函数
typedef void (*ct_jobpool_routine_t)(void*);
// 任务池
typedef struct ct_jobpool ct_jobpool_t;

/**
 * @brief 创建一个新的任务池
 *
 * @param thread_max 任务队列的最大容量
 * @param job_max 工作队列的最大容量
 * @param attr 线程属性 (NULL 则使用默认属性)
 * @return 返回新创建的任务池指针，如果创建失败则返回空指针
 */
COTER_API ct_jobpool_t* ct_jobpool_create(size_t thread_max, size_t job_max) __ct_throw;

/**
 * @brief 销毁任务池
 *
 * @param self 需要销毁的任务池指针, 不能为空指针
 */
COTER_API void ct_jobpool_destroy(ct_jobpool_t* self);

/**
 * @brief 向任务池中添加一个工作
 *
 * @param self 任务池指针, 为空则添加到全局任务池
 * @param routine 执行函数
 * @param arg 执行参数
 */
COTER_API int ct_jobpool_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_JOBPOOL_H
