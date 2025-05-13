/**
 * @file log_print_test.c
 * @brief 日志打印测试
 * @author tayne3@dingtalk.com
 * @date 2024.11.25
 */
#include "base/ct_platform.h"
#include "ctunit.h"
#include "mech/ct_log.h"

#define test_basic_verbose(...) CTLogger_HandleBasic(Verbose, 0, __VA_ARGS__)
#define test_basic_debug(...)   CTLogger_HandleBasic(Debug, 0, __VA_ARGS__)
#define test_basic_trace(...)   CTLogger_HandleBasic(Trace, 0, __VA_ARGS__)
#define test_basic_warning(...) CTLogger_HandleBasic(Warning, 0, __VA_ARGS__)
#define test_basic_error(...)   CTLogger_HandleBasic(Error, 0, __VA_ARGS__)
#define test_basic_fatal(...)   CTLogger_HandleBasic(Fatal, 0, __VA_ARGS__)

#define test_brief_verbose(...) CTLogger_HandleBrief(Verbose, 0, __VA_ARGS__)
#define test_brief_debug(...)   CTLogger_HandleBrief(Debug, 0, __VA_ARGS__)
#define test_brief_trace(...)   CTLogger_HandleBrief(Trace, 0, __VA_ARGS__)
#define test_brief_warning(...) CTLogger_HandleBrief(Warning, 0, __VA_ARGS__)
#define test_brief_error(...)   CTLogger_HandleBrief(Error, 0, __VA_ARGS__)
#define test_brief_fatal(...)   CTLogger_HandleBrief(Fatal, 0, __VA_ARGS__)

#define test_detail_verbose(...) CTLogger_HandleDetail(Verbose, 0, __VA_ARGS__)
#define test_detail_debug(...)   CTLogger_HandleDetail(Debug, 0, __VA_ARGS__)
#define test_detail_trace(...)   CTLogger_HandleDetail(Trace, 0, __VA_ARGS__)
#define test_detail_warning(...) CTLogger_HandleDetail(Warning, 0, __VA_ARGS__)
#define test_detail_error(...)   CTLogger_HandleDetail(Error, 0, __VA_ARGS__)
#define test_detail_fatal(...)   CTLogger_HandleDetail(Fatal, 0, __VA_ARGS__)

#define TEST_THREADS     2
#define TEST_THREAD_DATA 10000

static pthread_t g_thread_logger;
static bool      is_exit = false;

// 辅助函数: 日志调度线程函数
static inline void* thread_log_schedule(void* arg) {
	for (; !is_exit;) {
		ct_log_schedule(ct_getuptime_ms());
		ct_msleep(10);
	}
	return NULL;
	(void)arg;
}

// 辅助函数: 带日志的测试线程函数
static inline void* thread_print_with_basic_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		test_basic_trace("%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234,
						 1234, 1234, 1234, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL,
						 (void*)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 辅助函数: 带日志的测试线程函数
static inline void* thread_print_with_brief_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		test_brief_trace("%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234,
						 1234, 1234, 1234, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL,
						 (void*)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 辅助函数: 带日志的测试线程函数
static inline void* thread_print_with_detail_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		test_detail_trace("%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234,
						  1234, 1234, 1234, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL,
						  (void*)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 辅助函数: 不带日志的测试线程函数
static inline void* thread_print_without_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		printf("%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234, 1234,
			   1234, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, "test1",
			   "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 性能对比测试
static inline void test_print_performance_comparison(void) {
	pthread_t   threads[TEST_THREADS] = {0};
	ct_time64_t start, end;

	// 创建 Logger
	{
		ct_log_config_t config = {
			.level         = CTLog_LevelVerbose,
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
		const int ret = ct_log_init(ct_getuptime_ms(), 1, &config);
		ctunit_assert_int32_equal(ret, 0);

		ctunit_assert_true(ct_log_is_enable(0, CTLog_LevelVerbose));
		ctunit_assert_false(ct_log_is_enable(1, CTLog_LevelVerbose));
	}

	ctunit_assert_int32_equal(pthread_create(&g_thread_logger, NULL, thread_log_schedule, NULL), 0);

	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_create(&threads[i], NULL, thread_print_without_log, NULL), 0);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_join(threads[i], NULL), 0);
	}
	end                        = ct_getuptime_ms();
	const int time_without_log = (int)(end - start);

	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_create(&threads[i], NULL, thread_print_with_basic_log, NULL), 0);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_join(threads[i], NULL), 0);
	}
	end                           = ct_getuptime_ms();
	const int time_with_basic_log = (int)(end - start);

	// 新增 brief log 测试
	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_create(&threads[i], NULL, thread_print_with_brief_log, NULL), 0);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_join(threads[i], NULL), 0);
	}
	end                           = ct_getuptime_ms();
	const int time_with_brief_log = (int)(end - start);

	// 新增 detail log 测试
	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_create(&threads[i], NULL, thread_print_with_detail_log, NULL), 0);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int32_equal(pthread_join(threads[i], NULL), 0);
	}

	end                            = ct_getuptime_ms();
	const int time_with_detail_log = (int)(end - start);

	is_exit = true;
	ctunit_assert_int32_equal(pthread_join(g_thread_logger, NULL), 0);

	ct_log_flush();
	ct_log_schedule(ct_getuptime_ms());

	ctunit_trace(
		"Execution time: without log %d ms, with basic log %d ms, with brief log %d ms, with detail log %d ms\n",
		time_without_log, time_with_basic_log, time_with_brief_log, time_with_detail_log);

	ct_log_destroy();
}

int main(void) {
	test_print_performance_comparison();
	ctunit_trace("Finish! test_print_performance_comparison();\n");

	ctunit_pass();
}
