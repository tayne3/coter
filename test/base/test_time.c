/**
 * @file test_time.c
 * @brief 时间相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "base/ct_time.h"
#include "ctunit.h"

// 辅助函数: 修改系统时间 (需要管理员权限)
static inline int change_system_time(time_t new_time);
// 测试用例: 验证时间戳递增
static inline void test_timestamp_increment(void);
// 测试用例: 验证毫秒精度
static inline void test_millisecond_precision(void);
// 测试用例: 验证不受系统时间影响
static inline void test_unaffected_by_system_time(void);

int main(int argc, char *argv[]) {
	bool is_quiet = false;
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-q", 2) == 0) {
			is_quiet = true;
			break;
		}
	}

	ctunit_trace("Warning: About to change system time, please ensure you have administrator privileges\n");
	if (!is_quiet) {
		ctunit_trace("Press Enter to continue...");
		getchar();
	}

	test_timestamp_increment();
	ctunit_trace("Finish! test_timestamp_increment();\n");

	test_millisecond_precision();
	ctunit_trace("Finish! test_millisecond_precision();\n");

	test_unaffected_by_system_time();
	ctunit_trace("Finish! test_unaffected_by_system_time();\n");

	ctunit_pass();
}

// 辅助函数: 修改系统时间 (需要管理员权限)
static inline int change_system_time(time_t new_time) {
#ifdef _WIN32
	SYSTEMTIME     st;
	FILETIME       ft;
	ULARGE_INTEGER uli;
	uli.QuadPart      = (new_time + 11644473600LL) * 10000000LL;
	ft.dwLowDateTime  = uli.LowPart;
	ft.dwHighDateTime = uli.HighPart;
	FileTimeToSystemTime(&ft, &st);
	return SetSystemTime(&st) ? 0 : -1;
#elif HAVE_CLOCK_GETTIME
	struct timeval tv;
	tv.tv_sec  = new_time;
	tv.tv_usec = 0;
	return settimeofday(&tv, NULL);
#else
	return -1;
#endif
}

// 测试用例: 验证时间戳递增
static inline void test_timestamp_increment(void) {
	ct_time64_t tick1 = gettick_ms();
	ct_time64_t tick2 = gettick_ms();
	ctunit_assert_uint64(tick2, tick1, CTUnit_GreaterEqual);
}

// 测试用例: 验证毫秒精度
static inline void test_millisecond_precision(void) {
	ct_time64_t start_tick = gettick_ms();
#ifdef _WIN32
	Sleep(100);  // Sleep for 100 milliseconds
#else
	usleep(100000);  // Sleep for 100 milliseconds
#endif
	ct_time64_t end_tick = gettick_ms();
	// Allow some error
	ctunit_assert_uint64(end_tick - start_tick, 80, CTUnit_GreaterEqual);
	ctunit_assert_uint64(end_tick - start_tick, 120, CTUnit_Less);
}

// 测试用例: 验证不受系统时间影响
static inline void test_unaffected_by_system_time(void) {
	int ret;

	ct_time64_t before_tick = gettick_ms();
	time_t      before_time = time(NULL);
	ret                     = change_system_time(before_time + 3600);  // Adjust system time forward by 1 hour
	ctunit_assert_ret(ret, "change system time failed");

	ct_time64_t after_tick = gettick_ms();
	time_t      after_time = time(NULL);
	change_system_time(after_time - 3600);  // Adjust system time back
	ctunit_assert_ret(ret, "change system time failed");

	ctunit_assert_uint64(after_time - before_time, 3600 - 10, CTUnit_GreaterEqual);
	ctunit_assert_uint64(after_time - before_time, 3600 + 10, CTUnit_Less);

	// Allow 1 second error
	ctunit_assert_uint64(after_tick - before_tick, 1000, CTUnit_Less);
}
