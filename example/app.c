/**
 * @file app.c
 * @brief Application 实例
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#include "app.h"

#include <setjmp.h>

#ifdef CT_OS_WIN
// #if _MSC_VER
// #include <DbgHelp.h>
// #pragma comment(lib, "Dbghelp.lib")
// #endif
#else
#include <execinfo.h>
#endif

#include "base/ct_datetime.h"
#include "base/ct_platform.h"
#include "base/ct_time.h"
#include "excep.h"
#include "mech/ct_cron.h"
#include "mech/ct_evmsg.h"
#include "mech/ct_jobpool.h"
#include "mech/ct_log.h"
#include "mech/ct_msgqueue.h"
#include "mech/ct_thpool.h"
#include "mech/ct_timer.h"
#include "mech/log/ct_log_msg_asyn.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[app]"

/**
 * @brief coter 应用实例
 */
static struct app {
	bool              is_run; // 是否运行中
	pthread_t         tid;         // 主线程ID
	jmp_buf           jmp;         // 上下文信息
	excep_t           exitBuf[1];  // 异常退出缓冲区
	ct_msgqueue_t     exitMQ[1];   // 异常退出队列
	ct_thpool_ptr_t   thpool;      // 全局线程池
	ct_jobpool_ptr_t  jobpool;     // 全局任务池
} gapp[1] = {{
	.is_run  = false,
	.thpool  = ct_nullptr,
	.jobpool = ct_nullptr,
}};

// 初始化异常处理
static inline void app_exception_init(void);
// 打印堆栈信息
static inline void app_program_backtrace(void);
// 异常发生
static inline void app_occurred(const excep_t* excep);
// 启动
static inline void app_welcome(void);
// 结束
static inline void app_goobye(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

app_ptr_t app_create(void) {
	const ct_time_t   now  = ct_current_second();
	const ct_time64_t tick = gettick_ms();

	app_welcome();                                                      // 输出启动信息
	app_exception_init();                                               // 初始化异常处理函数
	ct_msgqueue_init(gapp->exitMQ, gapp->exitBuf, sizeof(excep_t), 1);  // 初始化异常退出队列
	gapp->thpool  = ct_thpool_global(NULL);                             // 创建全局线程池
	gapp->jobpool = ct_jobpool_global(16, 50);                          // 创建全局任务池
	ct_timer_mgr_init(tick);                                            // 初始化定时器中枢
	ct_evmsg_mgr_init();                                                // 初始化事件消息中枢
	ct_cron_mgr_init(now / 1000);                                       // 初始化cron任务中枢
	return gapp;
}

int app_exec(app_ptr_t self) {
	// 记录主线程ID
	self->tid = pthread_self();
	self->is_run = true;

	// 记录上下文信息
	if (setjmp(self->jmp) != 0) {
		goto Fail;
	}

	excep_t     excep;
	int         count = 0;
	ct_time64_t tick;
	ct_time_t   now;

	ct_forever {
		now  = ct_current_second();
		tick = gettick_ms();

		if (++count > 10) {
			count = 0;
			if (ct_msgqueue_try_dequeue(gapp->exitMQ, &excep)) {
				app_occurred(&excep);
				break;
			}
		}

		ct_cron_mgr_schedule(now);    // 执行cron任务调度
		ct_timer_mgr_schedule(tick);  // 执行定时器调度
		ct_evmsg_schedule();          // 执行事件消息调度
		ct_log_mgr_schedule();        // 执行日志调度
		ct_msleep(10);                // 调度间隔 (10ms)
	}

	// ct_exception_init(SIG_DFL);  // 重置异常处理函数
	// 	raise(self->abnormal);  // 如果为系统信号，则调用 raise 发送信号
	// 	ct_thpool_destroy(self->thpool);  // 销毁线程池
	// 	ct_jobpool_destroy(self->jobpool);  // 销毁任务池

Fail:
	ct_log_flush();  // 刷新日志缓冲区
	app_goobye();    // 输出结束信息
	return EXIT_FAILURE;
}

void app_exit(int code, const char* msg) {
	// 打印堆栈信息
	app_program_backtrace();

	// 检查主线程是否在运行中
	if (!gapp->is_run) {
		ct_log_flush();      // 刷新日志缓冲区
		exit(EXIT_FAILURE);  // 直接退出程序
	}

	// 发送异常退出消息
	const excep_t excep = EXCEP_INIT(code, msg, false);
	ct_msgqueue_enqueue(gapp->exitMQ, &excep);

	// 如果当前线程是主线程, 则跳转到记录的位置
	if (pthread_equal(pthread_self(), gapp->tid)) {
		longjmp(gapp->jmp, EXIT_FAILURE);
	}

	// 死循环，等待主线程退出
	ct_forever {
		ct_msleep(1000);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

#ifdef _WIN32
static BOOL WINAPI app_console_ctrl_handler(DWORD CtrlType) {
	excep_t excep;
	switch (CtrlType) {
		case CTRL_C_EVENT:
			excep_init(&excep, SIGINT, "CTRL+C pressed", true);
			ct_msgqueue_enqueue(gapp->exitMQ, &excep);
			return TRUE;
		case CTRL_BREAK_EVENT:
			excep_init(&excep, SIGINT, "CTRL+BREAK pressed", true);
			ct_msgqueue_enqueue(gapp->exitMQ, &excep);
			return TRUE;
		case CTRL_CLOSE_EVENT:
			excep_init(&excep, SIGTERM, "Window close event", true);
			ct_msgqueue_enqueue(gapp->exitMQ, &excep);
			return TRUE;
	}
	return FALSE;
}
#endif
static inline void app_exception_handler(int sig) {
	// 打印堆栈信息
	app_program_backtrace();

	// 检查主线程是否在运行中
	if (!gapp->is_run) {
		ct_log_flush();      // 刷新日志缓冲区
		exit(EXIT_FAILURE);  // 直接退出程序
	}

	// 发送异常退出消息
	excep_t excep = EXCEP_INIT(sig, "", true);
	switch (sig) {
		case SIGINT: excep.msg = "SIGINT pressed"; break;
		case SIGTERM: excep.msg = "SIGTERM pressed"; break;
		case SIGABRT: excep.msg = "SIGABRT pressed"; break;
	}
	ct_msgqueue_enqueue(gapp->exitMQ, &excep);

	// 如果当前线程是主线程, 则跳转到记录的位置
	if (pthread_equal(pthread_self(), gapp->tid)) {
		longjmp(gapp->jmp, sig);
	}

	// 死循环，等待主线程退出
	ct_forever {
		ct_msleep(1000);
	}
}

static inline void app_exception_init(void) {
#ifdef _WIN32
	// 设置控制台事件处理函数
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)app_console_ctrl_handler, TRUE);
#endif
	signal(SIGINT, app_exception_handler);
	signal(SIGTERM, app_exception_handler);
	signal(SIGABRT, app_exception_handler);
}

static inline void app_program_backtrace(void) {
#define BACKTRACE_SIZE 100
#ifdef CT_OS_UNIX
	// 缓存区
	void* buffer[BACKTRACE_SIZE];
	// 获取函数调用堆栈信息
	const int count = backtrace(buffer, BACKTRACE_SIZE);
	// 获取堆栈信息对应的符号名称
	char** symbols = backtrace_symbols(buffer, count);
	if (!symbols) {
		perror("backtrace symbols");
		exit(EXIT_FAILURE);
	}
	// 打印堆栈信息
	cfatal_n("[--] ---- backtrace start ---- " STR_NEWLINE);
	for (int i = 0; i < count; i++) {
		cfatal_n("[%02d] %s" STR_NEWLINE, i, symbols[i]);
	}
	cfatal_n("[--] ---- backtrace end ---- " STR_NEWLINE);

	// 释放内存
	free(symbols);
#endif
}

static inline void app_occurred(const excep_t* excep) {
	if (excep == ct_nullptr) {
		cerror(STR_CURRTITLE " error occurred (unknown)." STR_NEWLINE);
	} else {
		cerror(STR_CURRTITLE " error occurred (%s)." STR_NEWLINE, excep->msg);
	}
}

static inline void app_welcome(void) {
	char                str[CT_DATETIME_FMT_BUFLEN];
	const ct_datetime_t now = ct_datetime_now();
	ct_datetime_fmt(&now, str);
	ctrace(STR_CURRTITLE " application start at '%s'." STR_NEWLINE, str);
}

static inline void app_goobye(void) {
	char                str[CT_DATETIME_FMT_BUFLEN];
	const ct_datetime_t now = ct_datetime_now();
	ct_datetime_fmt(&now, str);
	ctrace(STR_CURRTITLE " application exit at '%s'." STR_NEWLINE, str);
	ct_log_flush();
}
