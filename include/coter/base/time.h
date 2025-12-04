/**
 * @file ct_time.h
 * @brief 时间
 */
#ifndef COTER_TIME_H
#define COTER_TIME_H

#include "coter/base/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
struct timezone {
	int tz_minuteswest; /* of Greenwich */
	int tz_dsttime;     /* type of dst correction to apply */
};

#include <sys/timeb.h>
static inline int gettimeofday(struct timeval* tv, struct timezone* tz) {
	struct _timeb tb;
	_ftime(&tb);
	if (tv) {
		tv->tv_sec  = (long)tb.time;
		tv->tv_usec = tb.millitm * 1000;
	}
	if (tz) {
		tz->tz_minuteswest = tb.timezone;
		tz->tz_dsttime     = tb.dstflag;
	}
	return 0;
}
#endif

// 时间戳类型
typedef time_t ct_time_t;
// 64位时间戳类型
typedef int64_t ct_time64_t;

// get milliseconds since system startup. (if available)
ct_time64_t ct_getuptime_ms(void);
// get time in milliseconds.
static inline ct_time64_t ct_gettimeofday_ms(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (ct_time64_t)tv.tv_sec * 1000LL + (ct_time64_t)tv.tv_usec / 1000LL;
}
// get time in microseconds.
static inline ct_time64_t ct_gettimeofday_us(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (ct_time64_t)tv.tv_sec * 1000000LL + (ct_time64_t)tv.tv_usec;
}
// get high-resolution time in microseconds.
ct_time64_t ct_gethrtime_us(void);

// 获取当前秒级时间戳 (自纪元时间)
#define ct_current_second() time(NULL)
// 获取当前毫秒级时间戳 (自纪元时间)
#define ct_current_millisecond() ct_gettimeofday_ms()
// 获取当前微秒级时间戳 (自纪元时间)
#define ct_current_microsecond() ct_gettimeofday_us()

/**
 * @brief 获取当前日期时间
 * @note
 * tm_year:    since the year 1900
 * tm_mon:     [0-11]
 * tm_mday:    [1-31]
 * tm_hour:    [0-23]
 * tm_min:     [0-59]
 * tm_sec:     [0-60] (1 leap second)
 */
static inline void ct_localtime_now(struct tm* tm) {
	const time_t now = time(NULL);
#ifdef CT_OS_WIN
	localtime_s(tm, &now);
#else
	localtime_r(&now, tm);
#endif
}

// 将时间戳转换为本地时间
#ifdef CT_OS_WIN
#define ct_localtime_r(ts, tm) localtime_s(tm, ts)
#define ct_localtime_s(tm, ts) localtime_s(tm, ts)
#else
#define ct_localtime_r(ts, tm) localtime_r(ts, tm)
#define ct_localtime_s(tm, ts) localtime_r(ts, tm)
#endif

// 将日期时间结构体转换为时间戳
#define ct_mktime(tm) mktime(tm)

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
char* ct_tm_duration_fmt(int sec, char* buf);
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
char* ct_tm_fmt(const struct tm* tm, char* buf);
#define CT_TM_FMT_MAX 20

#ifdef __cplusplus
}
#endif
#endif  // COTER_TIME_H
