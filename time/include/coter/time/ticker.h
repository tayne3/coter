/**
 * @file ticker.h
 * @brief 周期定时器 (Ticker)
 */
#ifndef COTER_TIME_TICKER_H
#define COTER_TIME_TICKER_H

#include "coter/time/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ct_ticker_callback_t)(void*);

typedef struct ct_ticker {
    CT_TIMER_BASE
    ct_time64_t interval;
} ct_ticker_t;

/**
 * @brief 初始化定时器
 * @param ticker 定时器指针
 */
CT_API void ct_ticker_init(ct_ticker_t* ticker);

/**
 * @brief 启动定时器
 * @param ticker 定时器指针
 * @param ms 周期间隔 (ms)
 * @param cb 回调函数
 * @param arg 回调参数
 */
CT_API int ct_ticker_start(ct_ticker_t* ticker, ct_time64_t ms, ct_ticker_callback_t cb, void* arg);

/**
 * @brief 重置定时器
 * @param ticker 定时器指针
 * @param ms 周期间隔 (ms)
 */
CT_API int ct_ticker_reset(ct_ticker_t* ticker, ct_time64_t ms);

/**
 * @brief 停止定时器
 * @param ticker 定时器指针
 */
CT_API int ct_ticker_stop(ct_ticker_t* ticker);

#ifdef __cplusplus
}
#endif
#endif  // COTER_TIME_TICKER_H
