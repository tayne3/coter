/**
 * @file ct_time.h
 * @brief 日期时间类型实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_TIME_H
#define _CT_TIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <time.h>

#include "base/ct_types.h"

// 时间戳 (64位秒级)
// typedef uint64_t ct_timestamp_sec_t;
// 时间戳 (64位毫秒级)
// typedef uint64_t ct_timestamp_msec_t;

/**
 * @brief 日期时间
 * @note
 * year:    since the year 1900
 * month:   [0-11]
 * day:     [1-31]
 * hour:    [0-23]
 * min:     [0-59]
 * sec:     [0-60] (1 leap second)
 */
typedef struct tm ct_datetime_t, ct_datetime_buf_t[1];

/**
 * @brief 时间戳 (32位秒级)
 */
typedef time_t ct_timestamp_t, ct_timestamp_buf_t[1];

/**
 * @brief 时间规格
 */
typedef struct timespec ct_timespec_t, ct_timespec_buf_t[1];

/**
 * @brief 时间值
 */
typedef struct timeval ct_timeval_t, ct_timeval_buf_t[1];

// clang-format off
#define CT_TIMESTAMP_INITIALIZATION 0                           				 // 时间戳-初始化
#define CT_TIMESTAMP_NULL           (ct_timestamp_t) CT_TIMESTAMP_INITIALIZATION // 时间戳-空值
#define CT_DATETIME_INITIALIZATION  {0, 0, 0, 1, 0, 1900}                        // 日期时间-初始化
#define CT_DATETIME_NULL            (ct_datetime_t) CT_DATETIME_INITIALIZATION   // 日期时间-空值
#define CT_TIMESPEC_INITIALIZATION  {0,0}                                        // 时间规格-初始化
#define CT_TIMESPEC_NULL            (ct_timespec_t) CT_TIMESPEC_INITIALIZATION   // 时间规格-空值
#define CT_TIMEVAL_INITIALIZATION   {0}                                          // 时间值-初始化
#define CT_TIMEVAL_NULL             (ct_timeval_t) CT_TIMEVAL_INITIALIZATION     // 时间值-空值
// clang-format on

/// 获取当前时间（时间戳）
/// 从纪元时间(1970年1月1日00:00:00 UTC)到当前时间的总秒数
ct_timestamp_t ct_current_timestamp(void) __ct_func_throw;
/// 获取当前时间（时间戳）
void ct_current_timestamp_r(ct_timestamp_buf_t ctm) __ct_func_throw;

/// 时间戳比较
enum ct_compare_result ct_timestamp_compare(ct_timestamp_t a, ct_timestamp_t b) __ct_func_throw;

/// 获取当前时间 (时间规格)
ct_timespec_t ct_current_timespec(void) __ct_func_throw;
/// 获取当前时间 (时间规格)
void ct_current_timespec_r(ct_timespec_buf_t cts) __ct_func_throw;

/// 获取当前时间 (绝对时间)
ct_timespec_t ct_current_realtime(void) __ct_func_throw;
/// 获取当前时间 (绝对时间)
void ct_current_realtime_r(ct_timespec_buf_t cts) __ct_func_throw;

/// 时间规格初始化
ct_timespec_t ct_timespec_init(ct_timestamp_t sec, ct_timestamp_t nsec) __ct_func_throw;
/// 时间规格初始化
void ct_timespec_init_r(ct_timespec_buf_t cts, ct_timestamp_t sec, ct_timestamp_t nsec) __ct_func_throw;
/// 精准时间-比较
enum ct_compare_result ct_timespec_compare(const ct_timespec_buf_t a, const ct_timespec_buf_t b) __ct_func_throw;
/// 日期时间-求和
ct_timespec_t ct_timespec_calculate_sum(const ct_timespec_buf_t a, const ct_timespec_buf_t b) __ct_func_throw;
/// 日期时间-求差
ct_timespec_t ct_timespec_calculate_diff(const ct_timespec_buf_t a, const ct_timespec_buf_t b) __ct_func_throw;
/// 精准时间-是否为零
bool ct_timespec_isnull(const ct_timespec_buf_t cts) __ct_func_throw;

/// 获取当前时间 (时间值)
ct_timeval_t ct_current_timeval(void) __ct_func_throw;
/// 获取当前时间 (时间值)
void ct_current_timeval_r(ct_timeval_buf_t ctv) __ct_func_throw;

/// 时间值初始化
ct_timeval_t ct_timeval_init(ct_timestamp_t sec, ct_timestamp_t usec) __ct_func_throw;
/// 时间值初始化
void ct_timeval_init_r(ct_timeval_buf_t ctv, ct_timestamp_t sec, ct_timestamp_t usec) __ct_func_throw;
/// 时间值-比较
enum ct_compare_result ct_timeval_compare(const ct_timeval_buf_t a, const ct_timeval_buf_t b) __ct_func_throw;
/// 时间值-求和
ct_timeval_t ct_timeval_calculate_sum(const ct_timeval_buf_t a, const ct_timeval_buf_t b) __ct_func_throw;
/// 时间值-求差
ct_timeval_t ct_timeval_calculate_diff(const ct_timeval_buf_t a, const ct_timeval_buf_t b) __ct_func_throw;

/// 获取当前时间 (日期时间)
ct_datetime_t ct_current_datetime(void) __ct_func_throw;
/// 获取当前时间 (日期时间)
void ct_current_datetime_r(ct_datetime_buf_t cdt) __ct_func_throw;
/// 获取当前时间字符串
size_t ct_current_datetime_string(const char *format, char *buf, size_t size) __ct_func_throw;

/// 日期时间-初始化
ct_datetime_t ct_datetime_init(int year, int month, int day, int hour, int min, int sec) __ct_func_throw;
/// 日期时间-初始化
void ct_datetime_init_r(ct_datetime_buf_t cdt, int year, int month, int day, int hour, int min,
						int sec) __ct_func_throw;
/// 日期时间-获取信息
// void ct_datetime_get(ct_datetime_buf_t cdt, int *year, int *month, int *day, int *hour, int *min, int *sec)
// __ct_func_throw;

/// 日期时间-比较
enum ct_compare_result ct_datetime_compare(const ct_datetime_buf_t a, const ct_datetime_buf_t b) __ct_func_throw;
/// 日期时间-求和
ct_datetime_t ct_datetime_calculate_sum(const ct_datetime_buf_t a, const ct_datetime_buf_t b) __ct_func_throw;
/// 日期时间-求差
ct_datetime_t ct_datetime_calculate_diff(const ct_datetime_buf_t a, const ct_datetime_buf_t b) __ct_func_throw;
/// 日期时间-是否为零
bool ct_datetime_isnull(const ct_datetime_buf_t cdt) __ct_func_throw;

/// 日期时间转字符串
size_t ct_datetime_to_string(char *buf, size_t max, const char *format, const ct_datetime_buf_t cdt) __ct_func_throw;
/// 字符串转日期时间
/// char *ct_datetime_from_string(const char *buf, const char *format, ct_datetime_buf_t cdt) __ct_func_throw;

/// 将 timespec 转换为 timestamp
void ct_timestamp_from_timespec(ct_timestamp_buf_t ctm, const ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timeval 转换为 timestamp
void ct_timestamp_from_timeval(ct_timestamp_buf_t ctm, const ct_timeval_buf_t ctv) __ct_func_throw;
/// 将 datetime 转换为 timestamp
void ct_timestamp_from_datetime(ct_timestamp_buf_t ctm, const ct_datetime_buf_t cdt) __ct_func_throw;
/// 将 timestamp 转换为 timespec
ct_timespec_t ct_timestamp_to_timespec(const ct_timestamp_t ctm) __ct_func_throw;
/// 将 timestamp 转换为 timeval
ct_timeval_t ct_timestamp_to_timeval(const ct_timestamp_t ctm) __ct_func_throw;
/// 将 timestamp 转换为 datetime
ct_datetime_t ct_timestamp_to_datetime(const ct_timestamp_t ctm) __ct_func_throw;
/// 将 timestamp 转换为 timespec
void ct_timestamp_to_timespec_r(const ct_timestamp_t ctm, ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timestamp 转换为 timeval
void ct_timestamp_to_timeval_r(const ct_timestamp_t ctm, ct_timeval_buf_t cts) __ct_func_throw;
/// 将 timestamp 转换为 datetime
void ct_timestamp_to_datetime_r(const ct_timestamp_t ctm, ct_datetime_buf_t cdt) __ct_func_throw;

/// 将 timestamp 转换为 timespec
void ct_timespec_from_timestamp(ct_timespec_buf_t cts, ct_timestamp_t ctm) __ct_func_throw;
/// 将 timeval 转换为 timespec
void ct_timespec_from_timeval(ct_timespec_buf_t cts, const ct_timeval_buf_t ctv) __ct_func_throw;
/// 将 datetime 转换为 timespec
void ct_timespec_from_datetime(ct_timespec_buf_t cts, const ct_datetime_buf_t cdt) __ct_func_throw;
/// 将 timespec 转换为 timestamp
ct_timestamp_t ct_timespec_to_timestamp(const ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timespec 转换为 timeval
ct_timeval_t ct_timespec_to_timeval(const ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timespec 转换为 datetime
ct_datetime_t ct_timespec_to_datetime(const ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timespec 转换为 timestamp
void ct_timespec_to_timestamp_r(const ct_timespec_buf_t cts, ct_timestamp_buf_t ctm) __ct_func_throw;
/// 将 timespec 转换为 timeval
void ct_timespec_to_timeval_r(const ct_timespec_buf_t cts, ct_timeval_buf_t ctv) __ct_func_throw;
/// 将 timespec 转换为 datetime
void ct_timespec_to_datetime_r(const ct_timespec_buf_t cts, ct_datetime_buf_t cdt) __ct_func_throw;

/// 将 timestamp 转换为 timeval
void ct_timeval_from_timestamp(ct_timeval_buf_t ctv, ct_timestamp_t ctm) __ct_func_throw;
/// 将 timespec 转换为 timeval
void ct_timeval_from_timespec(ct_timeval_buf_t ctv, const ct_timespec_buf_t cts) __ct_func_throw;
/// 将 datetime 转换为 timeval
void ct_timeval_from_datetime(ct_timeval_buf_t ctv, const ct_datetime_buf_t cdt) __ct_func_throw;
/// 将 timeval 转换为 timestamp
ct_timestamp_t ct_timeval_to_timestamp(const ct_timeval_buf_t ctv) __ct_func_throw;
/// 将 timeval 转换为 timespec
ct_timespec_t ct_timeval_to_timeval(const ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timeval 转换为 datetime
ct_datetime_t ct_timeval_to_datetime(const ct_timeval_buf_t ctv) __ct_func_throw;
/// 将 timeval 转换为 timestamp
void ct_timeval_to_timestamp_r(const ct_timeval_buf_t ctv, ct_timestamp_buf_t ctm) __ct_func_throw;
/// 将 timeval 转换为 timespec
void ct_timeval_to_timespec_r(const ct_timeval_buf_t ctv, ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timeval 转换为 datetime
void ct_timeval_to_datetime_r(const ct_timeval_buf_t ctv, ct_datetime_buf_t cdt) __ct_func_throw;

/// 将 timestamp 转换为 datetime
void ct_datetime_from_timestamp(ct_datetime_buf_t cdt, ct_timestamp_t ctm) __ct_func_throw;
/// 将 timespec 转换为 datetime
void ct_datetime_from_timespec(ct_datetime_buf_t cdt, const ct_timespec_buf_t cts) __ct_func_throw;
/// 将 timeval 转换为 datetime
void ct_datetime_from_timeval(ct_datetime_buf_t cdt, const ct_timeval_buf_t ctv) __ct_func_throw;
/// 将 datetime 转换为 timestamp
ct_timestamp_t ct_datetime_to_timestamp(const ct_datetime_buf_t cdt) __ct_func_throw;
/// 将 datetime 转换为 timespec
ct_timespec_t ct_datetime_to_timespec(const ct_datetime_buf_t cdt) __ct_func_throw;
/// 将 datetime 转换为 timeval
ct_timeval_t ct_datetime_to_timeval(const ct_datetime_buf_t cdt) __ct_func_throw;
/// 将 datetime 转换为 timestamp
void ct_datetime_to_timestamp_r(const ct_datetime_buf_t cdt, ct_timestamp_buf_t ctm) __ct_func_throw;
/// 将 datetime 转换为 timespec
void ct_datetime_to_timespec_r(const ct_datetime_buf_t cdt, ct_timespec_buf_t cts) __ct_func_throw;
/// 将 datetime 转换为 timeval
void ct_datetime_to_timeval_r(const ct_datetime_buf_t cdt, ct_timeval_buf_t ctv) __ct_func_throw;

#ifdef __cplusplus
}
#endif
#endif  // _CT_TIME_H
