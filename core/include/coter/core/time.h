#ifndef COTER_CORE_TIME_H
#define COTER_CORE_TIME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

// Timestamp type
typedef time_t ct_time_t;
// 64-bit timestamp type
typedef int64_t ct_time64_t;

// Get current UNIX epoch timestamp (seconds)
CT_API ct_time_t ct_unixtime_sec(void);
// Get current UNIX epoch timestamp (milliseconds)
CT_API ct_time64_t ct_unixtime_ms(void);
// Get current UNIX epoch timestamp (microseconds)
CT_API ct_time64_t ct_unixtime_us(void);

/**
 * @brief Get system monotonic clock in milliseconds
 * @note Unaffected by system clock adjustments. Ideal for timeouts and elapsed time
 *
 * @code
 * const ct_time64_t start = ct_gettick_ms();
 * // ... perform operations ...
 * const ct_time64_t elapsed = ct_gettick_ms() - start;
 * @endcode
 */
CT_API ct_time64_t ct_gettick_ms(void);

/**
 * @brief Get high-resolution monotonic clock in microseconds
 * @note Suitable for micro-benchmarks and performance profiling
 */
CT_API ct_time64_t ct_gethrtime_us(void);

// Thread sleep (seconds / milliseconds / microseconds)
CT_API void ct_sleep(uint32_t s);
CT_API void ct_msleep(uint32_t ms);
CT_API void ct_usleep(uint32_t us);

// Thread-safe calendar time conversion (equivalent to localtime_r / gmtime_r)
CT_API struct tm* ct_localtime_r(const time_t* ts, struct tm* tm);
CT_API struct tm* ct_gmtime_r(const time_t* ts, struct tm* tm);

/**
 * @brief Date and time structure
 */
typedef struct ct_datetime {
    int year;   // Year (1-9999)
    int month;  // Month (1-12)
    int day;    // Day (1-31)
    int hour;   // Hour (0-23)
    int min;    // Minute (0-59)
    int sec;    // Second (0-59)
    int ms;     // Millisecond (0-999)
} ct_datetime_t;

/**
 * @brief Get current local date and time
 *
 * @code
 * const ct_datetime_t now = ct_datetime_now();
 * printf("now: %04d-%02d-%02d %02d:%02d:%02d.%03d\n",
 *        now.year, now.month, now.day, now.hour, now.min, now.sec, now.ms);
 * @endcode
 */
CT_API ct_datetime_t ct_datetime_now(void);

/**
 * @brief Convert UNIX epoch seconds to local date and time
 *
 * @param seconds UNIX timestamp in seconds
 * @return ct_datetime_t Converted local datetime, or all zeros on failure
 *
 * @code
 * const ct_datetime_t dt = ct_datetime_localtime(ct_unixtime_sec());
 * @endcode
 */
CT_API ct_datetime_t ct_datetime_localtime(ct_time_t seconds);

/**
 * @brief Convert date and time structure to UNIX timestamp
 *
 * @param dt Pointer to datetime structure
 * @return ct_time_t UNIX timestamp in seconds, or (ct_time_t)-1 on failure
 *
 * @code
 * const ct_datetime_t dt = {2026, 1, 1, 12, 0, 0, 0};
 * ct_time_t timestamp = ct_datetime_mktime(&dt);
 * @endcode
 */
CT_API ct_time_t ct_datetime_mktime(const ct_datetime_t* dt);

/**
 * @brief Adjust date by a given number of days (forward or backward)
 *
 * @param dt Pointer to datetime structure
 * @param days Days to adjust (positive for future, negative for past)
 * @return bool true on success, false if invalid or year out of range [1, 9999]
 *
 * @code
 * ct_datetime_t dt = ct_datetime_now();
 * ct_datetime_add_days(&dt, 7);   // 7 days later
 * ct_datetime_add_days(&dt, -30); // 30 days ago
 * @endcode
 */
CT_API bool ct_datetime_add_days(ct_datetime_t* dt, int days);

/**
 * @brief Get number of days in a given month and year (leap-year aware)
 *
 * @code
 * int days = ct_datetime_days_in_month(2024, 2); // 29
 * @endcode
 */
CT_API int ct_datetime_days_in_month(int year, int month);

/**
 * @brief Format datetime as "YYYY-MM-DD HH:MM:SS"
 *
 * @code
 * char buf[CT_DATETIME_FMT_BUFLEN];
 * const ct_datetime_t now = ct_datetime_now();
 * ct_datetime_fmt(&now, buf, sizeof(buf));
 * @endcode
 */
CT_API char* ct_datetime_fmt(const ct_datetime_t* dt, char* buf, size_t size);
#define CT_DATETIME_FMT_BUFLEN 20  // "YYYY-MM-DD HH:MM:SS"

/**
 * @brief Format datetime as ISO 8601 string "YYYY-MM-DDTHH:MM:SS.sss"
 *
 * @code
 * char buf[CT_DATETIME_FMT_ISO_BUFLEN];
 * const ct_datetime_t now = ct_datetime_now();
 * ct_datetime_fmt_iso(&now, buf, sizeof(buf));
 * @endcode
 */
CT_API char* ct_datetime_fmt_iso(const ct_datetime_t* dt, char* buf, size_t size);
#define CT_DATETIME_FMT_ISO_BUFLEN 24  // "YYYY-MM-DDTHH:MM:SS.sss"

/**
 * @brief Format duration in seconds as "HH:MM:SS"
 *
 * @code
 * char buf[CT_DURATION_FMT_BUFLEN];
 * ct_duration_fmt(3661, buf, sizeof(buf)); // "01:01:01"
 * @endcode
 */
CT_API char* ct_duration_fmt(int sec, char* buf, size_t size);
#define CT_DURATION_FMT_BUFLEN 13  // "HH:MM:SS"

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_TIME_H
