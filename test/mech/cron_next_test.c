/**
 * @file cron_next_test.c
 * @brief 定时任务相关测试
 */
#include "cunit.h"
#include "coter/mech/cron.h"

// 辅助函数
static inline ct_time_t create_test_time(int year, int month, int day, int hour, int min, int sec) {
	struct tm tm = {0};
	tm.tm_year   = year - 1900;
	tm.tm_mon    = month - 1;
	tm.tm_mday   = day;
	tm.tm_hour   = hour;
	tm.tm_min    = min;
	tm.tm_sec    = sec;
	tm.tm_isdst  = -1;
	return mktime(&tm);
}

// 测试用例：每分钟执行
static inline void test_cron_minutely(void) {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);  // 2000-01-01 00:00:00
	const ct_time_t expected = create_test_time(2000, 1, 1, 0, 1, 0);  // 2000-01-01 00:01:00
	const ct_time_t next     = ct_cron_next_timeout(before, -1, -1, -1, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：每小时执行
static inline void test_cron_hourly(void) {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);  // 2000-01-01 00:00:00
	const ct_time_t expected = create_test_time(2000, 1, 1, 1, 0, 0);  // 2000-01-01 01:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, -1, -1, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：每天执行
static inline void test_cron_daily(void) {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);  // 2000-01-01 00:00:00
	const ct_time_t expected = create_test_time(2000, 1, 2, 0, 0, 0);  // 2000-01-02 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：每周执行
static inline void test_cron_weekly(void) {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);  // 2000-01-01 00:00:00 (星期六)
	const ct_time_t expected = create_test_time(2000, 1, 2, 0, 0, 0);  // 2000-01-02 00:00:00 (星期日)
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 0, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：每月执行
static inline void test_cron_monthly(void) {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);  // 2000-01-01 00:00:00
	const ct_time_t expected = create_test_time(2000, 2, 1, 0, 0, 0);  // 2000-02-01 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：每年执行
static inline void test_cron_yearly(void) {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);  // 2000-01-01 00:00:00
	const ct_time_t expected = create_test_time(2001, 1, 1, 0, 0, 0);  // 2001-01-01 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);

	assert_int64_eq(expected, next);
}

// 测试用例：无效参数
static inline void test_invalid_params(void) {
	const ct_time_t now = time(NULL);
	assert_int64_eq(-1, ct_cron_next_timeout(now, 60, -1, -1, -1, -1));  // 无效分钟
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, 24, -1, -1, -1));  // 无效小时
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, -1, 32, -1, -1));  // 无效日期
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, -1, -1, 7, -1));   // 无效星期
	assert_int64_eq(-1, ct_cron_next_timeout(now, -1, -1, -1, -1, 13));  // 无效月份
}

// 测试用例：跨月份边界
static inline void test_cross_month_boundary(void) {
	const ct_time_t before   = create_test_time(2000, 1, 31, 23, 59, 0);  // 2000-01-31 23:59:00
	const ct_time_t expected = create_test_time(2000, 2, 1, 0, 0, 0);     // 2000-02-01 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：跨年份边界
static inline void test_cross_year_boundary(void) {
	const ct_time_t before   = create_test_time(2000, 12, 31, 23, 59, 0);  // 2000-12-31 23:59:00
	const ct_time_t expected = create_test_time(2001, 1, 1, 0, 0, 0);      // 2001-01-01 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);

	assert_int64_eq(expected, next);
}

// 测试用例：闰年2月29日
static inline void test_leap_year(void) {
	const ct_time_t before   = create_test_time(2000, 2, 28, 23, 59, 0);  // 2000-02-28 23:59:00
	const ct_time_t expected = create_test_time(2000, 2, 29, 0, 0, 0);    // 2000-02-29 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：非闰年2月29日
static inline void test_non_leap_year(void) {
	const ct_time_t before   = create_test_time(2001, 2, 28, 23, 59, 0);  // 2001-02-28 23:59:00
	const ct_time_t expected = create_test_time(2001, 3, 29, 0, 0, 0);    // 2001-03-29 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：31日月份跳跃
static inline void test_day_31_skip(void) {
	// 1月31日 -> 3月31日 (跳过2月，因为2月没有31日)
	const ct_time_t before   = create_test_time(2023, 1, 31, 0, 0, 0);  // 2023-01-31 00:00:00
	const ct_time_t expected = create_test_time(2023, 3, 31, 0, 0, 0);  // 2023-03-31 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：30日月份跳跃
static inline void test_day_30_skip(void) {
	// 1月30日 -> 3月30日 (跳过2月，因为2月没有30日)
	const ct_time_t before   = create_test_time(2023, 1, 30, 0, 0, 0);  // 2023-01-30 00:00:00
	const ct_time_t expected = create_test_time(2023, 3, 30, 0, 0, 0);  // 2023-03-30 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 30, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：连续跳过多个月份
static inline void test_multiple_month_skip(void) {
	// 要求31日执行，从4月开始：4月(30天)、6月(30天)、9月(30天)、11月(30天) 都没有31日
	// 4月15日 -> 5月31日
	const ct_time_t before   = create_test_time(2023, 4, 15, 0, 0, 0);  // 2023-04-15 00:00:00
	const ct_time_t expected = create_test_time(2023, 5, 31, 0, 0, 0);  // 2023-05-31 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：星期跨月边界
static inline void test_weekly_cross_month(void) {
	// 2023-01-30是星期一，要求星期三(2)执行
	// 预期跳到2023-02-01 (星期三)
	const ct_time_t before   = create_test_time(2023, 1, 30, 0, 0, 0);  // 2023-01-30 00:00:00 (星期一)
	const ct_time_t expected = create_test_time(2023, 2, 1, 0, 0, 0);   // 2023-02-01 00:00:00 (星期三)
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 3, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：星期跨年边界
static inline void test_weekly_cross_year(void) {
	// 2023-12-30是星期六，要求星期一(1)执行
	// 预期跳到2024-01-01 (星期一)
	const ct_time_t before   = create_test_time(2023, 12, 30, 0, 0, 0);  // 2023-12-30 00:00:00 (星期六)
	const ct_time_t expected = create_test_time(2024, 1, 1, 0, 0, 0);    // 2024-01-01 00:00:00 (星期一)
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：时间已过 - 当日内
static inline void test_time_passed_same_day(void) {
	// 当前时间14:30，要求10:00执行，预期跳到明天10:00
	const ct_time_t before   = create_test_time(2023, 6, 15, 14, 30, 0);  // 2023-06-15 14:30:00
	const ct_time_t expected = create_test_time(2023, 6, 16, 10, 0, 0);   // 2023-06-16 10:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 10, -1, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：时间已过 - 月度任务
static inline void test_time_passed_monthly(void) {
	// 当前时间6月20日，要求每月15日执行，预期跳到7月15日
	const ct_time_t before   = create_test_time(2023, 6, 20, 0, 0, 0);  // 2023-06-20 00:00:00
	const ct_time_t expected = create_test_time(2023, 7, 15, 0, 0, 0);  // 2023-07-15 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 15, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：复杂参数组合 - 特定时间的月度任务
static inline void test_complex_monthly_time(void) {
	// 每月28日14:30执行
	const ct_time_t before   = create_test_time(2023, 3, 15, 10, 0, 0);   // 2023-03-15 10:00:00
	const ct_time_t expected = create_test_time(2023, 3, 28, 14, 30, 0);  // 2023-03-28 14:30:00
	const ct_time_t next     = ct_cron_next_timeout(before, 30, 14, 28, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：复杂参数组合 - 特定时间的年度任务
static inline void test_complex_yearly_time(void) {
	// 每年12月25日09:00执行 (圣诞节)
	const ct_time_t before   = create_test_time(2023, 6, 15, 10, 0, 0);  // 2023-06-15 10:00:00
	const ct_time_t expected = create_test_time(2023, 12, 25, 9, 0, 0);  // 2023-12-25 09:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 9, 25, -1, 12);

	assert_int64_eq(expected, next);
}

// 测试用例：闰年年度任务跳跃
static inline void test_leap_year_yearly_skip(void) {
	// 要求每年2月29日执行，从2023年(非闰年)开始，预期跳到2024年(闰年)
	const ct_time_t before   = create_test_time(2023, 1, 15, 0, 0, 0);  // 2023-01-15 00:00:00
	const ct_time_t expected = create_test_time(2024, 2, 29, 0, 0, 0);  // 2024-02-29 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, 2);

	assert_int64_eq(expected, next);
}

// 测试用例：边界值 - 最大日期
static inline void test_boundary_max_day(void) {
	// 要求31日执行，从12月开始，预期在12月31日执行
	const ct_time_t before   = create_test_time(2023, 12, 15, 0, 0, 0);  // 2023-12-15 00:00:00
	const ct_time_t expected = create_test_time(2023, 12, 31, 0, 0, 0);  // 2023-12-31 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);

	assert_int64_eq(expected, next);
}

// 测试用例：边界值 - 年度任务当年已过
static inline void test_boundary_yearly_passed(void) {
	// 要求每年1月1日执行，但当前已经是6月，预期跳到下一年
	const ct_time_t before   = create_test_time(2023, 6, 15, 0, 0, 0);  // 2023-06-15 00:00:00
	const ct_time_t expected = create_test_time(2024, 1, 1, 0, 0, 0);   // 2024-01-01 00:00:00
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);

	assert_int64_eq(expected, next);
}

// 测试用例：星期计算精确性
static inline void test_weekly_precision(void) {
	// 精确验证星期计算：2023-01-01是星期日(0)，要求星期六(6)执行
	// 预期是2023-01-07
	const ct_time_t before   = create_test_time(2023, 1, 1, 0, 0, 0);  // 2023-01-01 00:00:00 (星期日)
	const ct_time_t expected = create_test_time(2023, 1, 7, 0, 0, 0);  // 2023-01-07 00:00:00 (星期六)
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 6, -1);

	assert_int64_eq(expected, next);
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

	test_day_31_skip();
	cunit_println("Finish! test_day_31_skip();");

	test_day_30_skip();
	cunit_println("Finish! test_day_30_skip();");

	test_multiple_month_skip();
	cunit_println("Finish! test_multiple_month_skip();");

	test_weekly_cross_month();
	cunit_println("Finish! test_weekly_cross_month();");

	test_weekly_cross_year();
	cunit_println("Finish! test_weekly_cross_year();");

	test_time_passed_same_day();
	cunit_println("Finish! test_time_passed_same_day();");

	test_time_passed_monthly();
	cunit_println("Finish! test_time_passed_monthly();");

	test_complex_monthly_time();
	cunit_println("Finish! test_complex_monthly_time();");

	test_complex_yearly_time();
	cunit_println("Finish! test_complex_yearly_time();");

	test_leap_year_yearly_skip();
	cunit_println("Finish! test_leap_year_yearly_skip();");

	test_boundary_max_day();
	cunit_println("Finish! test_boundary_max_day();");

	test_boundary_yearly_passed();
	cunit_println("Finish! test_boundary_yearly_passed();");

	test_weekly_precision();
	cunit_println("Finish! test_weekly_precision();");

	cunit_pass();
}
