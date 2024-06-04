/**
 * @file ct_app.c
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#include "ct_app.h"

#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(CT_OS_LINUX)
#include <execinfo.h>
#elif defined(CT_OS_WIN)
#include <dbghelp.h>
#include <windows.h>
#pragma comment(lib, "dbghelp.lib")
#endif

#include "base/ct_platform.h"
#include "mech/ct_evmsg.h"
#include "mech/ct_log.h"
#include "mech/ct_thpool.h"
#include "mech/ct_timer.h"
#include "mech/log/ct_log_msg_asyn.h"
#include "sys/ct_mutex.h"
#include "sys/ct_thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_app]"

// 异常处理函数类型
typedef void (*ct_app_exception_handler_t)(int);
// 检查是否是系统信号
#define CT_APP_IS_SIGNAL(_signal) (_signal > 0 && _signal < SIGRTMAX)

/**
 * @brief coter 应用实例
 */
static struct ct_app {
	int             abnormal;  // 异常标志
	ct_thpool_ptr_t thpool;    // 全局线程池
	ct_thread_t     tid;       // 主线程ID
	jmp_buf         jmpbuf;    // 上下文信息
} app[1] = {{
	.abnormal = 0,
	.thpool   = ct_nullptr,
	.tid      = 0,
}};

// 初始化异常处理
static __ct_force_inline void ct_app_exception_init(ct_app_exception_handler_t handler);
// 打印堆栈信息
static __ct_force_inline void ct_app_program_backtrace(void);
// 输出堆栈信息
static inline void ct_app_exception_handler(int _signal);

#define ct_app_start()                                                             \
	do {                                                                           \
		char ctm_str[22];                                                          \
		ct_current_datetime_string("%Y.%m.%d-%H:%M:%S", ctm_str, sizeof(ctm_str)); \
		ctrace(STR_CURRTITLE " application start at '%s'." STR_NEWLINE, ctm_str);  \
	} while (0)

#define ct_app_end(code)                                                                            \
	do {                                                                                            \
		char ctm_str[22];                                                                           \
		ct_current_datetime_string("%Y.%m.%d-%H:%M:%S", ctm_str, sizeof(ctm_str));                  \
		cfatal(STR_CURRTITLE " application exit with code %d at '%s'." STR_NEWLINE, code, ctm_str); \
		ct_log_flush();                                                                             \
	} while (0)

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_app_ptr_t ct_app_create(void)
{
	ct_app_start();                                   // 输出启动信息
	ct_app_exception_init(ct_app_exception_handler);  // 初始化异常处理函数
	app->thpool = ct_thpool_global_create(16, 50);    // 创建全局线程池
	ct_timer_mgr_init();                              // 初始化定时器中枢
	ct_evmsg_mgr_init();                              // 初始化事件消息中枢
	return app;
}

int ct_app_exec(ct_app_ptr_t self)
{
#define CT_APP_SCHEDULE_INTERVAL 10         // 调度间隔
	self->tid      = ct_thread_tid();       // 记录主线程ID
	self->abnormal = setjmp(self->jmpbuf);  // 记录上下文信息
	for (; !self->abnormal;) {
		ct_log_mgr_schedule();                       // 执行日志调度
		ct_timer_mgr_schedule();                     // 执行定时器调度
		ct_evmsg_mgr_schedule();                     // 执行事件消息调度
		ct_thread_msleep(CT_APP_SCHEDULE_INTERVAL);  // 调度间隔
	}
	self->tid = 0;                   // 重置主线程ID
	ct_app_end(self->abnormal);      // 输出结束信息
	ct_log_flush();                  // 刷新日志缓冲区
	ct_app_exception_init(SIG_DFL);  // 重置异常处理函数
	if (CT_APP_IS_SIGNAL(self->abnormal)) {
		raise(self->abnormal);  // 如果为系统信号，则调用 raise 发送信号
	} else {
		ct_thpool_destroy(self->thpool);  // 销毁线程池
	}
	return self->abnormal;
}

void ct_app_exit(int status)
{
	// 检查是否在运行中
	if (!app->tid) {
		ct_app_end(status);  // 输出结束信息
		ct_log_flush();      // 刷新日志缓冲区
		exit(status);        // 直接退出程序
	}
	// 如果当前线程是主线程, 则跳转到记录的位置
	if (ct_thread_tid() == app->tid) {
		longjmp(app->jmpbuf, status);
	}
	// 设置异常标志
	app->abnormal = status;
	// 死循环，等待主线程退出
	ct_forever {
		ct_thread_msleep(100);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ct_app_exception_init(ct_app_exception_handler_t handler)
{
	signal(SIGHUP, handler);
	signal(SIGINT, handler);
	signal(SIGQUIT, handler);
	signal(SIGILL, handler);
	signal(SIGTRAP, handler);
	signal(SIGABRT, handler);
	signal(SIGBUS, handler);
	signal(SIGFPE, handler);
	signal(SIGUSR1, handler);
	signal(SIGSEGV, handler);
	signal(SIGUSR2, handler);
	signal(SIGPIPE, handler);
	signal(SIGALRM, handler);
	signal(SIGTERM, handler);
#ifdef SIGSTKFLT
	signal(SIGSTKFLT, handler);
#endif
}

static inline void ct_app_program_backtrace(void)
{
#define BACKTRACE_SIZE 100
#if defined(CT_OS_LINUX)
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
		cfatal_n("[--] ---- backtrace start ---- " STR_NEWLINE);
		for (int i = 0; i < count; i++) {
			cfatal_n("[%02d] %s" STR_NEWLINE, i, symbols[i]);
		}
		cfatal_n("[--] ---- backtrace end ---- " STR_NEWLINE);
	}
	// 释放内存
	free(symbols);
#elif defined(CT_OS_WIN)
	// 用于存储堆栈信息的缓冲区
	void* backtrace[BACKTRACE_SIZE];
	// 获取堆栈信息
	HANDLE  process = GetCurrentProcess();
	HANDLE  thread  = GetCurrentThread();
	CONTEXT context;
	RtlCaptureContext(&context);
	DWORD capturedFrames = CaptureStackBackTrace(0, BACKTRACE_SIZE, backtrace, ct_nullptr);

	// 初始化符号处理
	SymInitialize(process, ct_nullptr, TRUE);

	// 输出堆栈信息
	cfatal_n("[--] ---- backtrace start ---- " STR_NEWLINE);
	for (DWORD i = 0; i < capturedFrames; i++) {
		DWORD64      offset = 0;
		char         buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		PSYMBOL_INFO symbol  = (PSYMBOL_INFO)buffer;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen   = MAX_SYM_NAME;

		if (SymFromAddr(process, (DWORD64)backtrace[i], &offset, symbol)) {
			cfatal_n("[%02d] %s+0x%llX" STR_NEWLINE, i, symbol->Name, offset);
		} else {
			cfatal_n("[%02d] 0x%p" STR_NEWLINE, i, backtrace[i]);
		}
	}
	cfatal_n("[--] ---- backtrace end ---- " STR_NEWLINE);

	// 清理符号处理
	SymCleanup(process);
#else
#endif
}

static inline void ct_app_exception_handler(int _signal)
{
	// 避免多线程同时出错时, 异常处理函数被调用多次
	{
		static ct_mutex_buf_t mutex = {CT_MUTEX_INITIALIZATION};
		if (!ct_mutex_try_lock(mutex)) {
			ct_forever {
				ct_thread_msleep(100);
			}
		}
	}
	// 检查是否是系统信号
	if (CT_APP_IS_SIGNAL(_signal)) {
		// 输出堆栈标志
		bool is_backtrace = true;
		// 打印异常信息
		switch (_signal) {
			case SIGHUP:
				cfatal(STR_CURRTITLE " Terminal line hangup." STR_NEWLINE);
				is_backtrace = false;
				break;
			case SIGINT:
				cfatal(STR_CURRTITLE " Interrupt program." STR_NEWLINE);
				is_backtrace = false;
				break;
			case SIGQUIT:
				cfatal(STR_CURRTITLE " Quit program." STR_NEWLINE);
				is_backtrace = false;
				break;
			case SIGILL: cfatal(STR_CURRTITLE " Illegal instruction." STR_NEWLINE); break;
			case SIGTRAP: cfatal(STR_CURRTITLE " Trace trap." STR_NEWLINE); break;
			case SIGABRT: cfatal(STR_CURRTITLE " Abort program." STR_NEWLINE); break;
			case SIGBUS: cfatal(STR_CURRTITLE " Bus error." STR_NEWLINE); break;
			case SIGFPE: cfatal(STR_CURRTITLE " Floating point exception." STR_NEWLINE); break;
			case SIGSEGV: cfatal(STR_CURRTITLE " Segmentation fault." STR_NEWLINE); break;
			case SIGTERM: cfatal(STR_CURRTITLE " Termination request." STR_NEWLINE); break;
			default: cfatal(STR_CURRTITLE " Unknown signal %d." STR_NEWLINE, _signal); break;
		}
		// 输出堆栈信息
		if (is_backtrace) {
			ct_app_program_backtrace();
		}
	}
	// 检查主线程是否在运行中
	if (!app->tid) {
		ct_log_flush();  // 刷新日志缓冲区
		exit(_signal);   // 直接退出程序
	}
	// 如果当前线程是主线程, 则跳转到记录的位置
	if (ct_thread_tid() == app->tid) {
		longjmp(app->jmpbuf, _signal);
	}
	// 设置异常标志
	app->abnormal = _signal;
	// 死循环，等待主线程退出
	ct_forever {
		ct_thread_msleep(100);
	}
}
