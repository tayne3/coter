/**
 * @file ct_cron.h
 * @brief cron任务管理
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_CRON_H
#define _CT_CRON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_any.h"
#include "base/ct_platform.h"
#include "base/ct_time.h"

struct ct_thpool;

// cron任务id类型
typedef uint32_t ct_cron_id_t;
// 无效的cron任务id
#define CT_CRON_ID_INVALID 0
// cron任务回调函数类型
typedef void (*ct_cron_callback_t)(void*);

/**
 * @brief cron任务管理初始化
 * @param now 当前时间 (s)
 * @param thpool 任务池
 */
void ct_cron_mgr_init(ct_time_t now, struct ct_thpool* thpool) __ct_nonnull(2);

/**
 * @brief cron任务调度
 * @param now 当前时间 (s)
 * @return bool 是否有cron任务被触发
 */
bool ct_cron_mgr_schedule(ct_time_t now);

/**
 * @brief 启动cron任务
 * @param minute 分钟 (0-59, -1 表示每分钟)
 * @param hour 小时 (0-23, -1 表示每小时)
 * @param day 日期 (1-31, -1 表示每天)
 * @param week 星期 (0-6, 0 表示周日, -1 表示每周)
 * @param month 月份 (1-12, -1 表示每月)
 * @param callback 任务触发时的回调函数
 * @param arg 传递给回调函数的参数
 * @return ct_cron_id_t 返回cron任务的唯一标识符，如果创建失败则返回 CT_CRON_ID_INVALID
 */
ct_cron_id_t ct_cron_start(int minute, int hour, int day, int week, int month, ct_cron_callback_t callback, void* arg);

/**
 * @brief 停止cron任务
 * @param id cron任务id
 */
void ct_cron_stop(ct_cron_id_t id);

/**
 * @brief 计算下一个 cron 任务执行时间
 *
 * 根据给定的 cron 表达式参数, 计算下一个符合条件的执行时间。
 *
 * @param minute 分钟 (0-59, -1 表示每分钟)
 * @param hour 小时 (0-23, -1 表示每小时)
 * @param day 日期 (1-31, -1 表示每天)
 * @param week 星期 (0-6, 0 表示周日, -1 表示每周)
 * @param month 月份 (1-12, -1 表示每月)
 * @return time_t 下一个执行时间的时间戳, 如果参数无效则返回 -1
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
 * @note 这个函数在实现定时任务或调度系统时非常有用, 可以灵活地设置各种周期性任务。
 * minute   hour    day     week    month       action
 * 0~59     0~23    1~31    0~6     1~12
 *  -1      -1      -1      -1      -1          cron.minutely
 *  30      -1      -1      -1      -1          cron.hourly
 *  30      1       -1      -1      -1          cron.daily
 *  30      1       15      -1      -1          cron.monthly
 *  30      1       -1       0      -1          cron.weekly
 *  30      1        1      -1      10          cron.yearly
 */
ct_time_t ct_cron_next_timeout(ct_time_t now, int minute, int hour, int day, int week, int month);

#ifdef __cplusplus
}
#endif
#endif  // _CT_CRON_H
