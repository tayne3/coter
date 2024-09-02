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

extern ct_logger_t* g_logger;

#define cverbose(...) CTLogger_HandleBrief(VarBase, g_logger, __VA_ARGS__)
#define cdebug(...)   CTLogger_HandleBrief(Debug, g_logger, __VA_ARGS__)
#define ctrace(...)   CTLogger_HandleBrief(Trace, g_logger, __VA_ARGS__)
#define cwarning(...) CTLogger_HandleBrief(Warning, g_logger, __VA_ARGS__)
#define cerror(...)   CTLogger_HandleBrief(Error, g_logger, __VA_ARGS__)
#define cfatal(...)   CTLogger_HandleBrief(Fatal, g_logger, __VA_ARGS__)

#define cverbose_n(...) CTLogger_HandleBasic(VarBase, g_logger, __VA_ARGS__)
#define cdebug_n(...)   CTLogger_HandleBasic(Debug, g_logger, __VA_ARGS__)
#define ctrace_n(...)   CTLogger_HandleBasic(Trace, g_logger, __VA_ARGS__)
#define cwarning_n(...) CTLogger_HandleBasic(Warning, g_logger, __VA_ARGS__)
#define cerror_n(...)   CTLogger_HandleBasic(Error, g_logger, __VA_ARGS__)
#define cfatal_n(...)   CTLogger_HandleBasic(Fatal, g_logger, __VA_ARGS__)

void log_init(void);
void log_deinit(void);

#ifdef __cplusplus
}
#endif
#endif  // _LOG_H
