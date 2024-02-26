/**
 * @file ct_thpool.h
 * @brief 线程池实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#ifndef _CT_THPOOL_H
#define _CT_THPOOL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

/// 线程池执行函数
typedef void (*ct_thpool_routine_t)(void*);
/// 线程池
typedef struct ct_thpool* ct_thpool_ptr_t;

/**
 * @brief 创建一个新的线程池
 *
 * @param thread_max 任务队列的最大容量
 * @param job_max 工作队列的最大容量
 * @return 返回新创建的线程池指针，如果创建失败则返回空指针
 */
ct_thpool_ptr_t ct_thpool_create(size_t thread_max, size_t job_max) __ct_func_throw;

/**
 * @brief 创建全局线程池
 *
 * @param thread_max 任务队列的最大容量
 * @param job_max 工作队列的最大容量
 * @return 返回全局线程池指针，如果创建失败则返回空指针, 如果全局线程池已存在则返回全局线程池
 * @note 为保证线程安全, 当前函数只建议在主线程调用
 */
ct_thpool_ptr_t ct_thpool_global_create(size_t thread_max, size_t job_max) __ct_func_throw;

/**
 * @brief 销毁线程池
 *
 * @param self 需要销毁的线程池指针, 不能为空指针
 */
void ct_thpool_destroy(ct_thpool_ptr_t self);

/**
 * @brief 向线程池中添加一个工作
 *
 * @param self 线程池指针, 为空则添加到全局线程池
 * @param routine 执行函数
 * @param arg 执行参数
 */
void ct_thpool_add_job(ct_thpool_ptr_t self, ct_thpool_routine_t routine, void* arg);

/**
 * @brief 向线程池中添加一个常驻工作
 *
 * @param self 线程池指针，如果为空指针则添加到全局线程池
 * @param routine 执行函数
 * @param arg 执行参数
 */
void ct_thpool_add_task(ct_thpool_ptr_t self, ct_thpool_routine_t routine, void* arg);

#ifdef __cplusplus
}
#endif
#endif  // _CT_THPOOL_H
