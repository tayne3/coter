/**
 * @file test_log.c
 * @brief 日志测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "base/ct_platform.h"
#include "ctunit.h"
#include "mech/ct_log.h"

#define test_basic_verbose(__logger, ...) CTLogger_HandleBasic(VarBase, (__logger), __VA_ARGS__)
#define test_basic_debug(__logger, ...)   CTLogger_HandleBasic(Debug, (__logger), __VA_ARGS__)
#define test_basic_trace(__logger, ...)   CTLogger_HandleBasic(Trace, (__logger), __VA_ARGS__)
#define test_basic_warning(__logger, ...) CTLogger_HandleBasic(Warning, (__logger), __VA_ARGS__)
#define test_basic_error(__logger, ...)   CTLogger_HandleBasic(Error, (__logger), __VA_ARGS__)
#define test_basic_fatal(__logger, ...)   CTLogger_HandleBasic(Fatal, (__logger), __VA_ARGS__)

#define test_brief_verbose(__logger, ...) CTLogger_HandleBrief(VarBase, (__logger), __VA_ARGS__)
#define test_brief_debug(__logger, ...)   CTLogger_HandleBrief(Debug, (__logger), __VA_ARGS__)
#define test_brief_trace(__logger, ...)   CTLogger_HandleBrief(Trace, (__logger), __VA_ARGS__)
#define test_brief_warning(__logger, ...) CTLogger_HandleBrief(Warning, (__logger), __VA_ARGS__)
#define test_brief_error(__logger, ...)   CTLogger_HandleBrief(Error, (__logger), __VA_ARGS__)
#define test_brief_fatal(__logger, ...)   CTLogger_HandleBrief(Fatal, (__logger), __VA_ARGS__)

#define test_detail_verbose(__logger, ...) CTLogger_HandleDetail(VarBase, (__logger), __VA_ARGS__)
#define test_detail_debug(__logger, ...)   CTLogger_HandleDetail(Debug, (__logger), __VA_ARGS__)
#define test_detail_trace(__logger, ...)   CTLogger_HandleDetail(Trace, (__logger), __VA_ARGS__)
#define test_detail_warning(__logger, ...) CTLogger_HandleDetail(Warning, (__logger), __VA_ARGS__)
#define test_detail_error(__logger, ...)   CTLogger_HandleDetail(Error, (__logger), __VA_ARGS__)
#define test_detail_fatal(__logger, ...)   CTLogger_HandleDetail(Fatal, (__logger), __VA_ARGS__)

#define TEST_THREADS     20
#define TEST_THREAD_DATA 1000

static ct_logger_t* g_logger = NULL;
static pthread_t    g_thread_logger;
static bool         is_exit = false;

// 日志调度线程函数
static inline void* thread_logger_schedule(void* arg);

// 带日志的测试线程函数
static inline void* thread_print_with_basic_log(void* arg);
static inline void* thread_print_with_brief_log(void* arg);
static inline void* thread_print_with_detail_log(void* arg);
// 不带日志的测试线程函数
static inline void* thread_print_without_log(void* arg);
// 打印性能对比测试函数
static inline void test_print_performance_comparison(void);

int main(void) {
	test_print_performance_comparison();
	ctunit_trace("Finish! test_print_performance_comparison();\n");

	ctunit_pass();
}

static inline void* thread_logger_schedule(void* arg) {
	ctunit_assert_not_null(g_logger);
	for (; !is_exit;) {
		ct_logger_schedule(g_logger);
		ct_msleep(10);
	}
	pthread_exit(NULL);
	return NULL;
	(void)arg;
}

static inline void* thread_print_with_basic_log(void* arg) {
	ctunit_assert_not_null(g_logger);
	int tid = (int)(uintptr_t)arg;
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		test_basic_trace(g_logger, "<%d> test2(%d)\n", tid, i + 1);
	}
	pthread_exit(NULL);
	return NULL;
	(void)arg;
}

static inline void* thread_print_with_brief_log(void* arg) {
    ctunit_assert_not_null(g_logger);
    int tid = (int)(uintptr_t)arg;
    for (int i = 0; i < TEST_THREAD_DATA; i++) {
        test_brief_trace(g_logger, "<%d> test3(%d)\n", tid, i + 1);
    }
    pthread_exit(NULL);
    return NULL;
    (void)arg;
}

static inline void* thread_print_with_detail_log(void* arg) {
    ctunit_assert_not_null(g_logger);
    int tid = (int)(uintptr_t)arg;
    for (int i = 0; i < TEST_THREAD_DATA; i++) {
        test_detail_trace(g_logger, "<%d> test4(%d)\n", tid, i + 1);
        sched_yield();
    }
    pthread_exit(NULL);
    return NULL;
    (void)arg;
}


static inline void* thread_print_without_log(void* arg) {
	int tid = (int)(uintptr_t)arg;
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		printf("<%d> test1(%d)\n", tid, i + 1);
		sched_yield();
	}
	pthread_exit(NULL);
	return NULL;
	(void)arg;
}

static inline void test_print_performance_comparison(void) {
	pthread_t   threads[TEST_THREADS];
	ct_time64_t start, end;
	bool        is_ok;

	// 创建 Logger
	{
		ct_log_config_t config = {
			.level         = CTLog_LevelVarBase,
			.disable_print = false,

			.disable_save      = true,
			.file_dir          = "",
			.file_name         = "",
			.file_cache_size   = 1024,
			.file_size_max     = 1024 * 1024,
			.file_count_max    = 10,
			.autosave_interval = 10,

			.callback_routine  = NULL,
			.callback_userdata = NULL,
		};
		g_logger = ct_logger_create(&config);
		ctunit_assert_not_null(g_logger);
	}

	is_ok = 0 == pthread_create(&g_thread_logger, NULL, thread_logger_schedule, NULL);
	ctunit_assert_true(is_ok);

	start = gettick_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, thread_print_without_log, (void*)(uintptr_t)i);
		ctunit_assert_true(is_ok);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		ctunit_assert_true(is_ok);
	}
	end                        = gettick_ms();
	const int time_without_log = (int)(end - start);


    start = gettick_ms();
    for (int i = 0; i < TEST_THREADS; i++) {
        is_ok = 0 == pthread_create(&threads[i], NULL, thread_print_with_basic_log, (void*)(uintptr_t)i);
        ctunit_assert_true(is_ok);
    }
    for (int i = 0; i < TEST_THREADS; i++) {
        is_ok = 0 == pthread_join(threads[i], NULL);
        ctunit_assert_true(is_ok);
    }
    end = gettick_ms();
    const int time_with_basic_log = (int)(end - start);

    // 新增 brief log 测试
    start = gettick_ms();
    for (int i = 0; i < TEST_THREADS; i++) {
        is_ok = 0 == pthread_create(&threads[i], NULL, thread_print_with_brief_log, (void*)(uintptr_t)i);
        ctunit_assert_true(is_ok);
    }
    for (int i = 0; i < TEST_THREADS; i++) {
        is_ok = 0 == pthread_join(threads[i], NULL);
        ctunit_assert_true(is_ok);
    }
    end = gettick_ms();
    const int time_with_brief_log = (int)(end - start);

    // 新增 detail log 测试
    start = gettick_ms();
    for (int i = 0; i < TEST_THREADS; i++) {
        is_ok = 0 == pthread_create(&threads[i], NULL, thread_print_with_detail_log, (void*)(uintptr_t)i);
        ctunit_assert_true(is_ok);
    }
    for (int i = 0; i < TEST_THREADS; i++) {
        is_ok = 0 == pthread_join(threads[i], NULL);
        ctunit_assert_true(is_ok);
    }
	
    end = gettick_ms();
    const int time_with_detail_log = (int)(end - start);

    is_exit = true;
    is_ok   = 0 == pthread_join(g_thread_logger, NULL);
    ctunit_assert_true(is_ok);

    ct_logger_destroy(g_logger);
    g_logger = NULL;

    ctunit_trace("Execution time: without log %d ms, with basic log %d ms, with brief log %d ms, with detail log %d ms\n",
                 time_without_log, time_with_basic_log, time_with_brief_log, time_with_detail_log);
}
