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
typedef int64_t ct_time64_t;

// get milliseconds since system start (if available).
COTER_API unsigned int gettick_ms(void);
// get time in milliseconds.
static inline unsigned long long gettimeofday_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (unsigned long long)1000 + tv.tv_usec/1000;
}
// get time in microseconds.
static inline unsigned long long gettimeofday_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (unsigned long long)1000000 + tv.tv_usec;
}
// get high-resolution time in microseconds.
COTER_API unsigned long long gethrtime_us(void);

// 获取当前秒级时间戳 (自纪元时间)
#define ct_current_second() time(NULL)
// 获取当前毫秒级时间戳 (自纪元时间)
#define ct_current_millisecond() gettimeofday_ms()
// 获取当前微秒级时间戳 (自纪元时间)
#define ct_current_microsecond() gettimeofday_us()

/**
 * @brief 日期时间类型
 * @note
 * tm_year:    since the year 1900
 * tm_mon:     [0-11]
 * tm_mday:    [1-31]
 * tm_hour:    [0-23]
 * tm_min:     [0-59]
 * tm_sec:     [0-60] (1 leap second)
 */
typedef struct tm ct_tm_t, ct_tm_buf_t[1], *ct_tm_ref_t;


/**
 * @brief 获取当前日期时间
 * 
 * @return ct_tm_ref_t 指向当前日期时间的指针
 */
COTER_API ct_tm_ref_t ct_tm_current(void);

// /**
//  * @brief 获取当前日期时间字符串
//  * 
//  * @return ct_tm_ref_t 指向当前日期时间的指针
//  */
// void ct_tm_current_string(const char *format, char *buf, size_t max);

/**
 * @brief 时间戳转换为日期时间
 * 
 * @param t 时间戳
 * @return ct_tm_ref_t 指向转换后的日期时间的指针
 */
COTER_API ct_tm_ref_t ct_tm_from_time(ct_time_t t);

/**
 * @brief 日期时间转换为时间戳
 * 
 * @param dt 日期时间缓冲区
 * @return ct_time_t 转换后的时间戳
 */
COTER_API ct_time_t ct_tm_to_time(ct_tm_buf_t dt);

/**
 * @brief 将日期时间转换为字符串格式
 * 
 * @param buf 存储结果字符串的缓冲区
 * @param max 缓冲区的最大长度
 * @param format 指定输出格式的字符串
 * @param cdt 指向日期时间结构体的指针
 * @return size_t 返回格式化后的字符串长度
 * @note 
 * 格式化字符串的说明:
 * %a - 星期几的简写
 * %A - 星期几的全称
 * %b - 月份的简写
 * %B - 月份的全称
 * %c - 标准的日期和时间串
 * %C - 年份的前两位数字
 * %d - 十进制表示的每月的第几天
 * %D - 月/天/年
 * %e - 在两字符域中, 十进制表示的每月的第几天
 * %F - 年-月-日
 * %g - 年份的后两位数字, 使用基于周的年
 * %G - 年份, 使用基于周的年
 * %h - 简写的月份名
 * %H - 24小时制的小时
 * %I - 12小时制的小时
 * %j - 十进制表示的每年的第几天
 * %m - 十进制表示的月份
 * %M - 十进制表示的分钟数
 * %n - 新行符
 * %p - 本地的AM或PM的等价显示
 * %r - 12小时的时间
 * %R - 显示小时和分钟: hh:mm
 * %S - 十进制的秒数
 * %t - 水平制表符
 * %T - 显示时分秒: hh:mm:ss
 * %u - 每周的第几天, 星期一为第一天 (值从1到7, 星期一为1)
 * %U - 每年的第几周, 把星期日做为第一天 (值从0到53)
 * %w - 十进制表示的星期几 (值从0到6, 星期天为0)
 * %W - 每年的第几周, 把星期一做为第一天 (值从0到53)
 * %x - 标准的日期串
 * %X - 标凈的时间串
 * %y - 不带世纪的十进制年份 (值从0到99)
 * %Y - 带世纪部分的十进制年份
 * %z - 时区名称, 如果不能得到时区名称则返回空字符
 * %Z - 时区名称, 如果不能得到时区名称则返回空字符
 * %% - 百分号
 */
COTER_API size_t ct_tm_to_string(char *buf, size_t max, const char *format, const ct_tm_buf_t cdt);

/**
 * @brief 解析日期时间字符串
 * 
 * @param buf 输入的日期时间字符串
 * @param format 日期时间字符串的格式
 * @param cdt 存储解析结果的结构体
 * @return char* 成功返回解析后的字符串指针，失败返回NULL
 */
COTER_API const char *ct_tm_from_string(const char *buf, const char *format, ct_tm_buf_t cdt);

#ifdef __cplusplus
}
#endif
#endif  // _CT_TIME_H
