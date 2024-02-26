/**
 * @file ct_app.c
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#include "ct_app.h"

#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/ct_platform.h"
#include "base/ct_version.h"
#include "mech/ct_evmsg.h"
#include "mech/ct_log.h"
#include "mech/ct_thpool.h"
#include "mech/ct_timer.h"
#include "mech/log/ct_log_msg_asyn.h"
#include "sys/ct_thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_app]"

// 调度间隔
#define CT_APP_SCHEDULE_INTERVAL 10

/**
 * @brief coter 应用实例
 */
static struct ct_app {
	bool            running;   // 是否运行中
	int             abnormal;  // 异常标志
	ct_thpool_ptr_t thpool;    // 全局线程池
} app[1] = {{
	.running  = false,
	.abnormal = 0,
	.thpool   = ct_nullptr,
}};

// 初始化异常处理
static __ct_force_inline void ct_app_exception_init(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_app_ptr_t ct_app_create(void)
{
	ct_app_exception_init();                        // 初始化异常处理机制
	app->thpool = ct_thpool_global_create(16, 50);  // 创建全局线程池，参数分别为线程池大小和工作队列大小
	ct_timer_center_init();                         // 初始化定时器中枢
	ct_evmsg_center_init();                         // 初始化事件消息中枢
	return app;
}

int ct_app_exec(ct_app_ptr_t self)
{
	self->running = true;
	for (; !self->abnormal;) {
		ct_log_center_schedule();                    // 执行日志调度
		ct_timer_center_schedule();                  // 执行定时器调度
		ct_evmsg_center_schedule();                  // 执行事件消息调度
		ct_thread_msleep(CT_APP_SCHEDULE_INTERVAL);  // 调度间隔
	}
	self->running = false;
	cfatal(STR_CURRTITLE " application exit(%d)." STR_NEWLINE, self->abnormal);
	ct_log_flush();                   // 刷新日志缓冲区，确保所有日志都被写入
	ct_thpool_destroy(self->thpool);  // 销毁线程池
	if (self->abnormal < 128) {
		raise(self->abnormal);  // 如果异常标志小于128，则通过raise函数发送信号
	}
	return self->abnormal;
}

void ct_app_exit(int status)
{
	// 日志异常处理
	if (status < 128) {
		ct_log_center_exception_handler(status);
	}
	// 设置异常标志
	app->abnormal = status;
	// 重置所有信号处理函数为默认行为
	signal(SIGSEGV, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	// 检查是否在运行中
	if (!app->running) {
		exit(app->abnormal);
	}
	// 死循环，等待线程退出
	ct_forever {
		ct_thread_msleep(10000);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ct_app_exception_init(void)
{
	signal(SIGSEGV, ct_app_exit);
	signal(SIGABRT, ct_app_exit);
	signal(SIGINT, ct_app_exit);
	signal(SIGQUIT, ct_app_exit);
	signal(SIGFPE, ct_app_exit);
	signal(SIGTERM, ct_app_exit);
	signal(SIGILL, ct_app_exit);
	signal(SIGBUS, ct_app_exit);
}
