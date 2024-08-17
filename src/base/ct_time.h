/**
 * @file ct_time.h
 * @brief 时间
 * @author tayne3@dingtalk.com
 * @date 2023.11.25
 */
#ifndef _CT_TIME_H
#define _CT_TIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ct_platform.h"

#ifdef _MSC_VER
struct timezone {
    int tz_minuteswest; /* of Greenwich */
    int tz_dsttime;     /* type of dst correction to apply */
};

#include <sys/timeb.h>
static inline int gettimeofday(struct timeval *tv, struct timezone *tz) {
    struct _timeb tb;
    _ftime(&tb);
    if (tv) {
        tv->tv_sec =  (long)tb.time;
        tv->tv_usec = tb.millitm * 1000;
    }
    if (tz) {
        tz->tz_minuteswest = tb.timezone;
        tz->tz_dsttime = tb.dstflag;
    }
    return 0;
}
#endif

// 时间戳类型
typedef time_t ct_time_t;
// 64位时间戳类型
typedef uint64_t ct_time64_t;

// get milliseconds since system start (if available).
COTER_API ct_time64_t gettick_ms(void);
// get time in milliseconds.
static inline ct_time64_t gettimeofday_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (ct_time64_t)1000 + tv.tv_usec/1000;
}
// get time in microseconds.
static inline ct_time64_t gettimeofday_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (ct_time64_t)1000000 + tv.tv_usec;
}
// get high-resolution time in microseconds.
COTER_API ct_time64_t gethrtime_us(void);

// 获取当前秒级时间戳 (自纪元时间)
#define ct_current_second() time(NULL)
// 获取当前毫秒级时间戳 (自纪元时间)
#define ct_current_millisecond() gettimeofday_ms()
// 获取当前微秒级时间戳 (自纪元时间)
#define ct_current_microsecond() gettimeofday_us()

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
static inline struct tm* ct_localtime_now(void) {
	const time_t now = time(NULL);
	return localtime(&now);
}

// 将时间戳转换为本地时间
#define ct_localtime(t) localtime(t)
// 将日期时间结构体转换为时间戳
#define ct_mktime(dt) mktime(dt)

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
 *
 * @note 这个函数在显示持续时间或时间间隔时非常有用，如视频播放器或计时器。
 */
COTER_API char* ct_tm_duration_fmt(int sec, char* buf);
#define CT_TM_DURATION_MAX 12

/**
 * @brief 格式化日期时间
 *
 * 将日期时间结构体格式化为字符串形式(YYYY-MM-DD HH:MM:SS)。
 *
 * @param dt 指向日期时间结构体的指针
 * @param buf 用于存储格式化结果的字符缓冲区
 * @return char* 指向格式化后字符串的指针（与 buf 相同）
 *
 * @code
 * const struct tm dt = ct_tm_now();
 * char buf[CT_DATETIME_FMT_BUFLEN];
 * ct_tm_fmt(&dt, buf);
 * printf("当前日期时间：%s\n", buf);
 * @endcode
 *
 * @note 这个函数在需要以标准格式显示日期时间时很有用，如日志记录或用户界面显示。
 */
COTER_API char* ct_tm_fmt(const struct tm* dt, char* buf);
#define CT_TM_FMT_MAX 20

#ifdef __cplusplus
}
#endif
#endif  // _CT_TIME_H
