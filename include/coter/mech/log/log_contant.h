/**
 * @file ct_log_contant.h
 * @brief 日志常量
 */
#ifndef COTER_LOG_CONSTANT_H
#define COTER_LOG_CONSTANT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "coter/base/platform.h"
#include "coter/base/time.h"

#define CTLog_StyleBasic  0  // 基本信息
#define CTLog_StyleBrief  1  // 简洁信息
#define CTLog_StyleDetail 2  // 详细信息

#define CTLog_LevelVerbose 0  // 变量信息
#define CTLog_LevelDebug   1  // 调试信息
#define CTLog_LevelTrace   2  // 跟踪信息
#define CTLog_LevelWarning 3  // 警告信息
#define CTLog_LevelError   4  // 错误信息
#define CTLog_LevelFatal   5  // 致命错误

#define CTLog_StringVerbose "VBOSE"
#define CTLog_StringDebug   "DEBUG"
#define CTLog_StringTrace   "TRACE"
#define CTLog_StringWarning "WARNG"
#define CTLog_StringError   "ERROR"
#define CTLog_StringFatal   "FATAL"

// 日志样式字符串-起始
#define CTLog_StyleVerbose "\x1B[32;2m"
#define CTLog_StyleDebug   "\x1B[32;2m"
#define CTLog_StyleTrace   "\x1B[36;2m"
#define CTLog_StyleWarning "\x1B[33;2m"
#define CTLog_StyleError   "\x1B[31;2m"
#define CTLog_StyleFatal   "\x1B[31;2m"

// 日志样式字符串-结束
#define CTLog_StyleEnd "\x1B[0m"

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_CONSTANT_H
