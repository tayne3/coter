/**
 * @file ct_datetime.h
 * @brief 日期时间
 * @author tayne3@dingtalk.com
 * @date 2023.11.25
 */
#ifndef _CT_DATETIME_H
#define _CT_DATETIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ct_time.h"

/**
 * @brief 日期时间结构体
 *
 * 用于表示完整的日期时间信息，包括年、月、日、时、分、秒和毫秒。
 */
typedef struct {
	int year;   // 年份
	int month;  // 月份 (1-12)
	int day;    // 日期 (1-31)
	int wday;   // 星期几 (0-6, 0 表示星期日)
	int hour;   // 小时 (0-23)
	int min;    // 分钟 (0-59)
	int sec;    // 秒 (0-59)
	int ms;     // 毫秒 (0-999)
} ct_datetime_t, ct_datetime_buf_t[1];

/**
 * @brief 获取当前日期时间
 *
 * @return ct_datetime_t 当前日期时间
 *
 * @code
 * const ct_datetime_t now = ct_datetime_now();
 * printf("current datetime: %04d-%02d-%02d %02d:%02d:%02d.%03d\n",
 *        now.year, now.month, now.day, now.hour, now.min, now.sec, now.ms);
 * @endcode
 *
 * @note
 * 获取系统当前的日期时间，精确到毫秒。
 */
COTER_API ct_datetime_t ct_datetime_now(void);

/**
 * @brief 将时间戳转换为本地日期时间
 *
 * @param seconds UNIX 时间戳 (从1970年1月1日UTC开始的秒数)
 * @return ct_datetime_t 转换后的本地日期时间
 *
 * @code
 * const time_t timestamp = time(NULL);
 * const ct_datetime_t local_time = ct_datetime_localtime(timestamp);
 * printf("local datetime: %04d-%02d-%02d %02d:%02d:%02d\n",
 *        local_time.year, local_time.month, local_time.day,
 *        local_time.hour, local_time.min, local_time.sec);
 * @endcode
 *
 * @note
 * 将给定的 UNIX 时间戳转换为本地时区的日期时间结构体。
 */
COTER_API ct_datetime_t ct_datetime_localtime(time_t seconds);

/**
 * @brief 将日期时间结构体转换为时间戳
 *
 * @param dt 指向日期时间结构体的指针
 * @return time_t 转换后的 UNIX 时间戳
 *
 * @code
 * const ct_datetime_t dt = {2023, 12, 31, 23, 59, 59, 0};
 * const time_t timestamp = ct_datetime_mktime(&dt);
 * printf("timestamp: %ld\n", timestamp);
 * @endcode
 *
 * @note
 * 将给定的日期时间结构体转换为 UNIX 时间戳。
 */
COTER_API time_t ct_datetime_mktime(const ct_datetime_t* dt);

/**
 * @brief 计算过去的日期
 *
 * @param dt 指向日期时间结构体的指针
 * @param days 要向前推算的天数
 * @return ct_datetime_t* 指向修改后的日期时间结构体的指针
 *
 * @code
 * ct_datetime_t dt = ct_datetime_now();
 * ct_datetime_past(&dt, 7);
 * printf("一周前：%04d-%02d-%02d\n", dt.year, dt.month, dt.day);
 * @endcode
 *
 * @note
 * 从给定的日期时间向前推算指定的天数。
 */
COTER_API ct_datetime_t* ct_datetime_past(ct_datetime_t* dt, int days);

/**
 * @brief 计算未来的日期
 *
 * @param dt 指向日期时间结构体的指针
 * @param days 要向后推算的天数
 * @return ct_datetime_t* 指向修改后的日期时间结构体的指针
 *
 * @code
 * ct_datetime_t dt = ct_datetime_now();
 * ct_datetime_future(&dt, 30);
 * printf("30天后：%04d-%02d-%02d\n", dt.year, dt.month, dt.day);
 * @endcode
 *
 * @note
 * 从给定的日期时间向后推算指定的天数。
 */
COTER_API ct_datetime_t* ct_datetime_future(ct_datetime_t* dt, int days);

#define CT_TIME_FMT        "%02d:%02d:%02d"
#define CT_TIME_FMT_BUFLEN 12
/**
 * @brief 格式化持续时间
 *
 * 将给定的秒数格式化为小时:分钟:秒的字符串形式。
 *
 * @param sec 持续时间（秒）
 * @param buf 用于存储格式化结果的字符缓冲区
 * @return char* 指向格式化后字符串的指针（与 buf 相同）
 *
 * @code
 * char buf[CT_TIME_FMT_BUFLEN];
 * int duration_seconds = 3661; // 1小时1分钟1秒
 * ct_datetime_duration_fmt(duration_seconds, buf);
 * printf("duration: %s\n", buf); // 输出：duration: 01:01:01
 * @endcode
 *
 * @note 这个函数在显示持续时间或时间间隔时非常有用，如视频播放器或计时器。
 */
char* ct_datetime_duration_fmt(int sec, char* buf);

#define CT_DATETIME_FMT        "%04d-%02d-%02d %02d:%02d:%02d"
#define CT_DATETIME_FMT_ISO    "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ"
#define CT_DATETIME_FMT_BUFLEN 30
/**
 * @brief 格式化日期时间
 *
 * 将日期时间结构体格式化为字符串形式（YYYY-MM-DD HH:MM:SS）。
 *
 * @param dt 指向日期时间结构体的指针
 * @param buf 用于存储格式化结果的字符缓冲区
 * @return char* 指向格式化后字符串的指针（与 buf 相同）
 *
 * @code
 * const ct_datetime_t dt = ct_datetime_now();
 * char buf[CT_DATETIME_FMT_BUFLEN];
 * ct_datetime_fmt(&dt, buf);
 * printf("当前日期时间：%s\n", buf);
 * @endcode
 *
 * @note 这个函数在需要以标准格式显示日期时间时很有用，如日志记录或用户界面显示。
 */
char* ct_datetime_fmt(const ct_datetime_t* dt, char* buf);

/**
 * @brief 格式化ISO日期时间
 *
 * 将日期时间结构体格式化为ISO 8601格式（YYYY-MM-DDTHH:MM:SS.SSSZ）。
 *
 * @param dt 指向日期时间结构体的指针
 * @param buf 用于存储格式化结果的字符缓冲区
 * @return char* 指向格式化后字符串的指针（与 buf 相同）
 *
 * @code
 * ct_datetime_t dt = ct_datetime_now();
 * char buf[CT_DATETIME_FMT_BUFLEN];
 * ct_datetime_fmt_iso(&dt, buf);
 * printf("ISO 格式日期时间：%s\n", buf);
 * @endcode
 *
 * @note 这个函数在需要以标准格式显示日期时间时很有用，如日志记录或用户界面显示。
 */
char* ct_datetime_fmt_iso(const ct_datetime_t* dt, char* buf);

#define CT_GMTIME_FMT        "%.3s, %02d %.3s %04d %02d:%02d:%02d GMT"
#define CT_GMTIME_FMT_BUFLEN 30
/**
 * @brief 格式化 GMT 时间
 *
 * 将给定的时间戳格式化为 GMT 时间字符串。
 *
 * @param time UNIX 时间戳
 * @param buf 用于存储格式化结果的字符缓冲区
 * @return char* 指向格式化后字符串的指针（与 buf 相同）
 *
 * @code
 * const time_t now = time(NULL);
 * char buf[CT_GMTIME_FMT_BUFLEN];
 * ct_datetime_gmtime_fmt(now, buf);
 * printf("GMT time: %s\n", buf);
 * @endcode
 *
 * @note 这个函数在需要显示 GMT 时间时很有用，如生成 HTTP 头或处理国际化时间。
 */
char* ct_datetime_gmtime_fmt(time_t time, char* buf);

/**
 * @brief 计算指定月份的天数
 *
 * 根据给定的年份和月份计算该月的天数，考虑闰年。
 *
 * @param month 月份（1-12）
 * @param year 年份
 * @return int 该月的天数
 *
 * @code
 * int days = ct_datetime_days_of_month(2, 2024);
 * printf("2024年2月有 %d 天\n", days); // 输出：2024年2月有 29 天
 * @endcode
 *
 * @note 这个函数在处理日期计算和验证时很有用，特别是在考虑闰年的情况下。
 */
int ct_datetime_days_of_month(int month, int year);

/**
 * @brief 将月份名称转换为数字
 *
 * 将月份的英文名称（全名或缩写）转换为对应的数字（1-12）。
 *
 * @param month 月份的英文名称
 * @return int 月份对应的数字（1-12），如果转换失败返回 0
 *
 * @code
 * int month_num = ct_datetime_month_atoi("Feb");
 * printf("Feb 对应的月份数字：%d\n", month_num); // 输出：Feb 对应的月份数字：2
 * @endcode
 *
 * @note 这个函数在解析包含月份名称的日期字符串时很有用。
 */
int ct_datetime_month_atoi(const char* month);

/**
 * @brief 将月份数字转换为名称
 *
 * 将月份的数字（1-12）转换为对应的英文名称。
 *
 * @param month 月份数字（1-12）
 * @return const char* 月份的英文名称
 *
 * @code
 * const char* month_name = ct_datetime_month_itoa(2);
 * printf("2 对应的月份名称：%s\n", month_name); // 输出：2 对应的月份名称：February
 * @endcode
 *
 * @note 这个函数在格式化日期显示时很有用，特别是在需要显示月份名称而不是数字时。
 */
const char* ct_datetime_month_itoa(int month);

/**
 * @brief 将星期名称转换为数字
 *
 * 将星期的英文名称（全名或缩写）转换为对应的数字（0-6，0 表示星期日）。
 *
 * @param weekday 星期的英文名称
 * @return int 星期对应的数字（0-6），如果转换失败返回 -1
 *
 * @code
 * int weekday_num = ct_datetime_weekday_atoi("Mon");
 * printf("Mon 对应的星期数字：%d\n", weekday_num); // 输出：Mon 对应的星期数字：1
 * @endcode
 *
 * @note 这个函数在解析包含星期名称的日期字符串时很有用。
 */
int ct_datetime_weekday_atoi(const char* weekday);

/**
 * @brief 将星期数字转换为名称
 *
 * 将星期的数字（0-6，0 表示星期日）转换为对应的英文名称。
 *
 * @param weekday 星期数字（0-6 或 7，7 也表示星期日）
 * @return const char* 星期的英文名称
 *
 * @code
 * const char* weekday_name = ct_datetime_weekday_itoa(3);
 * printf("3 对应的星期名称：%s\n", weekday_name); // 输出：3 对应的星期名称：Wednesday
 * @endcode
 *
 * @note 这个函数在格式化日期显示时很有用，特别是在需要显示星期名称而不是数字时。
 */
const char* ct_datetime_weekday_itoa(int weekday);

/**
 * @brief 获取库的编译时间
 *
 * 返回一个表示库编译时间的日期时间结构体。
 *
 * @return ct_datetime_t 库的编译时间
 *
 * @code
 * ct_datetime_t time_compile = ct_datetime_compile();
 * char buf[CT_DATETIME_FMT_BUFLEN];
 * ct_datetime_fmt(&time_compile, buf);
 * printf("compile time: %s\n", buf);
 * @endcode
 *
 * @note 这个函数通常用于调试或版本信息显示，可以帮助识别特定的库构建。
 */
ct_datetime_t __ct_datetime_compile(const char* date, const char* time);
#define ct_datetime_compile() __ct_datetime_compile(__DATE__, __TIME__)

/**
 * @brief 计算下一个 cron 任务执行时间
 *
 * 根据给定的 cron 表达式参数，计算下一个符合条件的执行时间。
 *
 * @param minute 分钟（0-59，-1 表示每分钟）
 * @param hour 小时（0-23，-1 表示每小时）
 * @param day 日期（1-31，-1 表示每天）
 * @param week 星期（0-6，-1 表示每周）
 * @param month 月份（1-12，-1 表示每月）
 * @return time_t 下一个执行时间的时间戳，如果参数无效则返回 -1
 *
 * @code
 * // 设置每天凌晨1:30执行
 * time_t next_run = ct_datetime_cron_next_timeout(30, 1, -1, -1, -1);
 * if (next_run != -1) {
 *     char buf[CT_DATETIME_FMT_BUFLEN];
 *     ct_datetime_t dt = ct_datetime_datetime_localtime(next_run);
 *     ct_datetime_datetime_fmt(&dt, buf);
 *     printf("next run time: %s\n", buf);
 * }
 * @endcode
 *
 * @note 这个函数在实现定时任务或调度系统时非常有用，可以灵活地设置各种周期性任务。
 * minute   hour    day     week    month       action
 * 0~59     0~23    1~31    0~6     1~12
 *  -1      -1      -1      -1      -1          cron.minutely
 *  30      -1      -1      -1      -1          cron.hourly
 *  30      1       -1      -1      -1          cron.daily
 *  30      1       15      -1      -1          cron.monthly
 *  30      1       -1       0      -1          cron.weekly
 *  30      1        1      -1      10          cron.yearly
 */
time_t ct_datetime_cron_next_timeout(int minute, int hour, int day, int week, int month);

#ifdef __cplusplus
}
#endif
#endif  // _CT_DATETIME_H
