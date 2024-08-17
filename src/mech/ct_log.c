/**
 * @file ct_log.c
 * @brief 日志功能
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_log.h"

#include "base/ct_platform.h"
#include "log/ct_log_control.h"
#include "log/ct_log_msg.h"
#include "log/ct_log_msg_asyn.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_log]"

typedef void (*ct_log_msg_push_t)(ct_log_msg_buf_t msg);
typedef void (*ct_log_msg_flush_t)(void);
typedef void (*ct_log_msg_schedule_t)(void);

// 日志管理器
static struct ct_log_manager {
	int                   level;         // 日志级别 (默认: CTLogLevel_Trace)
	bool                  is_asyn;       // 是否异步输出日志 (默认: false)
	ct_log_msg_push_t     msg_push;      // 日志输出函数
	ct_log_msg_flush_t    msg_flush;     // 日志缓冲区刷新函数
	ct_log_msg_schedule_t msg_schedule;  // 日志调度函数
} mgr[1] = {{
	.level        = CTLogLevel_Trace,
	.is_asyn      = false,
	.msg_push     = ct_log_msg_push,
	.msg_flush    = ct_log_msg_flush,
	.msg_schedule = ct_log_msg_schedule,
}};

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_log_msg_debug(int type, int level, const char *file, const char *func, int line, const char *format, ...)
{
	assert(CTLOG_LEVEL_ISVALID(level) && CTLOG_TYPE_ISVALID(type));
	if (level < ct_log_mgr_get_level()) {
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

	mgr->msg_push(msg);
}

void ct_log_msg_basic(int type, int level, const char *format, ...)
{
	assert(CTLOG_LEVEL_ISVALID(level) && CTLOG_TYPE_ISVALID(type));
	if (level < ct_log_mgr_get_level()) {
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

	assert(mgr->msg_push);
	mgr->msg_push(msg);
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
	assert(mgr->msg_flush);
	mgr->msg_flush();
}

void ct_log_mgr_schedule(void)
{
	assert(mgr->msg_schedule);
	mgr->msg_schedule();
}

int ct_log_mgr_get_level(void)
{
	return mgr->level;
}

void ct_log_mgr_set_level(int level)
{
	mgr->level = CTLOG_LEVEL_ISVALID(level) ? level : CTLogLevel_Warning;
}

void ct_log_mgr_set_asyn(bool is_asyn)
{
	// 不允许重复切换同步/异步
	if (mgr->is_asyn) {
		fprintf(stderr, STR_CURRTITLE " already switch log sync/asyn mode is not allowed." STR_NEWLINE);
		return;
	}

	mgr->is_asyn = is_asyn;
	if (is_asyn) {
		ct_log_msg_init_asyn();

		mgr->msg_push     = ct_log_msg_push_asyn;
		mgr->msg_flush    = ct_log_msg_flush_asyn;
		mgr->msg_schedule = ct_log_msg_schedule_asyn;
	} else {
		mgr->msg_push     = ct_log_msg_push;
		mgr->msg_flush    = ct_log_msg_flush;
		mgr->msg_schedule = ct_log_msg_schedule;
	}
}

// -------------------------[STATIC DEFINITION]-------------------------
