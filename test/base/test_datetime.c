/**
 * @file test_datetime.c
 * @brief 日期时间相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "base/ct_datetime.h"
#include "ctunit.h"

// 辅助函数: 修改系统时间 (需要管理员权限)
static inline void change_system_datetime(int year, int month, int day, int hour, int minute, int second);
// 辅助函数: 修改系统时间 (需要管理员权限)
static inline int change_system_time(time_t new_time);
// 测试用例：每分钟执行
static inline void test_cron_minutely(void);
// 测试用例：每小时执行
static inline void test_cron_hourly(void);
// 测试用例：每天执行
static inline void test_cron_daily(void);
// 测试用例：每周执行
static inline void test_cron_weekly(void);
// 测试用例：每月执行
static inline void test_cron_monthly(void);
// 测试用例：每年执行
static inline void test_cron_yearly(void);
// 测试用例：无效参数
static inline void test_invalid_params(void);
// 测试用例：跨月份边界
static inline void test_cross_month_boundary(void);
// 测试用例：跨年份边界
static inline void test_cross_year_boundary(void);
// 测试用例：闰年2月29日
static inline void test_leap_year(void);
// 测试用例：非闰年2月29日
static inline void test_non_leap_year(void);

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

	test_cron_minutely();
	ctunit_trace("Finish! test_cron_minutely();\n");

	test_cron_hourly();
	ctunit_trace("Finish! test_cron_hourly();\n");

	test_cron_daily();
	ctunit_trace("Finish! test_cron_daily();\n");

	test_cron_weekly();
	ctunit_trace("Finish! test_cron_weekly();\n");

	test_cron_monthly();
	ctunit_trace("Finish! test_cron_monthly();\n");

	test_cron_yearly();
	ctunit_trace("Finish! test_cron_yearly();\n");

	test_invalid_params();
	ctunit_trace("Finish! test_invalid_params();\n");

	test_cross_month_boundary();
	ctunit_trace("Finish! test_cross_month_boundary();\n");

	test_cross_year_boundary();
	ctunit_trace("Finish! test_cross_year_boundary();\n");

	test_leap_year();
	ctunit_trace("Finish! test_leap_year();\n");

	test_non_leap_year();
	ctunit_trace("Finish! test_non_leap_year();\n");

	ctunit_pass();
}

// 辅助函数: 修改系统时间 (需要管理员权限)
static inline void change_system_datetime(int year, int month, int day, int hour, int minute, int second) {
	struct tm tm        = {0};
	tm.tm_year          = year - 1900;
	tm.tm_mon           = month - 1;
	tm.tm_mday          = day;
	tm.tm_hour          = hour;
	tm.tm_min           = minute;
	tm.tm_sec           = second;
	time_t current_time = mktime(&tm);
	// 修改系统时间
	change_system_time(current_time);
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

// 测试用例：每分钟执行
static inline void test_cron_minutely(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 1, 1, 0, 0, 0);  // 修改系统时间为: 2000-01-01 00:00:00

	const time_t after = time(NULL);
	ctunit_check_uint64(946656000, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(-1, -1, -1, -1, -1);
	change_system_time(now);

	ctunit_assert_int64(946656000 + 60, next, CTUnit_Equal);  // 预期下一分钟：2000-01-01 00:01:00
}

// 测试用例：每小时执行
static inline void test_cron_hourly(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 1, 1, 0, 0, 0);  // 修改系统时间为: 2000-01-01 00:00:00

	const time_t after = time(NULL);
	ctunit_check_uint64(946656000, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, -1, -1, -1, -1);
	change_system_time(now);

	ctunit_assert_int64(946656000 + 3600, next, CTUnit_Equal);  // 预期下一小时：2000-01-01 01:00:00
}

// 测试用例：每天执行
static inline void test_cron_daily(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 1, 1, 0, 0, 0);  // 修改系统时间为: 2000-01-01 00:00:00
	const time_t after = time(NULL);
	ctunit_check_uint64(946656000, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, -1, -1, -1);
	change_system_time(now);
	ctunit_assert_int64(946656000 + 24 * 3600, next, CTUnit_Equal);  // 预期下一天：2000-01-02 00:00:00
}

// 测试用例：每周执行
static inline void test_cron_weekly(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 1, 1, 0, 0, 0);  // 修改系统时间为: 2000-01-01 00:00:00 (星期六)
	const time_t after = time(NULL);
	ctunit_check_uint64(946656000, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, -1, 0, -1);
	change_system_time(now);
	ctunit_assert_int64(946656000 + 1 * 24 * 3600, next, CTUnit_Equal);  // 预期下一周日：2000-01-02 00:00:00
}

// 测试用例：每月执行
static inline void test_cron_monthly(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 1, 1, 0, 0, 0);  // 修改系统时间为: 2000-01-01 00:00:00
	const time_t after = time(NULL);
	ctunit_check_uint64(946656000, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, 1, -1, -1);
	change_system_time(now);
	ctunit_assert_int64(946656000 + 31 * 24 * 3600, next, CTUnit_Equal);  // 预期下个月1号：2000-02-01 00:00:00
}

// 测试用例：每年执行
static inline void test_cron_yearly(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 1, 1, 0, 0, 0);  // 修改系统时间为: 2000-01-01 00:00:00
	const time_t after = time(NULL);
	ctunit_check_uint64(946656000, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, 1, -1, 1);
	change_system_time(now);
	ctunit_assert_int64(946656000 + 366 * 24 * 3600, next, CTUnit_Equal);  // 预期明年1月1日：2001-01-01 00:00:00
}

// 测试用例：无效参数
static inline void test_invalid_params(void) {
	ctunit_assert_int64(-1, ct_datetime_cron_next_timeout(60, -1, -1, -1, -1), CTUnit_Equal);  // 无效分钟
	ctunit_assert_int64(-1, ct_datetime_cron_next_timeout(-1, 24, -1, -1, -1), CTUnit_Equal);  // 无效小时
	ctunit_assert_int64(-1, ct_datetime_cron_next_timeout(-1, -1, 32, -1, -1), CTUnit_Equal);  // 无效日期
	ctunit_assert_int64(-1, ct_datetime_cron_next_timeout(-1, -1, -1, 7, -1), CTUnit_Equal);   // 无效星期
	ctunit_assert_int64(-1, ct_datetime_cron_next_timeout(-1, -1, -1, -1, 13), CTUnit_Equal);  // 无效月份
}

// 测试用例：跨月份边界
static inline void test_cross_month_boundary(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 1, 31, 23, 59, 0);  // 修改系统时间为: 2000-01-31 23:59:00
	const time_t after = time(NULL);
	ctunit_check_uint64(946656000 + 31 * 24 * 3600 - 60, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, 1, -1, -1);
	change_system_time(now);
	ctunit_assert_int64(946656000 + 31 * 24 * 3600, next, CTUnit_Equal);  // 预期下个月1号：2000-02-01 00:00:00
}

// 测试用例：跨年份边界
static inline void test_cross_year_boundary(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 12, 31, 23, 59, 0);  // 修改系统时间为: 2000-12-31 23:59:00
	const time_t after = time(NULL);
	ctunit_check_uint64(978278340, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, 1, -1, 1);
	change_system_time(now);
	ctunit_assert_int64(978278340 + 60, next, CTUnit_Equal);  // 预期明年1月1日：2001-01-01 00:00:00
}

// 测试用例：闰年2月29日
static inline void test_leap_year(void) {
	const time_t now = time(NULL);
	change_system_datetime(2000, 2, 28, 23, 59, 0);  // 修改系统时间为: 2000-02-28 23:59:00
	const time_t after = time(NULL);
	ctunit_check_uint64(951753540, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, 29, -1, -1);
	change_system_time(now);
	ctunit_assert_int64(951753540 + 60, next, CTUnit_Equal);  // 预期2000年2月29日: 2000-02-29 00:00:00
}

// 测试用例：非闰年2月29日
static inline void test_non_leap_year(void) {
	const time_t now = time(NULL);
	change_system_datetime(2001, 2, 28, 23, 59, 0);  // 修改系统时间为: 2001-02-28 23:59:00
	const time_t after = time(NULL);
	ctunit_check_uint64(983375940, after, CTUnit_Equal);  // 检查系统时间是否被修改

	time_t next = ct_datetime_cron_next_timeout(0, 0, 29, -1, -1);
	change_system_time(now);
	ctunit_assert_int64(983375940 + 60, next, CTUnit_Equal);  // 预期2001年3月29日: 2001-03-29 00:00:00
}
