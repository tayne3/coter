#ifndef COTER_CORE_TIME_H
#define COTER_CORE_TIME_H

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

// Get current UNIX epoch timestamp (seconds / milliseconds / microseconds)
CT_API ct_time_t   ct_unixtime_sec(void);
CT_API ct_time64_t ct_unixtime_ms(void);
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

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_TIME_H
