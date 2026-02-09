/**
 * @file log_callback_test.c
 * @brief 日志回调测试
 */
#include "coter/core/platform.h"
#include "coter/log/log.h"
#include "cunit.h"

#define test_basic_verbose(...) CTLogger_HandleBasic(Verbose, 0, __VA_ARGS__)
#define test_basic_debug(...)   CTLogger_HandleBasic(Debug, 0, __VA_ARGS__)
#define test_basic_trace(...)   CTLogger_HandleBasic(Trace, 0, __VA_ARGS__)
#define test_basic_warning(...) CTLogger_HandleBasic(Warning, 0, __VA_ARGS__)
#define test_basic_error(...)   CTLogger_HandleBasic(Error, 0, __VA_ARGS__)
#define test_basic_fatal(...)   CTLogger_HandleBasic(Fatal, 0, __VA_ARGS__)

#define TEST_THREADS     4
#define TEST_THREAD_DATA 50000

static FILE*           g_file_with_log      = NULL;
static FILE*           g_file_without_log   = NULL;
static pthread_mutex_t g_file_without_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t g_thread_logger;
static bool      is_exit = false;

// 日志调度线程函数
static void* thread_log_schedule(void* arg) {
	for (; !is_exit;) {
		ct_log_schedule(ct_getuptime_ms());
		ct_msleep(10);
	}
	return NULL;
	(void)arg;
}

// 日志回调函数
static void log_callback(const char* msg, size_t size, void* userdata) {
	assert_not_null(msg);
	assert_not_null(g_file_with_log);
	fwrite(msg, 1, size, g_file_with_log);
	return;
	(void)userdata;
}

// 带回调的测试线程函数
static void* thread_callback_with_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; ++i) {
		test_basic_trace("%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234, 1234, 1234, (void*)0xFFFF0000ULL,
						 (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 直接调用回调函数的测试线程函数
static void* thread_callback_without_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; ++i) {
		pthread_mutex_lock(&g_file_without_mutex);
		char buffer[1024];
		int size = snprintf(buffer, sizeof(buffer), "%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234, 1234, 1234,
							(void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00,
							0x01, 0x02, 0x03);
		assert_not_null(g_file_without_log);
		fwrite(buffer, 1, (size_t)size, g_file_without_log);
		pthread_mutex_unlock(&g_file_without_mutex);
	}
	return NULL;
	(void)arg;
}

// 回调性能对比测试函数
static void test_callback_performance_comparison(size_t limit) {
	pthread_t   threads[TEST_THREADS] = {0};
	ct_time64_t start, end;

	// 检查当前文件夹下是否存在 test_log_out 文件夹
	if (access("test_log_out", 0) == -1) { (void)ct_mkdir("test_log_out"); }
	assert_true(access("test_log_out", 0) == 0);

	g_file_without_log = fopen("test_log_out/callback_without_log.log", "w");
	assert_not_null(g_file_without_log);

	g_file_with_log = fopen("test_log_out/callback_with_log.log", "w");
	assert_not_null(g_file_with_log);

	// 创建 Logger
	{
		ct_log_config_t config = {
			.level             = CTLog_LevelVerbose,
			.disable_print     = true,
			.disable_save      = true,
			.callback_routine  = log_callback,
			.callback_userdata = NULL,
			.callback_limit    = limit,
		};
		ct_log_init(ct_getuptime_ms(), 1, &config);
	}

	assert_int32_eq(pthread_create(&g_thread_logger, NULL, thread_log_schedule, NULL), 0);

	// 测试直接调用回调函数的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; ++i) { assert_int32_eq(pthread_create(&threads[i], NULL, thread_callback_without_log, NULL), 0); }
	for (int i = 0; i < TEST_THREADS; ++i) { assert_int32_eq(pthread_join(threads[i], NULL), 0); }
	end                             = ct_getuptime_ms();
	const int time_without_callback = (int)(end - start);

	// 测试带日志回调的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; ++i) { assert_int32_eq(pthread_create(&threads[i], NULL, thread_callback_with_log, NULL), 0); }
	for (int i = 0; i < TEST_THREADS; ++i) { assert_int32_eq(pthread_join(threads[i], NULL), 0); }
	end                          = ct_getuptime_ms();
	const int time_with_callback = (int)(end - start);

	is_exit = true;
	assert_int32_eq(pthread_join(g_thread_logger, NULL), 0);

	ct_log_flush();
	ct_log_schedule(ct_getuptime_ms());

	fclose(g_file_with_log);
	g_file_with_log = NULL;
	fclose(g_file_without_log);
	g_file_without_log = NULL;

	cunit_println("Execution time: with log callback %d ms, without log callback %d ms\n", time_with_callback, time_without_callback);

	ct_log_destroy();

	// 打开文件
	FILE* file_with_log    = fopen("test_log_out/callback_with_log.log", "r");
	FILE* file_without_log = fopen("test_log_out/callback_without_log.log", "r");
	assert_not_null(file_with_log);
	assert_not_null(file_without_log);

	// 获取文件大小
	fseek(file_with_log, 0, SEEK_END);
	fseek(file_without_log, 0, SEEK_END);
	int64_t size_with_log    = ftell(file_with_log);
	int64_t size_without_log = ftell(file_without_log);
	assert_int64_gt(size_with_log, 0);
	assert_int64_gt(size_without_log, 0);
	assert_int64_eq(size_with_log, size_without_log);

	cunit_println("with log file size: %ld, without log file size: %ld\n", (long)size_with_log, (long)size_without_log);

	// 将文件指针重置到文件开头
	rewind(file_with_log);
	rewind(file_without_log);

	char buffer_with_log[128];
	char buffer_without_log[128];

	size_t bytes_read_with_log, bytes_read_without_log;

	// 比较文件内容是否一致
	while (1) {
		bytes_read_with_log    = fread(buffer_with_log, 1, sizeof(buffer_with_log), file_with_log);
		bytes_read_without_log = fread(buffer_without_log, 1, sizeof(buffer_without_log), file_without_log);
		if (bytes_read_with_log == 0 || bytes_read_without_log == 0) { break; }
		assert_int32_eq(bytes_read_with_log, bytes_read_without_log);
		assert_str_n(buffer_with_log, buffer_without_log, bytes_read_with_log);
	}

	// 确保两个文件都已读取完毕
	assert_true(feof(file_with_log));
	assert_true(feof(file_without_log));

	fclose(file_with_log);
	fclose(file_without_log);

	remove("test_log_out/callback_without_log.log");
	remove("test_log_out/callback_with_log.log");
	(void)rmdir("test_log_out");
}

void test_callback_performance_comparison_0(void) {
	test_callback_performance_comparison(0);
}
void test_callback_performance_comparison_11(void) {
	test_callback_performance_comparison(11);
}
void test_callback_performance_comparison_999(void) {
	test_callback_performance_comparison(999);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("log_callback", NULL, NULL)
	CUNIT_TEST("comparison_0", test_callback_performance_comparison_0)
	CUNIT_TEST("comparison_11", test_callback_performance_comparison_11)
	CUNIT_TEST("comparison_999", test_callback_performance_comparison_999)
	CUNIT_SUITE_END()

	return cunit_run();
}
