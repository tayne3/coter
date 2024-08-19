/**
 * @file dump_unix.c
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#include "dump_unix.h"

#include "app.h"
#include "mech/ct_log.h"

#ifdef CT_OS_UNIX

#include <execinfo.h>
#include <signal.h>
#include <sys/resource.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[dump_unix]"

#define TRACE_MAX_STACK_FRAMES 100

/**
 * @brief 信号处理函数
 * @param sig 接收到的信号
 */
static inline void signal_handler(int sig);

/**
 * @brief 生成并打印当前程序的堆栈跟踪信息
 */
static void generate_backtrace(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

void print_stack_trace(void) {
	generate_backtrace();
}

#include <unistd.h>

void exception_init(void) {
	struct sigaction sa;
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);

	// 检查是否启用了core dump
	struct rlimit core_limit;
	getrlimit(RLIMIT_CORE, &core_limit);
	if (core_limit.rlim_cur == 0) {
		printf("Core dump is disabled. Enabling it...\n");
		core_limit.rlim_cur = RLIM_INFINITY;
		core_limit.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE, &core_limit);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void signal_handler(int sig) {
	app_crash(sig, strsignal(sig));
}

static void generate_backtrace(void) {
	void* buffer[TRACE_MAX_STACK_FRAMES];

	const int count   = backtrace(buffer, TRACE_MAX_STACK_FRAMES);
	char**    symbols = backtrace_symbols(buffer, count);
	if (!symbols) {
		perror("backtrace symbols");
		exit(EXIT_FAILURE);
	}

	cerror_n("thread %p [running]:" STR_NEWLINE, pthread_self());
	for (int i = 0; i < count; i++) {
		cerror_n("%s\n", symbols[i]);
	}
	cerror_n(STR_NEWLINE);

	free(symbols);
}

#endif
