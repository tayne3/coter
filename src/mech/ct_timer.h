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

#include "base/ct_types.h"
#include "common/ct_any.h"
#include "common/ct_time.h"

// 定时器id类型
typedef uint64_t ct_timer_id_t;
// 定时器回调函数类型
typedef void (*ct_timer_callback_t)(ct_timer_id_t, const ct_any_buf_t);
// 复杂定时器-计算下次触发时间的时间戳函数
typedef ct_timestamp_t (*ct_timer_caculate_t)(const ct_datetime_buf_t param, const ct_datetime_buf_t curr);

/**
 * @brief 定时器管理初始化
 */
void ct_timer_mgr_init(void);

/**
 * @brief 定时器调度
 */
void ct_timer_mgr_schedule(void);

/**
 * @brief 启动定时器 (一次性定时器)
 * @param interval 触发间隔 (s)
 * @param callback 回调函数
 * @param arg 回调参数
 * @return ct_timer_id_t 定时器id
 */
ct_timer_id_t ct_timer_start_oneoff(ct_timestamp_t interval, ct_timer_callback_t callback, ct_any_t arg);

/**
 * @brief 启动定时器 (周期定时器)
 * @param interval 触发间隔 (s)
 * @param isnow 是否立即触发
 * @param callback 回调函数
 * @param arg 回调参数
 * @return ct_timer_id_t 定时器id
 */
ct_timer_id_t ct_timer_start_periodic(ct_timestamp_t interval, bool isnow, ct_timer_callback_t callback, ct_any_t arg);

/**
 * @brief 启动定时器 (精确定时器)
 * @param interval 触发间隔 (ms)
 * @param isnow 是否立即触发
 * @param isloop 是否循环触发
 * @param callback 回调函数
 * @param arg 回调参数
 * @return ct_timer_id_t 定时器id
 */
ct_timer_id_t ct_timer_start_precision(uint64_t interval, bool isnow, bool isloop, ct_timer_callback_t callback,
									   ct_any_t arg);

/**
 * @brief 启动定时器 (指定日期时间定时器)
 * @param datetime 指定日期时间
 * @param callback 回调函数
 * @param arg 回调参数
 * @return ct_timer_id_t 定时器id
 */
ct_timer_id_t ct_timer_start_schedule(ct_datetime_t datetime, ct_timer_callback_t callback, ct_any_t arg);

/**
 * @brief 启动定时器 (自定义定时器)
 * @param param 过滤参数
 * @param caculate 计算距离下次触发时间的时间戳
 * @param isloop 是否循环触发
 * @param callback 回调函数
 * @param arg 回调参数
 * @return ct_timer_id_t 定时器id
 */
ct_timer_id_t ct_timer_start_custom(ct_datetime_t param, ct_timer_caculate_t caculate, bool isloop,
									ct_timer_callback_t callback, ct_any_t arg);

/**
 * @brief 停止定时器
 * @param id 定时器id
 */
void ct_timer_stop(ct_timer_id_t id);

#ifdef __cplusplus
}
#endif
#endif  // _CT_TIMER_H
