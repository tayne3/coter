/**
 * @file timer.h
 * @brief 一次性定时器 (Timer)
 */
#ifndef COTER_TIME_TIMER_H
#define COTER_TIME_TIMER_H

#include "coter/container/heap.h"
#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 运行定时器管理器
 * @param gettime_cb 时间获取回调 (为空则使用默认系统时间)
 * @note 阻塞运行, 使用 ct_timer_mgr_close 停止
 */
COTER_API void ct_timer_mgr_run(ct_time64_t (*gettime_cb)(void));

/**
 * @brief 停止定时器管理器
 */
COTER_API void ct_timer_mgr_close(void);

typedef void (*ct_timer_callback_t)(void *);

#define CT_TIMER_BASE                  \
	ct_heap_node_t      node;          \
	ct_time64_t         trigger_time;  \
	ct_timer_callback_t cb;            \
	void               *arg;           \
	uint64_t            type : 2;      \
	uint64_t            is_active : 1; \
	uint64_t            reserved : 61;

typedef struct ct_timer {
	CT_TIMER_BASE
} ct_timer_t;

/**
 * @brief 启动定时器
 * @param timer 定时器指针
 * @param timeout_ms 超时时间 (ms)
 * @param cb 回调函数
 * @param arg 回调参数
 */
COTER_API void ct_timer_start(ct_timer_t *timer, ct_time64_t timeout_ms, ct_timer_callback_t cb, void *arg);

/**
 * @brief 重置定时器
 * @param timer 定时器指针
 * @param timeout_ms 超时时间 (ms)
 */
COTER_API void ct_timer_reset(ct_timer_t *timer, ct_time64_t timeout_ms);

/**
 * @brief 停止定时器
 * @param timer 定时器指针
 */
COTER_API void ct_timer_stop(ct_timer_t *timer);

/**
 * @brief 创建并启动一个定时器
 * @param timeout_ms 超时时间 (ms)
 * @param cb 回调函数
 * @param arg 回调参数
 * @return 成功返回 0, 失败返回 -1
 * @note 到期自动关闭, 无法手动停止
 */
COTER_API int ct_set_timeout(ct_time64_t timeout_ms, void (*cb)(void *), void *arg);

#ifdef __cplusplus
}
#endif
#endif  // COTER_TIME_TIMER_H
