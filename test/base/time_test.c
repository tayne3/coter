/**
 * @file time_test.c
 * @brief 时间相关测试
 * @author tayne3@dingtalk.com
 */
#include "coter/base/time.h"
#include "cunit.h"

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
#else
	struct timeval tv;
	tv.tv_sec  = new_time;
	tv.tv_usec = 0;
	return settimeofday(&tv, NULL);
#endif
}

// 辅助函数: 是否为管理员
static inline int is_admin(void) {
#ifdef _WIN32
	BOOL   bIsElevated = FALSE;
	HANDLE hToken      = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		return -1;
	}

	TOKEN_ELEVATION elevation;
	DWORD           dwSize;
	if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
		return -1;
	}
	bIsElevated = (BOOL)elevation.TokenIsElevated;

	CloseHandle(hToken);
	return bIsElevated ? 0 : 1;
#else
	return getuid() == 0 ? 0 : 1;
#endif
}

// 测试用例: 验证时间戳递增
static inline void test_timestamp_increment(void) {
	ct_time64_t tick1 = ct_getuptime_ms();
	ct_time64_t tick2 = ct_getuptime_ms();
	assert_uint64_ge(tick2, tick1);
}

// 测试用例: 验证毫秒精度
static inline void test_millisecond_precision(void) {
	ct_time64_t start_tick, end_tick;

	start_tick = ct_getuptime_ms();
#ifdef _WIN32
	Sleep(100);  // Sleep for 100 milliseconds
#else
	usleep(100000);  // Sleep for 100 milliseconds
#endif
	end_tick = ct_getuptime_ms();
	// Allow some error
	assert_uint64_ge(end_tick - start_tick, 80);
	assert_uint64_lt(end_tick - start_tick, 120);

	start_tick = ct_getuptime_ms();
#ifdef _WIN32
	Sleep(200);  // Sleep for 200 milliseconds
#else
	usleep(200000);  // Sleep for 200 milliseconds
#endif
	end_tick = ct_getuptime_ms();
	// Allow some error
	assert_uint64_ge(end_tick - start_tick, 160);
	assert_uint64_lt(end_tick - start_tick, 240);
}

// 测试用例: 验证不受系统时间影响
static inline void test_unaffected_by_system_time(void) {
	int ret;

	ct_time64_t before_tick = ct_getuptime_ms();
	time_t      before_time = time(NULL);
	ret                     = change_system_time(before_time + 3600);  // Adjust system time forward by 1 hour
	assert_int_eq(ret, 0, "change system time failed");

	ct_time64_t after_tick = ct_getuptime_ms();
	time_t      after_time = time(NULL);
	change_system_time(after_time - 3600);  // Adjust system time back
	assert_int_eq(ret, 0, "change system time failed");

	assert_uint64_ge(after_time - before_time, 3600 - 10);
	assert_uint64_lt(after_time - before_time, 3600 + 10);

	// Allow 1 second error
	assert_uint64_lt(after_tick - before_tick, 1000);
}

int main(int argc, char *argv[]) {
	bool test_system_time = false;
	bool is_quiet         = false;
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-t", 2) == 0) {
			test_system_time = true;
			continue;
		}
		if (strncmp(argv[i], "-q", 2) == 0) {
			is_quiet = true;
			continue;
		}
	}

	test_timestamp_increment();
	cunit_println("Finish! test_timestamp_increment();");

	test_millisecond_precision();
	cunit_println("Finish! test_millisecond_precision();");

	if (test_system_time && is_admin() == 0) {
		cunit_println("Warning: About to change system time, please ensure you have administrator privileges");
		if (!is_quiet) {
			cunit_println("Press Enter to continue...");
			getchar();
		}
		test_unaffected_by_system_time();
		cunit_println("Finish! test_unaffected_by_system_time();");
	} else {
		cunit_println("Warning: Running without administrator privileges.");
		cunit_println("The following tests requiring elevated permissions will be skipped:");
		cunit_println(" - test_unaffected_by_system_time");
	}

	cunit_pass();
}
