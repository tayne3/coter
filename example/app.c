/**
 * @file app.c
 * @brief Application 实例
 */
#include "app.h"

#include <setjmp.h>
#include <signal.h>

#include "coter/container/list.h"
#include "coter/event/msgqueue.h"
#include "coter/runtime/thpool.h"
#include "coter/sync/waitgroup.h"
#include "coter/time/cron.h"
#include "coter/time/time.h"
#include "coter/time/timer.h"
#include "excep.h"

#ifdef CT_OS_DARWIN
#include <sys/event.h>
#endif

// -------------------------[STATIC DECLARATION]-------------------------

typedef struct atexit {
	ct_list_t list[1];
	void (*callback)(void);
} atexit_t;

/**
 * @brief coter 应用实例
 */
static struct gapp {
	excep_t       exitBuf[1];  // 异常退出缓冲区
	ct_msgqueue_t exitMQ[1];   // 异常退出队列

	ct_time_t   now;   // 当前时间 (秒级)
	ct_time64_t tick;  // 系统运行时间 (毫秒级)

	ct_thpool_t* thpool;  // 全局线程池

	jmp_buf jmp;         // 上下文信息
	bool    isShutdown;  // 是否关闭

	ct_list_t       atExitList[1];  // 退出列表
	pthread_mutex_t atExitMutex;    // 退出列表互斥锁

#ifdef CT_OS_WIN
	DWORD  mainThreadID;   // 主线程ID
	DWORD  catchThreadID;  // 异常捕捉线程ID
	HANDLE catchThread;    // 异常捕捉线程
	HANDLE logThread;      // 日志线程
#else
	pthread_t mainThread;    // 主线程
	pthread_t catchThread;   // 异常捕捉线程
	pthread_t signalThread;  // 信号捕捉线程
	pthread_t logThread;     // 日志线程
#endif
} gapp[1] = {{
	.now        = 0,
	.tick       = 0,
	.thpool     = NULL,
	.isShutdown = false,
}};

#ifdef CT_OS_WIN
#define IsMainThread()      (GetCurrentThreadId() == gapp->mainThreadID)
#define THREAD_RETURN_T     DWORD
#define THREAD_RETURN_VALUE 0
#else
#define IsMainThread()      pthread_equal(pthread_self(), gapp->mainThread)
#define THREAD_RETURN_T     void*
#define THREAD_RETURN_VALUE NULL
#endif

// 启动
static void ap_welcome(void);
// 结束
static void ap_goobye(void);
// 异常发生
static void ap_occurred(const excep_t* excep);
// 应用崩溃
static void ap_crash(int code, const char* msg, bool is_sig);

// 日志初始化
static int ap_log_init(void);
// 日志销毁
static void ap_log_deinit(void);
// 日志调度线程
static THREAD_RETURN_T ap_log_run(void* arg);

// 异常捕捉线程
static THREAD_RETURN_T ap_catch_thread(void* arg);
// 退出执行
static void ap_atexit_exec(void);

#ifdef CT_OS_WIN
// 控制台控制处理程序
BOOL WINAPI MyConsoleCtrlHandler(DWORD event);
// 未处理异常处理函数
LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* exp);
#else
// 信号处理回调
static void ap_signal_handler(int sig);
// 信号捕捉线程
static void* ap_signal_thread(void* arg);
#endif

// -------------------------[GLOBAL DEFINITION]-------------------------

gapp_t* gapp_create(void) {
	ct_msgqueue_init(gapp->exitMQ, gapp->exitBuf, sizeof(excep_t), 1);
	ct_list_init(gapp->atExitList);
	pthread_mutex_init(&gapp->atExitMutex, NULL);

#ifdef CT_OS_WIN
	gapp->mainThreadID = GetCurrentThreadId();
	SetConsoleCtrlHandler(MyConsoleCtrlHandler, TRUE);
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	gapp->catchThread = CreateThread(NULL, 0, ap_catch_thread, NULL, 0, &gapp->catchThreadID);

	if (ap_log_init() == 0) {
		global_atexit(ap_log_deinit);
		gapp->logThread = CreateThread(NULL, 0, ap_log_run, NULL, 0, NULL);
	}
#else
	gapp->mainThread = pthread_self();

	// 屏蔽所有信号
	// 不能阻止信号 SIGKILL, SIGSTOP 或 SIGTRACE
	// SIGFPE, SIGILL 和 SIGSEGV 信号不是由人为生成的，将不会被阻塞
	{
		sigset_t set;
		sigfillset(&set);
		pthread_sigmask(SIG_BLOCK, &set, NULL);

		struct sigaction sa;
		sa.sa_handler = ap_signal_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		sigaction(SIGFPE, &sa, NULL);
		sigaction(SIGILL, &sa, NULL);
		sigaction(SIGSEGV, &sa, NULL);
	}

	pthread_create(&gapp->signalThread, NULL, ap_signal_thread, NULL);
	pthread_create(&gapp->catchThread, NULL, ap_catch_thread, NULL);

	if (ap_log_init() == 0) {
		global_atexit(ap_log_deinit);
		pthread_create(&gapp->logThread, NULL, ap_log_run, NULL);
	}
#endif

	ap_welcome();

	gapp->thpool = ct_thpool_create(64, NULL);    // 创建全局线程池
	ct_timer_mgr_init(gapp->tick, gapp->thpool);  // 初始化定时器中枢
	ct_cron_mgr_init(gapp->now, gapp->thpool);    // 初始化cron任务中枢
	return gapp;
}

int gapp_exec(gapp_t* self) {
#ifdef CT_OS_WIN
	gapp->mainThreadID = GetCurrentThreadId();
#else
	gapp->mainThread = pthread_self();
#endif

	if (setjmp(self->jmp)) {
		goto Fail;
	}

	for (; !gapp->isShutdown;) {
		gapp->now  = ct_current_second();
		gapp->tick = ct_getuptime_ms();
		ct_cron_mgr_schedule(gapp->now);    // 执行cron任务调度
		ct_timer_mgr_schedule(gapp->tick);  // 执行定时器调度
		ct_msleep(10);
	}

Fail:
	ct_msgqueue_destroy(gapp->exitMQ);  // 销毁异常退出队列
	ap_goobye();                        // 输出结束信息

#ifdef CT_OS_WIN
	WaitForSingleObject(gapp->catchThread, INFINITE);
#else
	pthread_join(gapp->catchThread, NULL);
	pthread_join(gapp->signalThread, NULL);
#endif

	ap_atexit_exec();  // 执行退出回调

	pthread_mutex_destroy(&gapp->atExitMutex);
	exit(EXIT_FAILURE);
	return EXIT_FAILURE;
}

void global_exit(int code, const char* msg) {
	ap_crash(code, msg, false);
}

int global_atexit(void (*callback)(void)) {
	atexit_t* node = (atexit_t*)malloc(sizeof(atexit_t));
	if (!node) {
		return -1;
	}
	node->callback = callback;
	pthread_mutex_lock(&gapp->atExitMutex);
	ct_list_prepend(gapp->atExitList, node->list);  // 使用头插, 先加入的最后执行
	pthread_mutex_unlock(&gapp->atExitMutex);
	return 0;
}

int global_async(void (*routine)(void*), void* arg) {
	if (!gapp->thpool || !routine) {
		return -1;
	}
	return ct_thpool_submit(gapp->thpool, routine, arg);
}

// -------------------------[STATIC DEFINITION]-------------------------

static void ap_welcome(void) {
	char str[CT_TM_FMT_MAX];
	gapp->now  = ct_current_second();
	gapp->tick = ct_getuptime_ms();
	struct tm tm;
	ct_localtime_r(&gapp->now, &tm);
	ct_tm_fmt(&tm, str);
	logT("APPLICATION START AT '%s'." STR_NEWLINE, str);
}

static void ap_goobye(void) {
	char str[CT_TM_FMT_MAX];
	gapp->now  = ct_current_second();
	gapp->tick = ct_getuptime_ms();
	struct tm tm;
	ct_localtime_r(&gapp->now, &tm);
	ct_tm_fmt(&tm, str);
	logT("APPLICATION EXIT AT '%s'." STR_NEWLINE, str);
}

static void ap_occurred(const excep_t* excep) {
	logE("ERROR OCCURRED: '%s'." STR_NEWLINE, excep->msg);
}

static void ap_crash(int code, const char* msg, bool is_sig) {
	// 发送异常退出消息
	const excep_t excep = EXCEP_INIT(code, msg, is_sig);
	if (!ct_msgqueue_enqueue(gapp->exitMQ, &excep)) {
		ap_occurred(&excep);
		ap_log_deinit();
		exit(EXIT_FAILURE);
	}

	if (IsMainThread()) {
		for (; !gapp->isShutdown;) {
			sched_yield();
		}
		longjmp(gapp->jmp, code);  // 如果当前线程是主线程, 则跳转到记录的位置
		return;
	}

	// 死循环，等待主线程退出
	for (;;) {
		ct_msleep(1000);
	}
}

static int ap_log_init(void) {
	ct_log_config_t config;
	ct_log_config_default(&config);
	config.level = CTLog_LevelVerbose;
	if (ct_log_init(ct_getuptime_ms(), 1, &config)) {
		fprintf(stderr, "log init failed." STR_NEWLINE);
		ct_log_destroy();
		return -1;
	}
	return 0;
}

static void ap_log_deinit(void) {
#ifdef CT_OS_WIN
	WaitForSingleObject(gapp->logThread, INFINITE);
#else
	pthread_join(gapp->logThread, NULL);
#endif
	ct_log_destroy();
}

static THREAD_RETURN_T ap_log_run(void* arg) {
	for (; !gapp->isShutdown;) {
		ct_log_schedule(ct_getuptime_ms());
		ct_msleep(10);
	}
	return THREAD_RETURN_VALUE;
	ct_unused(arg);
}

static THREAD_RETURN_T ap_catch_thread(void* arg) {
	excep_t excep;
	ct_msgqueue_dequeue(gapp->exitMQ, &excep);
	ap_occurred(&excep);

	gapp->isShutdown = true;
	return THREAD_RETURN_VALUE;
	ct_unused(arg);
}

static void ap_atexit_exec(void) {
	ct_list_t head[1];
	ct_list_init(head);

	pthread_mutex_lock(&gapp->atExitMutex);
	ct_list_splice_next(head, gapp->atExitList);
	pthread_mutex_unlock(&gapp->atExitMutex);

	ct_list_foreach_entry_safe (node, head, atexit_t, list) {
		node->callback();
		free(node);
	}
}

#ifdef CT_OS_WIN
BOOL WINAPI MyConsoleCtrlHandler(DWORD event) {
	switch (event) {
		case CTRL_C_EVENT: {
			ap_crash(EXIT_FAILURE, "CTRL+C", true);
			return TRUE;
		}
		case CTRL_BREAK_EVENT: {
			ap_crash(EXIT_FAILURE, "CTRL+Break", true);
			return TRUE;
		}
		case CTRL_CLOSE_EVENT: {
			ap_crash(EXIT_FAILURE, "Console Close", true);
			return TRUE;
		}
		default: {
			return FALSE;
		}
	}
}

LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* exp) {
	ap_crash(EXIT_FAILURE, "Unhandled Exception", false);
	return EXCEPTION_EXECUTE_HANDLER;
	ct_unused(exp);
}
#elif defined(CT_OS_LINUX)
static void ap_signal_handler(int sig) {
	ap_crash(sig, strsignal(sig), true);
}

static void* ap_signal_thread(void* arg) {
	ct_unused(arg);

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGABRT);
	sigaddset(&set, SIGBUS);

	struct timespec timeout;
	timeout.tv_sec  = 1;
	timeout.tv_nsec = 0;

	excep_t   excep = EXCEP_NULL;
	siginfo_t info;
	for (; !gapp->isShutdown;) {
		if (sigtimedwait(&set, &info, &timeout) == -1) {
			continue;
		}
		excep.code   = info.si_signo;
		excep.msg    = strsignal(info.si_signo);
		excep.is_sig = true;
		if (ct_msgqueue_enqueue(gapp->exitMQ, &excep)) {
			break;
		} else {
			ap_occurred(&excep);
			ap_log_deinit();
			exit(EXIT_FAILURE);
		}
	}

	return NULL;
}
#elif defined(CT_OS_DARWIN)
static void ap_signal_handler(int sig) {
	ap_crash(sig, strsignal(sig), true);
}

static void* ap_signal_thread(void* arg) {
	ct_unused(arg);

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGABRT);
	sigaddset(&set, SIGBUS);

	int kq = kqueue();
	if (kq == -1) {
		perror("kqueue");
		return NULL;
	}
	struct kevent changes[4];
	EV_SET(&changes[0], SIGINT, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
	EV_SET(&changes[1], SIGTERM, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
	EV_SET(&changes[2], SIGABRT, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
	EV_SET(&changes[3], SIGBUS, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
	if (kevent(kq, changes, 4, NULL, 0, NULL) == -1) {
		perror("kevent register");
		close(kq);
		return NULL;
	}

	struct timespec timeout;
	timeout.tv_sec      = 1;
	timeout.tv_nsec     = 0;
	excep_t       excep = EXCEP_NULL;
	struct kevent events[1];

	for (; !gapp->isShutdown;) {
		int n = kevent(kq, NULL, 0, events, 1, &timeout);
		if (n == -1) {
			if (errno == EINTR) {
				continue;
			}
			perror("kevent wait");
			break;
		}
		if (n == 0) {
			continue;  // Timeout
		}
		if (events[0].filter == EVFILT_SIGNAL) {
			int sig      = (int)events[0].ident;
			excep.code   = sig;
			excep.msg    = strsignal(sig);
			excep.is_sig = true;
			if (ct_msgqueue_enqueue(gapp->exitMQ, &excep)) {
				break;
			} else {
				ap_occurred(&excep);
				ap_log_deinit();
				exit(EXIT_FAILURE);
			}
		}
	}

	close(kq);
	return NULL;
}
#else
#error "Unsupported platform"
#endif
