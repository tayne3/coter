/**
 * @file log_hex_test.c
 * @brief 日志十六进制测试
 */
#include "coter/core/platform.h"
#include "coter/log/log.h"
#include "cunit.h"

#define logVN(...) CTLogger_HandleBasic(Verbose, 0, __VA_ARGS__)
#define logDN(...) CTLogger_HandleBasic(Debug, 0, __VA_ARGS__)
#define logTN(...) CTLogger_HandleBasic(Trace, 0, __VA_ARGS__)
#define logWN(...) CTLogger_HandleBasic(Warning, 0, __VA_ARGS__)
#define logEN(...) CTLogger_HandleBasic(Error, 0, __VA_ARGS__)
#define logFN(...) CTLogger_HandleBasic(Fatal, 0, __VA_ARGS__)

#define logVH(__buf, __len) CTLogger_HandleHex(Verbose, 0, __buf, __len)
#define logDH(__buf, __len) CTLogger_HandleHex(Debug, 0, __buf, __len)
#define logTH(__buf, __len) CTLogger_HandleHex(Trace, 0, __buf, __len)
#define logWH(__buf, __len) CTLogger_HandleHex(Warning, 0, __buf, __len)
#define logEH(__buf, __len) CTLogger_HandleHex(Error, 0, __buf, __len)
#define logFH(__buf, __len) CTLogger_HandleHex(Fatal, 0, __buf, __len)

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

static void test_log_hex(void) {
	ct_time64_t start, end;

	// 检查当前文件夹下是否存在 test_log_out 文件夹
	if (access("test_log_out", 0) == -1) { ct_mkdir("test_log_out"); }
	assert_true(access("test_log_out", 0) == 0);

	FILE* file_without_log = fopen("test_log_out/without_log.log", "w");
	assert_not_null(file_without_log);

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

	// 写入日志
	const uint8_t buf[16] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0, 0x01};

	// 测试带日志回调的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < 10000; ++i) {
		logVH(buf, 16);
		logVN("\n");
		logDH(buf, 16);
		logDN("\n");
		logTH(buf, 16);
		logTN("\n");
		logWH(buf, 16);
		logWN("\n");
		logEH(buf, 16);
		logEN("\n");
		logFH(buf, 16);
		logFN("\n");
	}
	end                     = ct_getuptime_ms();
	const int time_with_log = (int)(end - start);

	// 测试不带日志回调的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < 60000; ++i) {
		fprintf(file_without_log,
				"%02X %02X %02X %02X %02X %02X %02X %02X "
				"%02X %02X %02X %02X %02X %02X %02X %02X\n",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
	}
	end                        = ct_getuptime_ms();
	const int time_without_log = (int)(end - start);

	is_exit = true;
	assert_int32_eq(pthread_join(g_thread_logger, NULL), 0);

	fclose(file_without_log);
	file_without_log = NULL;

	cunit_println("Execution time: with log %d ms, without log %d ms\n", time_with_log, time_without_log);

	ct_log_destroy();

	// 打开文件
	FILE* file_log_hex = fopen("test_log_out/with_log.log0", "r");
	file_without_log   = fopen("test_log_out/without_log.log", "r");
	assert_not_null(file_log_hex);
	assert_not_null(file_without_log);

	// 获取文件大小
	fseek(file_log_hex, 0, SEEK_END);
	fseek(file_without_log, 0, SEEK_END);
	int64_t size_log_hex     = ftell(file_log_hex);
	int64_t size_without_log = ftell(file_without_log);
	assert_int64_gt(size_log_hex, 0);
	assert_int64_gt(size_without_log, 0);
	assert_int64_eq(size_log_hex, size_without_log);

	// 将文件指针重置到文件开头
	rewind(file_log_hex);
	rewind(file_without_log);

	char buffer_log_hex[128];
	char buffer_without_log[128];

	size_t bytes_read_log_hex, bytes_read_without_log;

	// 比较文件内容是否一致
	while (1) {
		bytes_read_log_hex     = fread(buffer_log_hex, 1, sizeof(buffer_log_hex), file_log_hex);
		bytes_read_without_log = fread(buffer_without_log, 1, sizeof(buffer_without_log), file_without_log);
		if (bytes_read_log_hex == 0 || bytes_read_without_log == 0) { break; }
		assert_int32_eq(bytes_read_log_hex, bytes_read_without_log);
		assert_str_n(buffer_log_hex, buffer_without_log, bytes_read_log_hex);
	}

	// 确保两个文件都已读取完毕
	assert_true(feof(file_log_hex));
	assert_true(feof(file_without_log));

	fclose(file_log_hex);
	fclose(file_without_log);

	remove("test_log_out/with_log.log0");
	remove("test_log_out/without_log.log");
	rmdir("test_log_out");
}

static void test_log_long_text(const size_t text_size) {
	ct_time64_t start, end;

	// 检查当前文件夹下是否存在 test_log_out 文件夹
	if (access("test_log_out", 0) == -1) { ct_mkdir("test_log_out"); }
	assert_true(access("test_log_out", 0) == 0);

	FILE* file_without_log = fopen("test_log_out/without_log.log", "w");
	assert_not_null(file_without_log);

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

	// 创建日志线程
	is_exit = false;
	assert_int32_eq(pthread_create(&g_thread_logger, NULL, thread_log_schedule, NULL), 0);

	// 创建超长文本
	uint8_t* long_text = (uint8_t*)malloc(text_size + 1);
	for (size_t i = 0; i < text_size; ++i) { long_text[i] = i % 0xFE + 1; }
	long_text[text_size] = '\0';

	// 测试带日志回调的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < 10; ++i) {
		logVH(long_text, text_size);
		logVN("\n");
		logDH(long_text, text_size);
		logDN("\n");
		logTH(long_text, text_size);
		logTN("\n");
		logWH(long_text, text_size);
		logWN("\n");
		logEH(long_text, text_size);
		logEN("\n");
		logFH(long_text, text_size);
		logFN("\n");
	}
	end                     = ct_getuptime_ms();
	const int time_with_log = (int)(end - start);

	// 测试不带日志回调的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < 60; ++i) {
		for (size_t j = 0; j < text_size; ++j) {
			if (j != text_size - 1) {
				fprintf(file_without_log, "%02X ", long_text[j]);
			} else {
				fprintf(file_without_log, "%02X\n", long_text[j]);
			}
		}
	}
	end                        = ct_getuptime_ms();
	const int time_without_log = (int)(end - start);

	// 清理
	is_exit = true;
	assert_int32_eq(pthread_join(g_thread_logger, NULL), 0);

	free(long_text);
	long_text = NULL;
	fclose(file_without_log);
	file_without_log = NULL;

	cunit_println("Execution time: with log %d ms, without log %d ms\n", time_with_log, time_without_log);

	ct_log_destroy();

	// 打开文件
	FILE* file_log_hex = fopen("test_log_out/with_log.log0", "r");
	file_without_log   = fopen("test_log_out/without_log.log", "r");
	assert_not_null(file_log_hex);
	assert_not_null(file_without_log);

	// 获取文件大小
	fseek(file_log_hex, 0, SEEK_END);
	fseek(file_without_log, 0, SEEK_END);
	int64_t size_log_hex     = ftell(file_log_hex);
	int64_t size_without_log = ftell(file_without_log);
	assert_int64_gt(size_log_hex, 0);
	assert_int64_gt(size_without_log, 0);
	assert_int64_eq(size_log_hex, size_without_log);

	// 将文件指针重置到文件开头
	rewind(file_log_hex);
	rewind(file_without_log);

	char buffer_log_hex[128];
	char buffer_without_log[128];

	size_t bytes_read_log_hex, bytes_read_without_log;

	// 比较文件内容是否一致
	while (1) {
		bytes_read_log_hex     = fread(buffer_log_hex, 1, sizeof(buffer_log_hex), file_log_hex);
		bytes_read_without_log = fread(buffer_without_log, 1, sizeof(buffer_without_log), file_without_log);
		if (bytes_read_log_hex == 0 || bytes_read_without_log == 0) { break; }
		assert_int32_eq(bytes_read_log_hex, bytes_read_without_log);
		assert_str_n(buffer_log_hex, buffer_without_log, bytes_read_log_hex);
	}

	// 确保两个文件都已读取完毕
	assert_true(feof(file_log_hex));
	assert_true(feof(file_without_log));

	fclose(file_log_hex);
	fclose(file_without_log);

	remove("test_log_out/with_log.log0");
	remove("test_log_out/without_log.log");
	rmdir("test_log_out");
}

void test_log_long_text_512(void) {
	test_log_long_text(512);
}

void test_log_long_text_1024(void) {
	test_log_long_text(1024);
}

void test_log_long_text_2048(void) {
	test_log_long_text(2048);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("log_hex", NULL, NULL)
	CUNIT_TEST("hex", test_log_hex)
	CUNIT_TEST("long_text_512", test_log_long_text_512)
	CUNIT_TEST("long_text_1024", test_log_long_text_1024)
	CUNIT_TEST("long_text_2048", test_log_long_text_2048)
	CUNIT_SUITE_END()

	return cunit_run();
}
