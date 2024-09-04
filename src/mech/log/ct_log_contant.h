/**
 * @file ct_log_contant.h
 * @brief 日志常量
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_CONSTANT_H
#define _CT_LOG_CONSTANT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"
#include "base/ct_time.h"

#define CTLog_StyleBasic  0  // 基本信息
#define CTLog_StyleBrief  1  // 简洁信息
#define CTLog_StyleDetail 2  // 详细信息

#ifndef CTLog_StyleUser
#define CTLog_StyleUser CTLog_StyleBrief
#endif

#define CTLog_LevelVerBose 0  // 变量信息
#define CTLog_LevelDebug   1  // 调试信息
#define CTLog_LevelTrace   2  // 跟踪信息
#define CTLog_LevelWarning 3  // 警告信息
#define CTLog_LevelError   4  // 错误信息
#define CTLog_LevelFatal   5  // 致命错误

#define CTLog_StringVerBose "VBASE"
#define CTLog_StringDebug   "DEBUG"
#define CTLog_StringTrace   "TRACE"
#define CTLog_StringWarning "WARNG"
#define CTLog_StringError   "ERROR"
#define CTLog_StringFatal   "FATAL"

// 日志样式字符串-起始
#define CTLog_StyleVerBose "\x1B[32;2;4m"
#define CTLog_StyleDebug   "\x1B[32;2;4m"
#define CTLog_StyleTrace   "\x1B[36;2;4m"
#define CTLog_StyleWarning "\x1B[33;2;4m"
#define CTLog_StyleError   "\x1B[31;2;4m"
#define CTLog_StyleFatal   "\x1B[31;2;4m"

// 日志样式字符串-结束
#define CTLog_StyleEnd "\x1B[0m"

#if 0
// XXX(level, level_str, style_start, style_end)
#define CTLog_LevelMap(XXX)                                                          \
	XXX(CTLog_LevelVerBose, CTLog_StringVerBose, CTLog_StyleVerBose, CTLog_StyleEnd) \
	XXX(CTLog_LevelDebug, CTLog_StringDebug, CTLog_StyleDebug, CTLog_StyleEnd)       \
	XXX(CTLog_LevelTrace, CTLog_StringTrace, CTLog_StyleTrace, CTLog_StyleEnd)       \
	XXX(CTLog_LevelWarning, CTLog_StringWarning, CTLog_StyleWarning, CTLog_StyleEnd) \
	XXX(CTLog_LevelError, CTLog_StringError, CTLog_StyleError, CTLog_StyleEnd)       \
	XXX(CTLog_LevelFatal, CTLog_StringFatal, CTLog_StyleFatal, CTLog_StyleEnd)
#endif

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_CONSTANT_H
