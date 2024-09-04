/**
 * @file test_log.c
 * @brief 日志测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "base/ct_platform.h"
#include "ctunit.h"
#include "mech/ct_log.h"

#define test_basic_verbose(__logger, ...) CTLogger_HandleBasic(VerBose, (__logger), __VA_ARGS__)
#define test_basic_debug(__logger, ...)   CTLogger_HandleBasic(Debug, (__logger), __VA_ARGS__)
#define test_basic_trace(__logger, ...)   CTLogger_HandleBasic(Trace, (__logger), __VA_ARGS__)
#define test_basic_warning(__logger, ...) CTLogger_HandleBasic(Warning, (__logger), __VA_ARGS__)
#define test_basic_error(__logger, ...)   CTLogger_HandleBasic(Error, (__logger), __VA_ARGS__)
#define test_basic_fatal(__logger, ...)   CTLogger_HandleBasic(Fatal, (__logger), __VA_ARGS__)

#define test_brief_verbose(__logger, ...) CTLogger_HandleBrief(VerBose, (__logger), __VA_ARGS__)
#define test_brief_debug(__logger, ...)   CTLogger_HandleBrief(Debug, (__logger), __VA_ARGS__)
#define test_brief_trace(__logger, ...)   CTLogger_HandleBrief(Trace, (__logger), __VA_ARGS__)
#define test_brief_warning(__logger, ...) CTLogger_HandleBrief(Warning, (__logger), __VA_ARGS__)
#define test_brief_error(__logger, ...)   CTLogger_HandleBrief(Error, (__logger), __VA_ARGS__)
#define test_brief_fatal(__logger, ...)   CTLogger_HandleBrief(Fatal, (__logger), __VA_ARGS__)

#define test_detail_verbose(__logger, ...) CTLogger_HandleDetail(VerBose, (__logger), __VA_ARGS__)
#define test_detail_debug(__logger, ...)   CTLogger_HandleDetail(Debug, (__logger), __VA_ARGS__)
#define test_detail_trace(__logger, ...)   CTLogger_HandleDetail(Trace, (__logger), __VA_ARGS__)
#define test_detail_warning(__logger, ...) CTLogger_HandleDetail(Warning, (__logger), __VA_ARGS__)
#define test_detail_error(__logger, ...)   CTLogger_HandleDetail(Error, (__logger), __VA_ARGS__)
#define test_detail_fatal(__logger, ...)   CTLogger_HandleDetail(Fatal, (__logger), __VA_ARGS__)

#define TEST_THREADS     20
#define TEST_THREAD_DATA 100000

static FILE*           g_file_with_log      = NULL;
static FILE*           g_file_without_log   = NULL;
static pthread_mutex_t g_file_without_mutex = PTHREAD_MUTEX_INITIALIZER;

static ct_logger_t* g_logger = NULL;
static pthread_t    g_thread_logger;
static bool         is_exit = false;

// 日志调度线程函数
static inline void* thread_logger_schedule(void* arg);

// 日志回调函数
static void log_callback(const char* log, size_t size, void* userdata);
// 带回调的测试线程函数
static inline void* thread_callback_with_log(void* arg);
// 直接调用回调函数的测试线程函数
static inline void* thread_callback_without_log(void* arg);
// 回调性能对比测试函数
static inline void test_callback_performance_comparison(void);

int main(void) {
	test_callback_performance_comparison();
	ctunit_trace("Finish! test_callback_performance_comparison();\n");

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

// 日志回调函数
static void log_callback(const char* log, size_t size, void* userdata) {
	fwrite(log, 1, size, g_file_with_log);
	return;
	(void)userdata;
}

// 带回调的测试线程函数
static inline void* thread_callback_with_log(void* arg) {
	ctunit_assert_not_null(g_logger);
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		test_basic_trace(g_logger, "%s\n", "abcde-+=0123456789");
	}
	pthread_exit(NULL);
	return NULL;
	(void)arg;
}

// 直接调用回调函数的测试线程函数
static inline void* thread_callback_without_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		pthread_mutex_lock(&g_file_without_mutex);
		char buffer[1024];
		int  size = snprintf(buffer, sizeof(buffer), "%s\n", "abcde-+=0123456789");
		fwrite(buffer, 1, size, g_file_without_log);
		pthread_mutex_unlock(&g_file_without_mutex);
	}
	pthread_exit(NULL);
	return NULL;
	(void)arg;
}

static inline void test_callback_performance_comparison(void) {
	pthread_t   threads[TEST_THREADS];
	ct_time64_t start, end;
	bool        is_ok;

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
			.level             = CTLog_LevelVerBose,
			.disable_print     = true,
			.disable_save      = true,
			.callback_routine  = log_callback,
			.callback_userdata = NULL,
		};
		g_logger = ct_logger_create(&config);
		ctunit_assert_not_null(g_logger);
	}

	is_ok = 0 == pthread_create(&g_thread_logger, NULL, thread_logger_schedule, NULL);
	ctunit_assert_true(is_ok);

	// 测试直接调用回调函数的性能
	start = gettick_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, thread_callback_without_log, NULL);
		ctunit_assert_true(is_ok);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		ctunit_assert_true(is_ok);
	}
	end                             = gettick_ms();
	const int time_without_callback = (int)(end - start);

	// 测试带日志回调的性能
	start = gettick_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, thread_callback_with_log, NULL);
		ctunit_assert_true(is_ok);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		ctunit_assert_true(is_ok);
	}
	end                          = gettick_ms();
	const int time_with_callback = (int)(end - start);

	is_exit = true;
	is_ok   = 0 == pthread_join(g_thread_logger, NULL);
	ctunit_assert_true(is_ok);

	ct_logger_destroy(g_logger);
	g_logger = NULL;

	fclose(g_file_with_log);
	g_file_with_log = NULL;
	fclose(g_file_without_log);
	g_file_without_log = NULL;

	ctunit_trace("Execution time: with log callback %d ms, without log callback %d ms\n", time_with_callback,
				 time_without_callback);

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
}
