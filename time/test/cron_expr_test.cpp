#include <catch.hpp>

#include "coter/time/cron.h"

static inline ct_time_t create_test_time(int year, int month, int day, int hour, int min, int sec) {
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	tm.tm_year  = year - 1900;
	tm.tm_mon   = month - 1;
	tm.tm_mday  = day;
	tm.tm_hour  = hour;
	tm.tm_min   = min;
	tm.tm_sec   = sec;
	tm.tm_isdst = -1;
	return mktime(&tm);
}

TEST_CASE("cron_next_minutely", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
	const ct_time_t expected = create_test_time(2000, 1, 1, 0, 1, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, -1, -1, -1, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_hourly", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
	const ct_time_t expected = create_test_time(2000, 1, 1, 1, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, -1, -1, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_daily", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
	const ct_time_t expected = create_test_time(2000, 1, 2, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_weekly", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
	const ct_time_t expected = create_test_time(2000, 1, 2, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 0, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_monthly", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
	const ct_time_t expected = create_test_time(2000, 2, 1, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_yearly", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
	const ct_time_t expected = create_test_time(2001, 1, 1, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_invalid_params", "[cron_next]") {
	const ct_time_t now = time(nullptr);
	REQUIRE(-1 == ct_cron_next_timeout(now, 60, -1, -1, -1, -1));
	REQUIRE(-1 == ct_cron_next_timeout(now, -1, 24, -1, -1, -1));
	REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, 32, -1, -1));
	REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, -1, 7, -1));
	REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, -1, -1, 13));
}

TEST_CASE("cron_next_cross_month_boundary", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 1, 31, 23, 59, 0);
	const ct_time_t expected = create_test_time(2000, 2, 1, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_cross_year_boundary", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 12, 31, 23, 59, 0);
	const ct_time_t expected = create_test_time(2001, 1, 1, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_leap_year", "[cron_next]") {
	const ct_time_t before   = create_test_time(2000, 2, 28, 23, 59, 0);
	const ct_time_t expected = create_test_time(2000, 2, 29, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_non_leap_year", "[cron_next]") {
	const ct_time_t before   = create_test_time(2001, 2, 28, 23, 59, 0);
	const ct_time_t expected = create_test_time(2001, 3, 29, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_day_31_skip", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 1, 31, 0, 0, 0);
	const ct_time_t expected = create_test_time(2023, 3, 31, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_day_30_skip", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 1, 30, 0, 0, 0);
	const ct_time_t expected = create_test_time(2023, 3, 30, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 30, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_multiple_month_skip", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 4, 15, 0, 0, 0);
	const ct_time_t expected = create_test_time(2023, 5, 31, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_weekly_cross_month", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 1, 30, 0, 0, 0);
	const ct_time_t expected = create_test_time(2023, 2, 1, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 3, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_weekly_cross_year", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 12, 30, 0, 0, 0);
	const ct_time_t expected = create_test_time(2024, 1, 1, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_time_passed_same_day", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 6, 15, 14, 30, 0);
	const ct_time_t expected = create_test_time(2023, 6, 16, 10, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 10, -1, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_time_passed_monthly", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 6, 20, 0, 0, 0);
	const ct_time_t expected = create_test_time(2023, 7, 15, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 15, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_complex_monthly_time", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 3, 15, 10, 0, 0);
	const ct_time_t expected = create_test_time(2023, 3, 28, 14, 30, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 30, 14, 28, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_complex_yearly_time", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 6, 15, 10, 0, 0);
	const ct_time_t expected = create_test_time(2023, 12, 25, 9, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 9, 25, -1, 12);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_leap_year_yearly_skip", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 1, 15, 0, 0, 0);
	const ct_time_t expected = create_test_time(2024, 2, 29, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, 2);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_boundary_max_day", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 12, 15, 0, 0, 0);
	const ct_time_t expected = create_test_time(2023, 12, 31, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_boundary_yearly_passed", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 6, 15, 0, 0, 0);
	const ct_time_t expected = create_test_time(2024, 1, 1, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);
	REQUIRE(expected == next);
}

TEST_CASE("cron_next_weekly_precision", "[cron_next]") {
	const ct_time_t before   = create_test_time(2023, 1, 1, 0, 0, 0);
	const ct_time_t expected = create_test_time(2023, 1, 7, 0, 0, 0);
	const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 6, -1);
	REQUIRE(expected == next);
}
