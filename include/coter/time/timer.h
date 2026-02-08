/**
 * @file timer.h
 * @brief 软件定时器实现
 */
#ifndef COTER_TIME_TIMER_H
#define COTER_TIME_TIMER_H

#include "coter/core/platform.h"
#include "coter/time/time.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ct_thpool;

// 定时器id类型
typedef uint32_t ct_timer_id_t;
// 无效的定时器id
#define CT_TIMER_ID_INVALID 0
// 定时器回调函数类型
typedef void (*ct_timer_callback_t)(void *);

/**
 * @brief 定时器管理初始化
 * @param tick 运行时间 (ms)
 * @param thpool 任务池
 * @return int 成功返回0，失败返回非0值
 */
COTER_API int ct_timer_mgr_init(ct_time64_t tick, struct ct_thpool *thpool);

/**
 * @brief 定时器调度
 * @param tick 运行时间 (ms)
 * @return bool 是否有定时器被触发
 */
COTER_API bool ct_timer_mgr_schedule(ct_time64_t tick);

/**
 * @brief 启动定时器
 * @param interval 触发间隔 (ms)
 * @param is_loop 是否循环触发
 * @param is_now 是否立即触发
 * @param callback 回调函数
 * @param arg 回调参数
 * @return ct_timer_id_t 定时器id
 */
COTER_API ct_timer_id_t ct_timer_start(ct_time64_t interval, bool is_loop, bool is_now, ct_timer_callback_t callback,
									   void *arg);

/**
 * @brief 停止定时器
 * @param id 定时器id
 */
COTER_API void ct_timer_stop(ct_timer_id_t id);

#ifdef __cplusplus
}
#endif
#endif  // COTER_TIME_TIMER_H
