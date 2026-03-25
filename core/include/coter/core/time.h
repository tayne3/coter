/**
 * @file time.h
 * @brief Cross-platform time wrapper
 */
#ifndef COTER_CORE_TIME_H
#define COTER_CORE_TIME_H

#include <time.h>

#include "coter/core/macro.h"

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !HAVE_STRUCT_TIMEVAL
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif

#if !HAVE_STRUCT_TIMEZONE
struct timezone {
    int tz_minuteswest; /* of Greenwich */
    int tz_dsttime;     /* type of dst correction to apply */
};
#endif

#ifdef _MSC_VER
CT_INLINE int gettimeofday(struct timeval* tv, struct timezone* tz) {
    if (tv) {
        FILETIME ft;
        uint64_t ft64;
        GetSystemTimeAsFileTime(&ft);
        ft64 = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        ft64 -= 116444736000000000ULL;
        tv->tv_sec  = (long)(ft64 / 10000000ULL);
        tv->tv_usec = (long)((ft64 % 10000000ULL) / 10ULL);
    }
    if (tz) {
        tz->tz_minuteswest = 0;
        tz->tz_dsttime     = 0;
    }
    return 0;
}
#endif

// Timestamp type
typedef time_t ct_time_t;
// 64-bit timestamp type
typedef int64_t ct_time64_t;

// get milliseconds since system startup. (if available)
CT_API ct_time64_t ct_getuptime_ms(void);
// get time in milliseconds.
CT_API ct_time64_t ct_gettimeofday_ms(void);
// get time in microseconds.
CT_API ct_time64_t ct_gettimeofday_us(void);
// get high-resolution time in microseconds.
CT_API ct_time64_t ct_gethrtime_us(void);

// sleep for seconds.
CT_API void ct_sleep(uint32_t s);
// sleep for milliseconds.
CT_API void ct_msleep(uint32_t ms);
// sleep for microseconds.
CT_API void ct_usleep(uint32_t us);

// Get current timestamp in seconds (since epoch)
#define ct_current_second() time(NULL)
// Get current timestamp in milliseconds (since epoch)
#define ct_current_millisecond() ct_gettimeofday_ms()
// Get current timestamp in microseconds (since epoch)
#define ct_current_microsecond() ct_gettimeofday_us()

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

// Convert timestamp to local time
#ifdef CT_OS_WIN
#define ct_localtime_r(ts, tm) localtime_s(tm, ts)
#define ct_localtime_s(tm, ts) localtime_s(tm, ts)
#else
#define ct_localtime_r(ts, tm) localtime_r(ts, tm)
#define ct_localtime_s(tm, ts) localtime_r(ts, tm)
#endif

// Convert date/time structure to timestamp
#define ct_mktime(tm) mktime(tm)

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_TIME_H
