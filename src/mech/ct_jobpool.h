/**
 * @file ct_jobpool.h
 * @brief 任务池实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#ifndef _CT_JOBPOOL_H
#define _CT_JOBPOOL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

/// 任务池执行函数
typedef void (*ct_jobpool_routine_t)(void*);
/// 任务池
typedef struct ct_thpool* ct_jobpool_ptr_t;

/**
 * @brief 创建一个新的任务池
 *
 * @param thread_max 任务队列的最大容量
 * @param job_max 工作队列的最大容量
 * @param attr 线程属性 (NULL 则使用默认属性)
 * @return 返回新创建的任务池指针，如果创建失败则返回空指针
 */
COTER_API ct_jobpool_ptr_t ct_jobpool_create(size_t thread_max, size_t job_max) __ct_func_throw;

/**
 * @brief 创建全局任务池
 *
 * @param thread_max 任务队列的最大容量
 * @param job_max 工作队列的最大容量
 * @return 返回全局任务池指针，如果创建失败则返回空指针, 如果全局任务池已存在则返回全局任务池
 * @note 为保证线程安全, 当前函数只建议在主线程调用
 */
COTER_API ct_jobpool_ptr_t ct_jobpool_global(size_t thread_max, size_t job_max) __ct_func_throw;

/**
 * @brief 销毁任务池
 *
 * @param self 需要销毁的任务池指针, 不能为空指针
 */
COTER_API void ct_jobpool_destroy(ct_jobpool_ptr_t self);

/**
 * @brief 向任务池中添加一个工作
 *
 * @param self 任务池指针, 为空则添加到全局任务池
 * @param routine 执行函数
 * @param arg 执行参数
 */
COTER_API void ct_jobpool_add(ct_jobpool_ptr_t self, ct_jobpool_routine_t routine, void* arg);

#ifdef __cplusplus
}
#endif
#endif  // _CT_JOBPOOL_H
