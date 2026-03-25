/**
 * @file fmt.h
 * @brief 时间格式化
 */
#ifndef COTER_TIME_FMT_H
#define COTER_TIME_FMT_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 格式化持续时间
 *
 * 将给定的秒数格式化为字符串形式(HH:MM:SS)
 *
 * @param sec 持续时间（秒）
 * @param buf 用于存储格式化结果的字符缓冲区
 * @return char* 指向格式化后字符串的指针（与 buf 相同）
 *
 * @code
 * char buf[CT_TM_FMT_BUFLEN];
 * int duration_seconds = 3661; // 1小时1分钟1秒
 * ct_tm_duration_fmt(duration_seconds, buf);
 * printf("duration: %s\n", buf); // 输出：duration: 01:01:01
 * @endcode
 */
CT_API char* ct_tm_duration_fmt(int sec, char* buf);
#define CT_TM_DURATION_MAX 12

/**
 * @brief 格式化日期时间
 *
 * 将日期时间结构体格式化为字符串形式(YYYY-MM-DD HH:MM:SS)。
 *
 * @param tm 指向日期时间结构体的指针
 * @param buf 用于存储格式化结果的字符缓冲区
 * @return char* 指向格式化后字符串的指针（与 buf 相同）
 *
 * @code
 * const struct tm tm = ct_tm_now();
 * char buf[CT_DATETIME_FMT_BUFLEN];
 * ct_tm_fmt(&tm, buf);
 * printf("当前日期时间：%s\n", buf);
 * @endcode
 */
CT_API char* ct_tm_fmt(const struct tm* tm, char* buf);
#define CT_TM_FMT_MAX 20

#ifdef __cplusplus
}
#endif
#endif  // COTER_TIME_FMT_H
