/**
 * @file log.h
 * @brief 日志宏
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

#define logV(...) CTLogger_HandleDetail(Verbose, LogType_Default, __VA_ARGS__)
#define logD(...) CTLogger_HandleDetail(Debug, LogType_Default, __VA_ARGS__)
#define logT(...) CTLogger_HandleDetail(Trace, LogType_Default, __VA_ARGS__)
#define logW(...) CTLogger_HandleDetail(Warning, LogType_Default, __VA_ARGS__)
#define logE(...) CTLogger_HandleDetail(Error, LogType_Default, __VA_ARGS__)
#define logF(...) CTLogger_HandleDetail(Fatal, LogType_Default, __VA_ARGS__)

/* 日志输出宏 (无样式) */

#define logVN(...) CTLogger_HandleBasic(Verbose, LogType_Default, __VA_ARGS__)
#define logDN(...) CTLogger_HandleBasic(Debug, LogType_Default, __VA_ARGS__)
#define logTN(...) CTLogger_HandleBasic(Trace, LogType_Default, __VA_ARGS__)
#define logWN(...) CTLogger_HandleBasic(Warning, LogType_Default, __VA_ARGS__)
#define logEN(...) CTLogger_HandleBasic(Error, LogType_Default, __VA_ARGS__)
#define logFN(...) CTLogger_HandleBasic(Fatal, LogType_Default, __VA_ARGS__)

/* 日志输出宏 (16进制) */

#define logVH(__buf, __len) CTLogger_HandleHex(Verbose, LogType_Default, __buf, __len)
#define logDH(__buf, __len) CTLogger_HandleHex(Debug, LogType_Default, __buf, __len)
#define logTH(__buf, __len) CTLogger_HandleHex(Trace, LogType_Default, __buf, __len)
#define logWH(__buf, __len) CTLogger_HandleHex(Warning, LogType_Default, __buf, __len)
#define logEH(__buf, __len) CTLogger_HandleHex(Error, LogType_Default, __buf, __len)
#define logFH(__buf, __len) CTLogger_HandleHex(Fatal, LogType_Default, __buf, __len)

#ifdef __cplusplus
}
#endif
#endif  // _LOG_H
