/**
 * @file ct_log.c
 * @brief 日志功能
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_log.h"

#include <assert.h>
#include <execinfo.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "base/ct_platform.h"
#include "log/ct_log_control.h"
#include "log/ct_log_msg.h"
#include "log/ct_log_msg_asyn.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_log]"

typedef void (*ct_log_msg_push_t)(ct_log_msg_buf_t msg);
typedef void (*ct_log_msg_flush_t)(void);
typedef void (*ct_log_msg_schedule_t)(void);

static struct ct_log_center_setting {
	int                   level;         // 日志级别 (默认: CTLogLevel_Warning)
	bool                  is_asyn;       // 是否异步输出日志 (默认: false)
	ct_log_msg_push_t     msg_push;      // 日志输出函数
	ct_log_msg_flush_t    msg_flush;     // 日志缓冲区刷新函数
	ct_log_msg_schedule_t msg_schedule;  // 日志调度函数
} setting[1] = {{
	.level        = CTLogLevel_Warning,
	.is_asyn      = false,
	.msg_push     = ct_log_msg_push,
	.msg_flush    = ct_log_msg_flush,
	.msg_schedule = ct_log_msg_schedule,
}};

static inline void program_backtrace(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_log_msg_debug(int type, int level, const char *file, const char *func, int line, const char *format, ...)
{
	assert(CTLOG_LEVEL_ISVALID(level) && CTLOG_TYPE_ISVALID(type));
	if (level < ct_log_center_get_level()) {
		return;
	}

	ct_log_control_t *const self = ct_log_control_ask(type);
	if (!self) {
		return;
	}

	ct_log_msg_buf_t msg = {{
		.control = self,
		.context = {CT_CONTEXT_INIT(file, func, line)},
		.level   = level,
	}};

	// 填充到缓存
	{
		va_list args;
		va_start(args, format);
		msg->msg_size = (size_t)vsnprintf(msg->msg_cache, sizeof(msg->msg_cache), format, args);
		va_end(args);
	}

	setting->msg_push(msg);
}

void ct_log_msg_basic(int type, int level, const char *format, ...)
{
	assert(CTLOG_LEVEL_ISVALID(level) && CTLOG_TYPE_ISVALID(type));
	if (level < ct_log_center_get_level()) {
		return;
	}

	ct_log_control_t *const self = ct_log_control_ask(type);
	if (!self) {
		return;
	}
	ct_log_msg_buf_t msg = {{
		.control = self,
		.context = {CT_CONTEXT_INIT(ct_nullptr, ct_nullptr, 0)},
		.level   = level,
	}};

	// 填充到缓存
	{
		va_list args;
		va_start(args, format);
		msg->msg_size = (size_t)vsnprintf(msg->msg_cache, sizeof(msg->msg_cache), format, args);
		va_end(args);
	}

	setting->msg_push(msg);
}

void ct_log_msg_hex(int type, int level, const uint8_t *array, int length, const char *format, ...)
{
	if (!array) {
		return;
	}

	char   buffer[CTLOG_BUFFER_MAX];
	size_t size = 0;
	// 打印数组
	for (; length-- > 0 && array;) {
		size += ct_snprintf(buffer + size, sizeof(buffer) - size - 1, "%02X ", *array++);
		if (size >= sizeof(buffer) - 1) {
			ct_log_msg_basic(type, level, "%s", buffer);
			size = 0;
		}
	}

	// 打印信息
	va_list list;
	va_start(list, format);
	vsnprintf(buffer + size, sizeof(buffer) - size, format, list);
	va_end(list);

	ct_log_msg_basic(type, level, "%s", buffer);
}

void ct_log_flush(void)
{
	setting->msg_flush();
}

void ct_log_center_schedule(void)
{
	setting->msg_schedule();
}

int ct_log_center_get_level(void)
{
	return setting->level;
}

void ct_log_center_set_level(int level)
{
	setting->level = CTLOG_LEVEL_ISVALID(level) ? level : CTLogLevel_Warning;
}

void ct_log_center_set_asyn(bool is_asyn)
{
	setting->is_asyn = is_asyn;
	if (is_asyn) {
		setting->msg_push     = ct_log_msg_push_asyn;
		setting->msg_flush    = ct_log_msg_flush_asyn;
		setting->msg_schedule = ct_log_msg_schedule_asyn;
		ct_log_msg_flush();
	} else {
		setting->msg_push     = ct_log_msg_push;
		setting->msg_flush    = ct_log_msg_flush;
		setting->msg_schedule = ct_log_msg_schedule;
		ct_log_msg_flush_asyn();
	}
}

void ct_log_center_exception_handler(int _signal)
{
	bool is_backtrace = true;
	// 打印异常信息
	switch (_signal) {
		case SIGSEGV:  // 非法内存访问
			// cfatal_n("Segmentation fault" STR_NEWLINE);
			break;
		case SIGABRT:  // 异常终止
			// cfatal_n("Abnormal termination" STR_NEWLINE);
			break;
		case SIGINT:  // 外部中断
			// cfatal_n("Program interrupted" STR_NEWLINE);
			// is_backtrace = false;
			break;
		case SIGQUIT:  // 终止请求
			// cfatal_n("Program quit" STR_NEWLINE);
			// is_backtrace = false;
			break;
		case SIGFPE:  // 浮点数异常
			// cfatal_n("Floating point exception" STR_NEWLINE);
			break;
		case SIGTERM:  // 终止请求
			// cfatal_n("Program terminated" STR_NEWLINE);
			break;
		case SIGILL:  // 非法指令
			// cfatal_n("Program illegal instruction" STR_NEWLINE);
			break;
		case SIGBUS:  // 非法地址
			// cfatal_n("Bus error" STR_NEWLINE);
			break;
		default: break;
	}

	// 清空日志缓冲区
	setting->msg_flush();
	// 输出堆栈信息
	if (is_backtrace) {
		program_backtrace();
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

#define BACKTRACE_SIZE 100

static inline void program_backtrace(void)
{
	// 缓存区
	void *buffer[BACKTRACE_SIZE];
	// 获取函数调用堆栈信息
	const int count = backtrace(buffer, BACKTRACE_SIZE);

	// 获取堆栈信息对应的符号名称
	char **symbols = backtrace_symbols(buffer, count);
	if (!symbols) {
		perror("backtrace symbols");
		exit(EXIT_FAILURE);
	}

	// 打印堆栈信息
	{
		fprintf(stderr, STR_NEWLINE);
		fprintf(stderr, "[--] ---- backtrace start ---- " STR_NEWLINE);
		for (int i = 0; i < count; i++) {
			fprintf(stderr, "[%02d] %s" STR_NEWLINE, i, symbols[i]);
		}
		fprintf(stderr, "[--] ---- backtrace end ---- " STR_NEWLINE);
	}
	// 释放内存
	free(symbols);
}
