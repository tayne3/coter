/**
 * @file cron.h
 * @brief Cron-style job scheduler.
 */
#ifndef COTER_TIME_CRON_H
#define COTER_TIME_CRON_H

#include "coter/container/heap.h"
#include "coter/core/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run the cron manager. Blocks until ct_cron_mgr_shutdown() is called.
 */
CT_API void ct_cron_mgr_run(void);

/**
 * @brief Stop the cron manager and release all scheduled jobs.
 */
CT_API void ct_cron_mgr_shutdown(void);

/**
 * @brief Compute the next fire time for a cron expression.
 *
 * Each field accepts a concrete value or -1 to mean "any". Fields take
 * priority in order from coarsest to finest: month → week → day → hour →
 * minute. Passing -1 for all fields schedules a minutely job.
 *
 * @param now    Current wall-clock time in seconds.
 * @param minute Minute of the hour  (0-59, or -1 for every minute).
 * @param hour   Hour of the day     (0-23, or -1 for every hour).
 * @param day    Day of the month    (1-31, or -1 for every day).
 * @param week   Day of the week     (0-6,  0 = Sunday, or -1 for every day).
 * @param month  Month of the year   (1-12, or -1 for every month).
 * @return Next fire time as a Unix timestamp (seconds), or -1 on invalid input.
 *
 * @note Common schedule examples:
 * | minute | hour | day | week | month | description     |
 * |--------|------|-----|------|-------|-----------------|
 * |  -1    |  -1  | -1  |  -1  |  -1   | every minute    |
 * |  30    |  -1  | -1  |  -1  |  -1   | every hour      |
 * |  30    |   1  | -1  |  -1  |  -1   | daily at 01:30  |
 * |  30    |   1  | 15  |  -1  |  -1   | monthly         |
 * |  30    |   1  | -1  |   0  |  -1   | weekly (Sunday) |
 * |  30    |   1  |  1  |  -1  |  10   | yearly (Oct 1)  |
 */
CT_API ct_time_t ct_cron_next_timeout(ct_time_t now, int minute, int hour, int day, int week, int month);

typedef ct_time64_t (*ct_cron_gettime_cb)(void);
typedef void (*ct_cron_callback_t)(void*);

typedef struct ct_cron {
    ct_heap_node_t     node;
    ct_cron_callback_t cb;
    void*              arg;
    ct_time_t          next_time;
    int32_t            minute : 7;
    int32_t            hour : 6;
    int32_t            day : 6;
    int32_t            week : 4;
    int32_t            month : 5;
    uint32_t           is_active : 1;
    uint32_t           is_queued : 1;
    uint32_t           reserved : 2;
} ct_cron_t;

/**
 * @brief Initialize a cron job.
 * @param cron Pointer to the cron job.
 */
CT_API void ct_cron_init(ct_cron_t* cron);

/**
 * @brief Schedule and start a cron job.
 *        If the job is already active, it is rescheduled with the new expression.
 * @param cron     Pointer to the cron job.
 * @param minute   Minute of the hour  (0-59, or -1 for every minute).
 * @param hour     Hour of the day     (0-23, or -1 for every hour).
 * @param day      Day of the month    (1-31, or -1 for every day).
 * @param week     Day of the week     (0-6,  0 = Sunday, or -1 for every day).
 * @param month    Month of the year   (1-12, or -1 for every month).
 * @param callback Callback invoked on each fire.
 * @param arg      User argument passed to the callback.
 * @return 0 on success, -1 on failure.
 */
CT_API int ct_cron_start(ct_cron_t* cron, int minute, int hour, int day, int week, int month,
                         ct_cron_callback_t callback, void* arg);

/**
 * @brief Reschedule an active cron job with a new expression, keeping the existing callback.
 * @param cron   Pointer to the cron job.
 * @param minute Minute of the hour  (0-59, or -1 for every minute).
 * @param hour   Hour of the day     (0-23, or -1 for every hour).
 * @param day    Day of the month    (1-31, or -1 for every day).
 * @param week   Day of the week     (0-6,  0 = Sunday, or -1 for every day).
 * @param month  Month of the year   (1-12, or -1 for every month).
 * @return 0 on success, -1 if the job has no callback set.
 */
CT_API int ct_cron_reset(ct_cron_t* cron, int minute, int hour, int day, int week, int month);

/**
 * @brief Cancel a scheduled cron job.
 * @param cron Pointer to the cron job.
 * @return 0 on success, -1 if the job is not active.
 */
CT_API int ct_cron_stop(ct_cron_t* cron);

#ifdef __cplusplus
}
#endif
#endif  // COTER_TIME_CRON_H
