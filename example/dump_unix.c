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

#define TRACE_MAX_STACK_FRAMES 100

/**
 * @brief 信号处理函数
 * @param sig 接收到的信号
 */
static inline void signal_handler(int sig);

// -------------------------[GLOBAL DEFINITION]-------------------------

void print_stack_trace(void) {
	void* buffer[TRACE_MAX_STACK_FRAMES];

	const int count = backtrace(buffer, TRACE_MAX_STACK_FRAMES);
	if (count == 0) {
		return;
	}

	char** symbols = backtrace_symbols(buffer, count);
	if (!symbols) {
		perror("backtrace symbols");
		exit(EXIT_FAILURE);
	}

	log_error_n("thread %p [running]:\n", (void*)pthread_self());
	for (int i = 0; i < count; i++) {
		log_error_n("%s\n", symbols[i]);
	}
	log_error_n("\n");

	free(symbols);
}

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
		log_verbose("core dump is disabled, attempting to enable it.\n");

		core_limit.rlim_cur = RLIM_INFINITY;
		core_limit.rlim_max = RLIM_INFINITY;

		if (setrlimit(RLIMIT_CORE, &core_limit) == -1) {
			log_warning("set core dump size limit failed: %s.\n", strerror(errno));
			return;
		}
	}

	char  path[1035];
	FILE* fp = popen("sysctl -n kernel.core_pattern", "r");
	if (fp != NULL) {
		if (fgets(path, sizeof(path) - 1, fp) != NULL) {
			log_warning("core dump path: %s", path);
		} else {
			log_error("get core dump path failed.\n");
		}
		pclose(fp);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void signal_handler(int sig) {
	app_crash(sig, strsignal(sig));
}

#endif
