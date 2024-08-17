/**
 * @file ct_timer.h
 * @brief 软件定时器实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_TIMER_H
#define _CT_TIMER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"
#include "base/ct_time.h"
#include "common/ct_any.h"

// 定时器id类型
typedef uint64_t ct_timer_id_t;
// 定时器回调函数类型
typedef void (*ct_timer_callback_t)(ct_timer_id_t, const ct_any_buf_t);

/**
 * @brief 定时器管理初始化
 */
COTER_API void ct_timer_mgr_init(void);

/**
 * @brief 定时器调度
 * @param now 当前时间 (ms)
 * @param tick 运行时间 (ms)
 */
COTER_API void ct_timer_mgr_schedule(ct_time64_t now, ct_time64_t tick);

/**
 * @brief 启动定时器
 * @param interval 触发间隔 (ms)
 * @param isnow 是否立即触发
 * @param isloop 是否循环触发
 * @param callback 回调函数
 * @param arg 回调参数
 * @return ct_timer_id_t 定时器id
 */
COTER_API ct_timer_id_t ct_timer_start(ct_time64_t interval, bool isnow, bool isloop, ct_timer_callback_t callback,
									   ct_any_t arg);

/**
 * @brief 停止定时器
 * @param id 定时器id
 */
COTER_API void ct_timer_stop(ct_timer_id_t id);

#ifdef __cplusplus
}
#endif
#endif  // _CT_TIMER_H
