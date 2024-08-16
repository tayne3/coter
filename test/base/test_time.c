/**
 * @file test_time.c
 * @brief 时间相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "base/ct_time.h"
#include "ctunit.h"

static inline int  change_system_time(time_t new_time);
static inline void test_gettick_ms(void);

int main(void) {
	test_gettick_ms();
	ctunit_trace("Finish! test_gettick_ms();\n");

	ctunit_pass();
}

// 修改系统时间(需要管理员权限)
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

static inline void test_gettick_ms(void) {
	// Test 1: Verify timestamp increment
	ct_time64_t tick1 = gettick_ms();
	ct_time64_t tick2 = gettick_ms();
	ctunit_assert_uint64(tick2, tick1, CTUnit_GreaterEqual);
	ctunit_trace("Test 1 passed: Timestamp increment\n");

	// Test 2: Verify millisecond precision
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
	ctunit_trace("Test 2 passed: Millisecond precision verification\n");

	// Test 3: Verify unaffected by system time
	ctunit_trace("Warning: About to change system time, please ensure you have administrator privileges\n");
	ctunit_trace("Press Enter to continue...");
	getchar();

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
	ctunit_trace("Test 3 passed: Timestamp unaffected by system time\n");
}
