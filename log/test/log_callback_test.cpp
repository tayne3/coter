#include <catch.hpp>
#include <cstring>

#include "coter/core/platform.h"
#include "coter/log/log.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"

#define test_basic_verbose(...) CTLogger_HandleBasic(Verbose, 0, __VA_ARGS__)
#define test_basic_debug(...)   CTLogger_HandleBasic(Debug, 0, __VA_ARGS__)
#define test_basic_trace(...)   CTLogger_HandleBasic(Trace, 0, __VA_ARGS__)
#define test_basic_warning(...) CTLogger_HandleBasic(Warning, 0, __VA_ARGS__)
#define test_basic_error(...)   CTLogger_HandleBasic(Error, 0, __VA_ARGS__)
#define test_basic_fatal(...)   CTLogger_HandleBasic(Fatal, 0, __VA_ARGS__)

#define TEST_THREADS     4
#define TEST_THREAD_DATA 50000

namespace {
struct mutex {
	mutex() { ct_mutex_init(&d); }
	~mutex() { ct_mutex_destroy(&d); }

	void lock() { ct_mutex_lock(&d); }
	void unlock() { ct_mutex_unlock(&d); }
	bool try_lock() { return ct_mutex_trylock(&d); }

private:
	ct_mutex_t d;
};
}  // namespace

static FILE* g_file_with_log    = nullptr;
static FILE* g_file_without_log = nullptr;
static mutex g_file_without_mutex;

static ct_thread_t g_thread_logger;
static bool      is_exit = false;

// 日志调度线程函数
static int thread_log_schedule(void* arg) {
	CT_UNUSED(arg);
	for (; !is_exit;) {
		ct_log_schedule(ct_getuptime_ms());
		ct_msleep(10);
	}
	return 0;
}

// 日志回调函数
static void log_callback(const char* msg, size_t size, void* userdata) {
	(void)userdata;
	REQUIRE(msg != nullptr);
	REQUIRE(g_file_with_log != nullptr);
	fwrite(msg, 1, size, g_file_with_log);
}

// 带回调的测试线程函数
static int thread_callback_with_log(void* arg) {
	CT_UNUSED(arg);
	for (int i = 0; i < TEST_THREAD_DATA; ++i) {
		test_basic_trace("%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234, 1234, 1234,
						 (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL,
						 (unsigned long long)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
	}
	return 0;
}

// 直接调用回调函数的测试线程函数
static int thread_callback_without_log(void* arg) {
	CT_UNUSED(arg);
	for (int i = 0; i < TEST_THREAD_DATA; ++i) {
		g_file_without_mutex.lock();
		char buffer[1024];
		int size = snprintf(buffer, sizeof(buffer), "%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234,
							1234, 1234, (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL,
							(unsigned long long)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
		REQUIRE(g_file_without_log != nullptr);
		fwrite(buffer, 1, (size_t)size, g_file_without_log);
		g_file_without_mutex.unlock();
	}
	return 0;
}

// 回调性能对比测试函数
static void test_callback_performance_comparison(size_t limit) {
	ct_thread_t threads[TEST_THREADS] = {};
	ct_time64_t start, end;

	// 检查当前文件夹下是否存在 test_log_out 文件夹
	if (ct_access("test_log_out", 0) == -1) { (void)ct_mkdir("test_log_out"); }
	REQUIRE(ct_access("test_log_out", 0) == 0);

	g_file_without_log = fopen("test_log_out/callback_without_log.log", "w");
	REQUIRE(g_file_without_log != nullptr);

	g_file_with_log = fopen("test_log_out/callback_with_log.log", "w");
	REQUIRE(g_file_with_log != nullptr);

	// 创建 Logger
	{
		ct_log_config_t config;
		memset(&config, 0, sizeof(config));
		config.level             = CTLog_LevelVerbose;
		config.disable_print     = true;
		config.disable_save      = true;
		config.callback_routine  = log_callback;
		config.callback_userdata = nullptr;
		config.callback_limit    = limit;

		ct_log_init(ct_getuptime_ms(), 1, &config);
	}

	REQUIRE(ct_thread_create(&g_thread_logger, nullptr, thread_log_schedule, nullptr) == 0);

	// 测试直接调用回调函数的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; ++i) { REQUIRE(ct_thread_create(&threads[i], nullptr, thread_callback_without_log, nullptr) == 0); }
	for (int i = 0; i < TEST_THREADS; ++i) { REQUIRE(ct_thread_join(threads[i], nullptr) == 0); }
	end                             = ct_getuptime_ms();
	const int time_without_callback = (int)(end - start);

	// 测试带日志回调的性能
	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; ++i) { REQUIRE(ct_thread_create(&threads[i], nullptr, thread_callback_with_log, nullptr) == 0); }
	for (int i = 0; i < TEST_THREADS; ++i) { REQUIRE(ct_thread_join(threads[i], nullptr) == 0); }
	end                          = ct_getuptime_ms();
	const int time_with_callback = (int)(end - start);

	is_exit = true;
	REQUIRE(ct_thread_join(g_thread_logger, nullptr) == 0);

	ct_log_flush();
	ct_log_schedule(ct_getuptime_ms());

	fclose(g_file_with_log);
	g_file_with_log = nullptr;
	fclose(g_file_without_log);
	g_file_without_log = nullptr;

	printf("Execution time: with log callback %d ms, without log callback %d ms\n", time_with_callback, time_without_callback);

	ct_log_destroy();

	// 打开文件
	FILE* file_with_log    = fopen("test_log_out/callback_with_log.log", "r");
	FILE* file_without_log = fopen("test_log_out/callback_without_log.log", "r");
	REQUIRE(file_with_log != nullptr);
	REQUIRE(file_without_log != nullptr);

	// 获取文件大小
	fseek(file_with_log, 0, SEEK_END);
	fseek(file_without_log, 0, SEEK_END);
	int64_t size_with_log    = ftell(file_with_log);
	int64_t size_without_log = ftell(file_without_log);
	REQUIRE(size_with_log > 0);
	REQUIRE(size_without_log > 0);
	REQUIRE(size_with_log == size_without_log);

	printf("with log file size: %ld, without log file size: %ld\n", (long)size_with_log, (long)size_without_log);

	// 将文件指针重置到文件开头
	rewind(file_with_log);
	rewind(file_without_log);

	// 比较文件内容是否一致
	while (1) {
		char         buffer_with_log[128]    = {0};
		char         buffer_without_log[128] = {0};
		const size_t bytes_read_with_log     = fread(buffer_with_log, 1, sizeof(buffer_with_log), file_with_log);
		const size_t bytes_read_without_log  = fread(buffer_without_log, 1, sizeof(buffer_without_log), file_without_log);
		if (bytes_read_with_log == 0 || bytes_read_without_log == 0) { break; }
		REQUIRE(bytes_read_with_log == bytes_read_without_log);
		REQUIRE(std::memcmp(buffer_with_log, buffer_without_log, bytes_read_with_log) == 0);
	}

	// 确保两个文件都已读取完毕
	REQUIRE(feof(file_with_log));
	REQUIRE(feof(file_without_log));

	fclose(file_with_log);
	fclose(file_without_log);

	remove("test_log_out/callback_without_log.log");
	remove("test_log_out/callback_with_log.log");
	(void)ct_rmdir("test_log_out");
}

TEST_CASE("log_callback", "[log]") {
	SECTION("comparison_0") {
		test_callback_performance_comparison(0);
	}
	SECTION("comparison_11") {
		test_callback_performance_comparison(11);
	}
	SECTION("comparison_999") {
		test_callback_performance_comparison(999);
	}
}
