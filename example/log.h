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

enum log_type {
	LogType_Default = 0,
};

/**
 *  日志输出宏
 */

#define log_verbose(...) CTLogger_HandleBrief(VerBose, LogType_Default, __VA_ARGS__)
#define log_debug(...)   CTLogger_HandleBrief(Debug, LogType_Default, __VA_ARGS__)
#define log_trace(...)   CTLogger_HandleBrief(Trace, LogType_Default, __VA_ARGS__)
#define log_warning(...) CTLogger_HandleBrief(Warning, LogType_Default, __VA_ARGS__)
#define log_error(...)   CTLogger_HandleBrief(Error, LogType_Default, __VA_ARGS__)
#define log_fatal(...)   CTLogger_HandleBrief(Fatal, LogType_Default, __VA_ARGS__)

/**
 *  日志输出宏 (无样式)
 */

#define log_verbose_n(...) CTLogger_HandleBasic(VerBose, LogType_Default, __VA_ARGS__)
#define log_debug_n(...)   CTLogger_HandleBasic(Debug, LogType_Default, __VA_ARGS__)
#define log_trace_n(...)   CTLogger_HandleBasic(Trace, LogType_Default, __VA_ARGS__)
#define log_warning_n(...) CTLogger_HandleBasic(Warning, LogType_Default, __VA_ARGS__)
#define log_error_n(...)   CTLogger_HandleBasic(Error, LogType_Default, __VA_ARGS__)
#define log_fatal_n(...)   CTLogger_HandleBasic(Fatal, LogType_Default, __VA_ARGS__)

/**
 *  日志输出宏 (16进制)
 */

#define log_verbose_hex(__buf, __len) CTLogger_HandleHex(VerBose, LogType_Default, __buf, __len)
#define log_debug_hex(__buf, __len)   CTLogger_HandleHex(Debug, LogType_Default, __buf, __len)
#define log_trace_hex(__buf, __len)   CTLogger_HandleHex(Trace, LogType_Default, __buf, __len)
#define log_warning_hex(__buf, __len) CTLogger_HandleHex(Warning, LogType_Default, __buf, __len)
#define log_error_hex(__buf, __len)   CTLogger_HandleHex(Error, LogType_Default, __buf, __len)
#define log_fatal_hex(__buf, __len)   CTLogger_HandleHex(Fatal, LogType_Default, __buf, __len)

/**
 * @brief 初始化日志系统
 */
void log_init(void);

/**
 * @brief 反初始化日志系统
 */
void log_deinit(void);

#ifdef __cplusplus
}
#endif
#endif  // _LOG_H
