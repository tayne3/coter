/**
 * @file log_callback_test.c
 * @brief 日志回调测试
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

#define TEST_THREADS     2
#define TEST_THREAD_DATA 100000

static FILE*           g_file_with_log      = NULL;
static FILE*           g_file_without_log   = NULL;
static pthread_mutex_t g_file_without_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t g_thread_logger;
static bool      is_exit = false;

// 日志调度线程函数
static inline void* thread_log_schedule(void* arg) {
	for (; !is_exit;) {
		ct_log_schedule(getuptime_ms());
		ct_msleep(10);
	}
	return NULL;
	(void)arg;
}

// 日志回调函数
static inline void log_callback(const char* msg, size_t size, void* userdata) {
	fwrite(msg, 1, size, g_file_with_log);
	return;
	(void)userdata;
}

// 带回调的测试线程函数
static inline void* thread_callback_with_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		test_basic_trace("%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234,
						 1234, 1234, 1234, (void*)0xFFFF0000, (void*)0xFFFF0000, (void*)0xFFFF0000, (void*)0xFFFF0000,
						 "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 直接调用回调函数的测试线程函数
static inline void* thread_callback_without_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		pthread_mutex_lock(&g_file_without_mutex);
		char buffer[1024];
		int  size = snprintf(buffer, sizeof(buffer),
							 "%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234,
							 1234, 1234, 1234, (void*)0xFFFF0000, (void*)0xFFFF0000, (void*)0xFFFF0000,
							 (void*)0xFFFF0000, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
		fwrite(buffer, 1, (size_t)size, g_file_without_log);
		pthread_mutex_unlock(&g_file_without_mutex);
	}
	return NULL;
	(void)arg;
}

// 回调性能对比测试函数
static inline void test_callback_performance_comparison(void) {
	pthread_t   threads[TEST_THREADS];
	ct_time64_t start, end;

	// 检查当前文件夹下是否存在 test_log_out 文件夹
	if (access("test_log_out", 0) == -1) {
		ct_mkdir("test_log_out");
	}
	ctunit_assert_true(access("test_log_out", 0) == 0);

	g_file_without_log = fopen("test_log_out/callback_without_log.log", "w");
	ctunit_assert_not_null(g_file_without_log);

	g_file_with_log = fopen("test_log_out/callback_with_log.log", "w");
	ctunit_assert_not_null(g_file_with_log);

	// 创建 Logger
	{
		ct_log_config_t config = {
			.level             = CTLog_LevelVerbose,
			.disable_print     = true,
			.disable_save      = true,
			.callback_routine  = log_callback,
			.callback_userdata = NULL,
		};
		ct_log_init(getuptime_ms(), 1, &config);
	}

	ctunit_assert_int(pthread_create(&g_thread_logger, NULL, thread_log_schedule, NULL), 0, CTUnit_Equal);

	// 测试直接调用回调函数的性能
	start = getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int(pthread_create(&threads[i], NULL, thread_callback_without_log, NULL), 0, CTUnit_Equal);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int(pthread_join(threads[i], NULL), 0, CTUnit_Equal);
	}
	end                             = getuptime_ms();
	const int time_without_callback = (int)(end - start);

	// 测试带日志回调的性能
	start = getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int(pthread_create(&threads[i], NULL, thread_callback_with_log, NULL), 0, CTUnit_Equal);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		ctunit_assert_int(pthread_join(threads[i], NULL), 0, CTUnit_Equal);
	}
	end                          = getuptime_ms();
	const int time_with_callback = (int)(end - start);

	is_exit = true;
	ctunit_assert_int(pthread_join(g_thread_logger, NULL), 0, CTUnit_Equal);

	ct_log_flush();
	ct_log_schedule(getuptime_ms());

	fclose(g_file_with_log);
	g_file_with_log = NULL;
	fclose(g_file_without_log);
	g_file_without_log = NULL;

	ctunit_trace("Execution time: with log callback %d ms, without log callback %d ms\n", time_with_callback,
				 time_without_callback);

	ct_log_destroy();

	// 打开文件
	FILE* file_with_log    = fopen("test_log_out/callback_with_log.log", "r");
	FILE* file_without_log = fopen("test_log_out/callback_without_log.log", "r");
	ctunit_assert_not_null(file_with_log);
	ctunit_assert_not_null(file_without_log);

	// 获取文件大小
	fseek(file_with_log, 0, SEEK_END);
	fseek(file_without_log, 0, SEEK_END);
	int64_t size_with_log    = ftell(file_with_log);
	int64_t size_without_log = ftell(file_without_log);
	ctunit_assert_int64(size_with_log, 0, CTUnit_Greater);
	ctunit_assert_int64(size_without_log, 0, CTUnit_Greater);
	ctunit_assert_int64(size_with_log, size_without_log, CTUnit_Equal);

	ctunit_trace("with log file size: %d, without log file size: %d\n", size_with_log, size_without_log);

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
		if (bytes_read_with_log == 0 || bytes_read_without_log == 0) {
			break;
		}
		ctunit_assert_int(bytes_read_with_log, bytes_read_without_log, CTUnit_Equal);
		ctunit_assert_string_n(buffer_with_log, buffer_without_log, bytes_read_with_log);
	}

	// 确保两个文件都已读取完毕
	ctunit_assert_true(feof(file_with_log));
	ctunit_assert_true(feof(file_without_log));

	fclose(file_with_log);
	fclose(file_without_log);

	remove("test_log_out/callback_without_log.log");
	remove("test_log_out/callback_with_log.log");
	remove("test_log_out");
}

int main(void) {
	test_callback_performance_comparison();
	ctunit_trace("Finish! test_callback_performance_comparison();\n");

	ctunit_pass();
}
