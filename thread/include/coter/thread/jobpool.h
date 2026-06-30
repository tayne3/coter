/**
 * @file jobpool.h
 * @brief 任务池实现
 */
#ifndef COTER_THREAD_JOBPOOL_H
#define COTER_THREAD_JOBPOOL_H

#include "coter/core/macro.h"
#include "coter/core/time.h"
#include "coter/thread/thread.h"

#ifdef __cplusplus
extern "C" {
#endif

// 任务池执行函数
typedef void (*ct_jobpool_routine_t)(void*);
// 任务池
typedef struct ct_jobpool ct_jobpool_t;

// 任务池配置
typedef struct ct_jobpool_config {
    ct_thread_attr_t* thread_attr;  // 线程属性 (为空时使用默认值)

    // 连续执行 N 个任务后主动让出 CPU 时间片 (0=禁用，依赖 OS 抢占)
    // 适用于密集微任务场景，防止工作线程长时间独占 CPU 核心。
    size_t yield_every;
} ct_jobpool_config_t;

/**
 * @brief 创建一个新的任务池
 *
 * @param thread_max 工作线程数量
 * @param job_max 工作队列的最大容量
 * @param config 任务池配置 (NULL 则使用默认配置)
 * @return 返回新创建的任务池指针；所有线程均创建失败或参数无效时返回 NULL
 */
CT_API ct_jobpool_t* ct_jobpool_create(size_t thread_max, size_t job_max,
                                       const ct_jobpool_config_t* config) CT_ATTR_THROW;

/**
 * @brief 销毁任务池
 *
 * @param self 需要销毁的任务池指针, 不能为空指针
 */
CT_API void ct_jobpool_destroy(ct_jobpool_t* self);

/**
 * @brief 向任务池中添加一个工作（阻塞版本）
 *
 * 若队列已满则阻塞调用方，直到有空位或任务池关闭。
 *
 * @param self 任务池指针
 * @param routine 执行函数
 * @param arg 执行参数
 * @return 0=成功, 非0=失败
 */
CT_API int ct_jobpool_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg);

/**
 * @brief 向任务池中添加一个工作（非阻塞版本）
 *
 * 若队列已满则立即返回 -1，不阻塞调用方。
 *
 * @param self 任务池指针
 * @param routine 执行函数
 * @param arg 执行参数
 * @return 0=成功, -1=队列已满或参数无效
 */
CT_API int ct_jobpool_try_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg);

/**
 * @brief 向任务池中添加一个工作（超时版本）
 *
 * 若队列已满则等待至多 timeout_ms 毫秒；超时后返回 -1。
 *
 * @param self 任务池指针
 * @param routine 执行函数
 * @param arg 执行参数
 * @param timeout_ms 等待时间 (ms)；0=立即返回；负数=无限等待
 * @return 0=成功, 非0=失败
 */
CT_API int ct_jobpool_submit_for(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg, ct_time64_t timeout_ms);

/**
 * @brief 获取当前队列中待执行的任务数
 *
 * @param self 任务池指针
 * @return 当前积压的任务数量
 */
CT_API size_t ct_jobpool_pending(ct_jobpool_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_JOBPOOL_H
