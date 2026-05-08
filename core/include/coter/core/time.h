/**
 * @file time.h
 * @brief time primitives
 */
#ifndef COTER_CORE_TIME_H
#define COTER_CORE_TIME_H

#include <time.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

// Timestamp type
typedef time_t ct_time_t;
// 64-bit timestamp type
typedef int64_t ct_time64_t;

// Convert date/time struct to timestamp
#define ct_mktime(tm) mktime(tm)
// Get current timestamp in seconds
#define ct_time(ts)         time(ts)
#define ct_current_second() time(NULL)
// Get current timestamp in milliseconds
#define ct_current_millisecond() ct_gettimeofday_ms()
// Get current timestamp in microseconds
#define ct_current_microsecond() ct_gettimeofday_us()

// Convert timestamp to local time
#ifdef CT_OS_WIN
#define ct_localtime_r(ts, tm) localtime_s(tm, ts)
#define ct_localtime_s(tm, ts) localtime_s(tm, ts)
#define ct_gmtime_r(ts, tm)    gmtime_s(tm, ts)
#define ct_gmtime_s(tm, ts)    gmtime_s(tm, ts)
#else
#define ct_localtime_r(ts, tm) localtime_r(ts, tm)
#define ct_localtime_s(tm, ts) localtime_r(ts, tm)
#define ct_gmtime_r(ts, tm)    gmtime_r(ts, tm)
#define ct_gmtime_s(tm, ts)    gmtime_r(ts, tm)
#endif

/**
 * @brief Get current date and time
 * @note
 * tm_year:    since the year 1900
 * tm_mon:     [0-11]
 * tm_mday:    [1-31]
 * tm_hour:    [0-23]
 * tm_min:     [0-59]
 * tm_sec:     [0-60] (1 leap second)
 */
CT_API void ct_localtime_now(struct tm* tm);

// Get milliseconds since system startup.
CT_API ct_time64_t ct_getuptime_ms(void);

// Get wall-clock time in milliseconds.
CT_API ct_time64_t ct_gettimeofday_ms(void);
// Get wall-clock time in microseconds.
CT_API ct_time64_t ct_gettimeofday_us(void);
// Get high-resolution monotonic time in microseconds.
CT_API ct_time64_t ct_gethrtime_us(void);

// Sleep for seconds.
CT_API void ct_sleep(uint32_t s);
// Sleep for milliseconds.
CT_API void ct_msleep(uint32_t ms);
// Sleep for microseconds.
CT_API void ct_usleep(uint32_t us);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_TIME_H
