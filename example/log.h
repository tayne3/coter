/**
 * @file log.h
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#ifndef _LOG_H
#define _LOG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"
#include "mech/ct_log.h"

#define LogType_Default 0

/* 日志输出宏 */

#define logV(...) CTLogger_HandleBrief(Verbose, LogType_Default, __VA_ARGS__)
#define logD(...) CTLogger_HandleBrief(Debug, LogType_Default, __VA_ARGS__)
#define logT(...) CTLogger_HandleBrief(Trace, LogType_Default, __VA_ARGS__)
#define logW(...) CTLogger_HandleBrief(Warning, LogType_Default, __VA_ARGS__)
#define logE(...) CTLogger_HandleBrief(Error, LogType_Default, __VA_ARGS__)
#define logF(...) CTLogger_HandleBrief(Fatal, LogType_Default, __VA_ARGS__)

/* 日志输出宏 (无样式) */

#define logV_n(...) CTLogger_HandleBasic(Verbose, LogType_Default, __VA_ARGS__)
#define logD_n(...) CTLogger_HandleBasic(Debug, LogType_Default, __VA_ARGS__)
#define logT_n(...) CTLogger_HandleBasic(Trace, LogType_Default, __VA_ARGS__)
#define logW_n(...) CTLogger_HandleBasic(Warning, LogType_Default, __VA_ARGS__)
#define logE_n(...) CTLogger_HandleBasic(Error, LogType_Default, __VA_ARGS__)
#define logF_n(...) CTLogger_HandleBasic(Fatal, LogType_Default, __VA_ARGS__)

/* 日志输出宏 (16进制) */

#define logV_hex(__buf, __len) CTLogger_HandleHex(Verbose, LogType_Default, __buf, __len)
#define logD_hex(__buf, __len) CTLogger_HandleHex(Debug, LogType_Default, __buf, __len)
#define logT_hex(__buf, __len) CTLogger_HandleHex(Trace, LogType_Default, __buf, __len)
#define logW_hex(__buf, __len) CTLogger_HandleHex(Warning, LogType_Default, __buf, __len)
#define logE_hex(__buf, __len) CTLogger_HandleHex(Error, LogType_Default, __buf, __len)
#define logF_hex(__buf, __len) CTLogger_HandleHex(Fatal, LogType_Default, __buf, __len)

/**
 * @brief 初始化日志系统
 */
int glog_init(void);

/**
 * @brief 反初始化日志系统
 */
void glog_deinit(void);

/**
 * @brief 日志调度线程函数
 */
void *glog_run(void *arg);

#ifdef __cplusplus
}
#endif
#endif  // _LOG_H
