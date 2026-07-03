/**
 * @file timer.h
 * @brief software timer
 */
#ifndef COTER_TIME_TIMER_H
#define COTER_TIME_TIMER_H

#include "coter/container/heap.h"
#include "coter/core/macro.h"
#include "coter/core/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run the timer manager. Blocks until ct_timer_mgr_shutdown() is called.
 */
CT_API void ct_timer_mgr_run(void);

/**
 * @brief Stop the timer manager and release all pending timers.
 */
CT_API void ct_timer_mgr_shutdown(void);

typedef void (*ct_timer_callback_t)(void*);

#define CT_TIMER_BASE                  \
    ct_heap_node_t      node;          \
    ct_timer_callback_t cb;            \
    void*               arg;           \
    ct_time64_t         next_time;     \
    uint64_t            type : 2;      \
    uint64_t            is_active : 1; \
    uint64_t            is_queued : 1; \
    uint64_t            reserved : 60;

typedef struct ct_timer {
    CT_TIMER_BASE
} ct_timer_t;

#define CT_TIMER_INITIALIZER {{NULL, NULL, NULL}, NULL, NULL, 0, 0, 0, 0, 0}

/**
 * @brief Initialize a timer.
 * @param timer Pointer to the timer.
 */
CT_API void ct_timer_init(ct_timer_t* timer);

/**
 * @brief Start a one-shot timer.
 * @param timer      Pointer to the timer.
 * @param timeout_ms Timeout in milliseconds.
 * @param cb         Callback invoked on expiry.
 * @param arg        User argument passed to the callback.
 * @return 0 on success, -1 on failure.
 */
CT_API int ct_timer_start(ct_timer_t* timer, ct_time64_t timeout_ms, ct_timer_callback_t cb, void* arg);

/**
 * @brief Restart a timer with a new timeout, keeping the existing callback.
 * @param timer      Pointer to the timer.
 * @param timeout_ms New timeout in milliseconds.
 * @return 0 on success, -1 if the timer has no callback set.
 */
CT_API int ct_timer_reset(ct_timer_t* timer, ct_time64_t timeout_ms);

/**
 * @brief Cancel a pending timer.
 * @param timer Pointer to the timer.
 * @return 0 on success, -1 if the timer is not active.
 */
CT_API int ct_timer_stop(ct_timer_t* timer);

/**
 * @brief Schedule a fire-and-forget timeout.
 *        The timer is allocated internally and freed automatically on expiry.
 * @param timeout_ms Timeout in milliseconds.
 * @param cb         Callback invoked on expiry.
 * @param arg        User argument passed to the callback.
 * @return 0 on success, -1 on failure.
 */
CT_API int ct_set_timeout(ct_time64_t timeout_ms, void (*cb)(void*), void* arg);

typedef void (*ct_ticker_callback_t)(void*);

typedef struct ct_ticker {
    CT_TIMER_BASE
    ct_time64_t interval;
} ct_ticker_t;

#define CT_TICKER_INITIALIZER {{NULL, NULL, NULL}, NULL, NULL, 0, 0, 0, 0, 0, 0}

/**
 * @brief Initialize a ticker.
 * @param ticker Pointer to the ticker.
 */
CT_API void ct_ticker_init(ct_ticker_t* ticker);

/**
 * @brief Start a periodic ticker.
 * @param ticker Pointer to the ticker.
 * @param ms     Interval in milliseconds.
 * @param cb     Callback invoked on each tick.
 * @param arg    User argument passed to the callback.
 * @return 0 on success, -1 on failure.
 */
CT_API int ct_ticker_start(ct_ticker_t* ticker, ct_time64_t ms, ct_ticker_callback_t cb, void* arg);

/**
 * @brief Change the interval of a running ticker.
 * @param ticker Pointer to the ticker.
 * @param ms     New interval in milliseconds.
 * @return 0 on success, -1 if the ticker has no callback set.
 */
CT_API int ct_ticker_reset(ct_ticker_t* ticker, ct_time64_t ms);

/**
 * @brief Stop a running ticker.
 * @param ticker Pointer to the ticker.
 * @return 0 on success, -1 if the ticker is not active.
 */
CT_API int ct_ticker_stop(ct_ticker_t* ticker);

#ifdef __cplusplus
}
#endif
#endif  // COTER_TIME_TIMER_H
