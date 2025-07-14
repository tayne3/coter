/**
 * @file cron_next_test.c
 * @brief 定时任务相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "cunit.h"
#include "mech/ct_cron.h"

// 测试用例：每分钟执行
static inline void test_cron_minutely(void) {
	const time_t before = 946656000;       // 初始时间为: 2000-01-01 00:00:00
	const time_t after  = 946656000 + 60;  // 预期下一分钟：2000-01-01 00:01:00
	const time_t next   = ct_cron_next_timeout(before, -1, -1, -1, -1, -1);

	assert_int64_eq(after, next);
}

// 测试用例：每小时执行
static inline void test_cron_hourly(void) {
	const time_t before = 946656000;         // 初始时间为: 2000-01-01 00:00:00
	const time_t after  = 946656000 + 3600;  // 预期下一小时：2000-01-01 01:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, -1, -1, -1, -1);

	assert_int64_eq(after, next);
}

// 测试用例：每天执行
static inline void test_cron_daily(void) {
	const time_t before = 946656000;              // 初始时间为: 2000-01-01 00:00:00
	const time_t after  = 946656000 + 24 * 3600;  // 预期下一天：2000-01-02 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, -1, -1, -1);

	assert_int64_eq(after, next);
}

// 测试用例：每周执行
static inline void test_cron_weekly(void) {
	const time_t before = 946656000;                  // 初始时间为: 2000-01-01 00:00:00 (星期六)
	const time_t after  = 946656000 + 1 * 24 * 3600;  // 预期下一周日：2000-01-02 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, -1, 0, -1);

	assert_int64_eq(after, next);
}

// 测试用例：每月执行
static inline void test_cron_monthly(void) {
	const time_t before = 946656000;                   // 初始时间为: 2000-01-01 00:00:00
	const time_t after  = 946656000 + 31 * 24 * 3600;  // 预期下个月1号：2000-02-01 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);

	assert_int64_eq(after, next);
}

// 测试用例：每年执行
static inline void test_cron_yearly(void) {
	const time_t before = 946656000;                    // 初始时间为: 2000-01-01 00:00:00
	const time_t after  = 946656000 + 366 * 24 * 3600;  // 预期明年1月1日：2001-01-01 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);

	assert_int64_eq(after, next);
}

// 测试用例：无效参数
static inline void test_invalid_params(void) {
	const time_t now = time(NULL);
	assert_int64_eq(-1, ct_cron_next_timeout(now, 60, -1, -1, -1, -1));  // 无效分钟
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, 24, -1, -1, -1));  // 无效小时
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, -1, 32, -1, -1));  // 无效日期
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, -1, -1, 7, -1));   // 无效星期
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, -1, -1, -1, 13));  // 无效月份
}

// 测试用例：跨月份边界
static inline void test_cross_month_boundary(void) {
	const time_t before = 946656000 + 31 * 24 * 3600 - 60;  // 初始时间为: 2000-01-31 23:59:00
	const time_t after  = 946656000 + 31 * 24 * 3600;       // 预期下个月1号：2000-02-01 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);

	assert_int64_eq(after, next);
}

// 测试用例：跨年份边界
static inline void test_cross_year_boundary(void) {
	const time_t before = 978278340;       // 初始时间为: 2000-12-31 23:59:00
	const time_t after  = 978278340 + 60;  // 预期明年1月1日：2001-01-01 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);

	assert_int64_eq(after, next);
}

// 测试用例：闰年2月29日
static inline void test_leap_year(void) {
	const time_t before = 951753540;       // 初始时间为: 2000-02-28 23:59:00
	const time_t after  = 951753540 + 60;  // 预期2000年2月29日: 2000-02-29 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);

	assert_int64_eq(after, next);
}

// 测试用例：非闰年2月29日
static inline void test_non_leap_year(void) {
	const time_t before = 983375940;       // 初始时间为: 2001-02-28 23:59:00
	const time_t after  = 983375940 + 60;  // 预期2001年3月29日: 2001-03-29 00:00:00
	const time_t next   = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);

	assert_int64_eq(after, next);
}

int main(void) {
	test_cron_minutely();
	cunit_println("Finish! test_cron_minutely();");

	test_cron_hourly();
	cunit_println("Finish! test_cron_hourly();");

	test_cron_daily();
	cunit_println("Finish! test_cron_daily();");

	test_cron_weekly();
	cunit_println("Finish! test_cron_weekly();");

	test_cron_monthly();
	cunit_println("Finish! test_cron_monthly();");

	test_cron_yearly();
	cunit_println("Finish! test_cron_yearly();");

	test_invalid_params();
	cunit_println("Finish! test_invalid_params();");

	test_cross_month_boundary();
	cunit_println("Finish! test_cross_month_boundary();");

	test_cross_year_boundary();
	cunit_println("Finish! test_cross_year_boundary();");

	test_leap_year();
	cunit_println("Finish! test_leap_year();");

	test_non_leap_year();
	cunit_println("Finish! test_non_leap_year();");

	cunit_pass();
}
