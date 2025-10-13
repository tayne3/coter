/**
 * @file ct_log_contant.h
 * @brief 日志常量
 */
#ifndef COTER_LOG_CONSTANT_H
#define COTER_LOG_CONSTANT_H

#include "coter/base/platform.h"
#include "coter/base/time.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CTLog_StyleBasic  0  // 基本样式
#define CTLog_StyleBrief  1  // 简洁样式
#define CTLog_StyleDetail 2  // 详细样式

#define CTLog_LevelVerbose 0  // 变量日志
#define CTLog_LevelDebug   1  // 调试日志
#define CTLog_LevelTrace   2  // 跟踪日志
#define CTLog_LevelWarning 3  // 警告日志
#define CTLog_LevelError   4  // 错误日志
#define CTLog_LevelFatal   5  // 致命错误

#define CTLog_StringVerbose "VER"
#define CTLog_StringDebug   "DBG"
#define CTLog_StringTrace   "TRC"
#define CTLog_StringWarning "WRN"
#define CTLog_StringError   "ERR"
#define CTLog_StringFatal   "FTL"

// 日志样式字符串
#define CTLog_StyleVerbose "\x1b[34;22m"
#define CTLog_StyleDebug   "\x1b[37;22m"
#define CTLog_StyleTrace   "\x1b[32;22m"
#define CTLog_StyleWarning "\x1b[33;22m"
#define CTLog_StyleError   "\x1b[31;22m"
#define CTLog_StyleFatal   "\x1b[31;22m"

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_CONSTANT_H
