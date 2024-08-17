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
	pthread_t         tid;         // 主线程ID
	jmp_buf           jmp;         // 上下文信息
	excep_t           exitBuf[1];  // 异常退出缓冲区
	ct_msgqueue_buf_t exitMQ;      // 异常退出队列
	ct_time64_t       timecurr;    // 当前调度时间
	ct_thpool_ptr_t   thpool;      // 全局线程池
	ct_jobpool_ptr_t  jobpool;     // 全局任务池
} gapp[1] = {{
	.tid    = 0,
	.thpool = ct_nullptr,
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
	app_welcome();                                                      // 输出启动信息
	app_exception_init();                                               // 初始化异常处理函数
	ct_msgqueue_init(gapp->exitMQ, gapp->exitBuf, sizeof(excep_t), 1);  // 初始化异常退出队列
	gapp->thpool  = ct_thpool_global(NULL);                             // 创建全局线程池
	gapp->jobpool = ct_jobpool_global(16, 50);                          // 创建全局任务池
	ct_timer_mgr_init();                                                // 初始化定时器中枢
	ct_evmsg_mgr_init();                                                // 初始化事件消息中枢
	return gapp;
}

int app_exec(app_ptr_t self) {
	// 记录主线程ID
	self->tid = pthread_self();

	// 记录上下文信息
	if (setjmp(self->jmp) != 0) {
		goto Fail;
	}

	excep_t     excep;
	int         count = 0;
	ct_time64_t now, tick;

	ct_forever {
		now  = gettimeofday_ms();
		tick = gettick_ms();

		if (++count > 10) {
			count = 0;
			if (ct_msgqueue_try_dequeue(gapp->exitMQ, &excep)) {
				app_occurred(&excep);
				break;
			}
		}

		ct_timer_mgr_schedule(now, tick);  // 执行定时器调度
		ct_evmsg_schedule();               // 执行事件消息调度
		ct_log_mgr_schedule();             // 执行日志调度
		ct_msleep(10);                     // 调度间隔 (10ms)
	}

	// self->tid = 0;              // 重置主线程ID
	// ct_goobye(self->abnormal);  // 输出结束信息
	// ct_log_flush();             // 刷新日志缓冲区

	// ct_exception_init(SIG_DFL);  // 重置异常处理函数
	// if (CT_APP_IS_SIGNAL(self->abnormal)) {
	// 	raise(self->abnormal);  // 如果为系统信号，则调用 raise 发送信号
	// } else {
	// 	ct_thpool_destroy(self->thpool);  // 销毁线程池
	// 	ct_jobpool_destroy(self->jobpool);  // 销毁任务池
	// }
	// return self->abnormal;

Fail:
	ct_log_flush();  // 刷新日志缓冲区
	app_goobye();    // 输出结束信息
	return EXIT_FAILURE;
}

void app_exit(int code, const char* msg) {
	// 打印堆栈信息
	app_program_backtrace();
	// 发送异常退出消息
	{
		const excep_t excep = EXCEP_INIT(code, msg, false);
		ct_msgqueue_enqueue(gapp->exitMQ, &excep);
	}
	// 如果当前线程是主线程, 则跳转到记录的位置
	// if (pthread_self() == gapp->tid) {
	//	longjmp(gapp->jmp, EXIT_FAILURE);
	//}
	if (pthread_equal(pthread_self(), gapp->tid)) {
		longjmp(gapp->jmp, EXIT_FAILURE);
	}
	// 死循环，等待主线程退出
	ct_forever {
		ct_msleep(1000);
	}

	// // 检查是否在运行中
	// if (!gapp->tid) {
	// 	ct_goobye(status);  // 输出结束信息
	// 	ct_log_flush();      // 刷新日志缓冲区
	// 	exit(status);        // 直接退出程序
	// }
	// // 如果当前线程是主线程, 则跳转到记录的位置
	// if (ct_thread_self() == gapp->tid) {
	// 	longjmp(gapp->jmp, status);
	// }
	// // 设置异常标志
	// gapp->abnormal = status;
	// // 死循环，等待主线程退出
	// ct_forever {
	// 	ct_msleep(100);
	// }
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
#else
static inline void app_exception_handler(int sig) {
	// 检查主线程是否在运行中
	if (!gapp->tid) {
		ct_log_flush();  // 刷新日志缓冲区
		exit(sig);       // 直接退出程序
	}
	// 发送异常退出消息
	{
		const excep_t excep = CT_EXCEP_INIT(sig, strsignal(sig), true);
		ct_msgqueue_enqueue(gapp->exitMQ, &excep);
	}
	// 如果当前线程是主线程, 则跳转到记录的位置
	// if (ct_thread_self() == gapp->tid) {
	// 	longjmp(gapp->jmp, sig);
	// }
	if (pthread_equal(pthread_self(), gapp->tid)) {
		longjmp(gapp->jmp, sig);
	}
	// 死循环，等待主线程退出
	ct_forever {
		ct_msleep(1000);
	}

	// // 避免多线程同时出错时, 异常处理函数被调用多次
	// if (!ct_mutex_try_lock(gapp->mutex)) {
	// 	ct_forever {
	// 		ct_msleep(100);
	// 	}
	// }
	// // 打印异常信息
	// cfatal(STR_CURRTITLE " catch signal: %s(%d)." STR_NEWLINE, strsignal(sig), sig);
	// // 打印堆栈信息
	// ct_program_backtrace();
	// // 检查主线程是否在运行中
	// if (!gapp->tid) {
	// 	ct_log_flush();  // 刷新日志缓冲区
	// 	exit(sig);       // 直接退出程序
	// }
	// // 如果当前线程是主线程, 则跳转到记录的位置
	// if (ct_thread_self() == gapp->tid) {
	// 	longjmp(gapp->jmp, sig);
	// }
	// // 设置异常标志
	// gapp->abnormal = sig;
	// // 死循环，等待主线程退出
	// ct_forever {
	// 	ct_msleep(100);
	// }
}
#endif

static inline void app_exception_init(void) {
#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)app_console_ctrl_handler, TRUE);
#else
	// struct sigaction action[1];
	// action->sa_handler = ct_exception_handler;
	// sigemptyset(&action->sa_mask);
	// action->sa_flags = 0;

	// if (sigaction(SIGINT, action, ct_nullptr) < 0) {
	// 	exit(EXIT_FAILURE);
	// }
	// if (sigaction(SIGTERM, action, ct_nullptr) < 0) {
	// 	exit(EXIT_FAILURE);
	// }
	signal(SIGINT, ct_exception_handler);
	signal(SIGTERM, ct_exception_handler);
#endif
}

static inline void app_program_backtrace(void) {
#define BACKTRACE_SIZE 100

#if defined(CT_OS_WIN)

	// #define STACK_INFO_LEN 1024

	// 	static const int MAX_STACK_FRAMES = 12;
	// 	void*            pStack[MAX_STACK_FRAMES];
	// 	 char      szStackInfo[STACK_INFO_LEN * MAX_STACK_FRAMES];
	// 	 char      szFrameInfo[STACK_INFO_LEN];
	// 	HANDLE           process = GetCurrentProcess();
	// 	SymInitialize(process, NULL, TRUE);
	// 	WORD frames = CaptureStackBackTrace(0, MAX_STACK_FRAMES, pStack, NULL);
	// 	strcpy(szStackInfo, "stack traceback:\n");
	// 	strcat(szStackInfo, " ");
	// 	for (WORD i = 0; i < frames; ++i) {
	// 		DWORD64      address         = (DWORD64)pStack[i];
	// 		DWORD64      displacementSym = 0;
	// 		char         buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	// 		PSYMBOL_INFO pSymbol             = (PSYMBOL_INFO)buffer;
	// 		pSymbol->SizeOfStruct            = sizeof(SYMBOL_INFO);
	// 		pSymbol->MaxNameLen              = MAX_SYM_NAME;
	// 		DWORD           displacementLine = 0;
	// 		IMAGEHLP_LINE64 line;
	// 		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	// 		if (SymFromAddr(process, address, &displacementSym, pSymbol) &&
	// 			SymGetLineFromAddr64(process, address, &displacementLine, &line)) {
	// 			ct_snprintf(szFrameInfo, sizeof(szFrameInfo), "\t%s() at %s:%d(0x%x)\n", pSymbol->Name, line.FileName,
	// 						line.LineNumber, pSymbol->Address);
	// 		} else {
	// 			ct_snprintf(szFrameInfo, sizeof(szFrameInfo), "\terror: %d\n", GetLastError());
	// 		}
	// 		strcat(szStackInfo, szFrameInfo);
	// 	}
	// 	cfatal_n(szStackInfo);

// #if _MSC_VER
// 	// 用于存储堆栈信息的缓冲区
// 	void* backtrace[BACKTRACE_SIZE];
// 	// 获取堆栈信息
// 	HANDLE  process = GetCurrentProcess();
// 	CONTEXT context;
// 	RtlCaptureContext(&context);
// 	DWORD capturedFrames = CaptureStackBackTrace(0, BACKTRACE_SIZE, backtrace, ct_nullptr);

// 	// 初始化符号处理
// 	SymInitialize(process, ct_nullptr, TRUE);

// 	// 输出堆栈信息
// 	cfatal_n("[--] ---- backtrace start ---- " STR_NEWLINE);
// 	for (DWORD i = 0; i < capturedFrames; i++) {
// 		DWORD64      offset = 0;
// 		char         buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
// 		PSYMBOL_INFO symbol  = (PSYMBOL_INFO)buffer;
// 		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
// 		symbol->MaxNameLen   = MAX_SYM_NAME;

// 		if (SymFromAddr(process, (DWORD64)backtrace[i], &offset, symbol)) {
// 			cfatal_n("[%02d] %s+0x%llX" STR_NEWLINE, i, symbol->Name, offset);
// 		} else {
// 			cfatal_n("[%02d] 0x%p" STR_NEWLINE, i, backtrace[i]);
// 		}
// 	}
// 	cfatal_n("[--] ---- backtrace end ---- " STR_NEWLINE);

// 	// 清理符号处理
// 	SymCleanup(process);
// #endif
#else
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
