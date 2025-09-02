/**
 * @file log_write_test.c
 * @brief 日志写入测试
 */
#include "coter/base/platform.h"
#include "cunit.h"
#include "coter/mech/log.h"

#define test_basic_verbose(...) CTLogger_HandleBasic(Verbose, 0, __VA_ARGS__)
#define test_basic_debug(...)   CTLogger_HandleBasic(Debug, 0, __VA_ARGS__)
#define test_basic_trace(...)   CTLogger_HandleBasic(Trace, 0, __VA_ARGS__)
#define test_basic_warning(...) CTLogger_HandleBasic(Warning, 0, __VA_ARGS__)
#define test_basic_error(...)   CTLogger_HandleBasic(Error, 0, __VA_ARGS__)
#define test_basic_fatal(...)   CTLogger_HandleBasic(Fatal, 0, __VA_ARGS__)

#define TEST_THREADS     2
#define TEST_THREAD_DATA 100000

static FILE*     g_file = NULL;
static pthread_t g_thread_logger;
static bool      is_exit = false;

// 日志调度线程函数
static inline void* thread_log_schedule(void* arg) {
	for (; !is_exit;) {
		ct_log_schedule(ct_getuptime_ms());
		ct_msleep(10);
	}
	return NULL;
	(void)arg;
}

// 带日志的测试线程函数
static inline void* thread_write_with_log(void* arg) {
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		test_basic_trace("%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234,
						 1234, 1234, 1234, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL,
						 (void*)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 不带日志的测试线程函数
static inline void* thread_write_without_log(void* arg) {
	assert_not_null(g_file);
	for (int i = 0; i < TEST_THREAD_DATA; i++) {
		fprintf(g_file, "%04d/%05d/%06d/%07d %16p/%16p/%16p/%16p %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234,
				1234, 1234, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL, (void*)0xFFFF0000ULL,
				"test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return NULL;
	(void)arg;
}

// 写入性能对比测试函数
static inline void test_write_performance_comparison(void) {
	pthread_t   threads[TEST_THREADS] = {0};
	ct_time64_t start, end;

	// 检查当前文件夹下是否存在 test_log_out 文件夹
	if (access("test_log_out", 0) == -1) {
		(void)ct_mkdir("test_log_out");
	}
	assert_true(access("test_log_out", 0) == 0);

	g_file = fopen("test_log_out/without_log.log", "w");
	assert_not_null(g_file);

	remove("test_log_out/with_log.log0");

	// 创建 Logger
	{
		ct_log_config_t config = {
			.level         = CTLog_LevelVerbose,
			.disable_print = true,

			.disable_save      = false,
			.file_dir          = "test_log_out",
			.file_name         = "with_log",
			.file_cache_size   = 10 * 1024,
			.file_size_max     = 1024 * 1024 * 1024,
			.file_count_max    = 1,
			.autosave_interval = 3600,

			.callback_routine  = NULL,
			.callback_userdata = NULL,
		};
		ct_log_init(ct_getuptime_ms(), 1, &config);
	}

	assert_int32_eq(pthread_create(&g_thread_logger, NULL, thread_log_schedule, NULL), 0);

	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		assert_int32_eq(pthread_create(&threads[i], NULL, thread_write_without_log, (void*)(uintptr_t)i), 0);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		assert_int32_eq(pthread_join(threads[i], NULL), 0);
	}
	end                        = ct_getuptime_ms();
	const int time_without_log = (int)(end - start);

	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		assert_int32_eq(pthread_create(&threads[i], NULL, thread_write_with_log, (void*)(uintptr_t)i), 0);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		assert_int32_eq(pthread_join(threads[i], NULL), 0);
	}
	end                     = ct_getuptime_ms();
	const int time_with_log = (int)(end - start);

	is_exit = true;
	assert_int32_eq(pthread_join(g_thread_logger, NULL), 0);

	ct_log_flush();
	ct_log_schedule(ct_getuptime_ms());

	cunit_println("Execution time: with log %d ms, without log %d ms\n", time_with_log, time_without_log);

	ct_log_destroy();

	fclose(g_file);
	g_file = NULL;

	// 打开文件
	FILE* file_with_log    = fopen("test_log_out/with_log.log0", "r");
	FILE* file_without_log = fopen("test_log_out/without_log.log", "r");
	assert_not_null(file_with_log);
	assert_not_null(file_without_log);

	// 获取文件大小
	fseek(file_with_log, 0, SEEK_END);
	fseek(file_without_log, 0, SEEK_END);
	const int64_t size_with_log    = ftell(file_with_log);
	const int64_t size_without_log = ftell(file_without_log);
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
		if (bytes_read_with_log == 0 || bytes_read_without_log == 0) {
			break;
		}
		assert_int32_eq(bytes_read_with_log, bytes_read_without_log);
		assert_str_n(buffer_with_log, buffer_without_log, bytes_read_with_log);
	}

	// 确保两个文件都已读取完毕
	assert_true(feof(file_with_log));
	assert_true(feof(file_without_log));

	fclose(file_with_log);
	fclose(file_without_log);

	remove("test_log_out/with_log.log0");
	remove("test_log_out/without_log.log");
	(void)rmdir("test_log_out");
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("log_write", NULL, NULL)
	CUNIT_TEST("performance_comparison", test_write_performance_comparison)
	CUNIT_SUITE_END()

	return cunit_run();
}
