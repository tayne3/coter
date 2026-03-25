#include <catch.hpp>

#include "coter/time/cron.h"

namespace {
ct_time_t create_test_time(int year, int month, int day, int hour, int min, int sec) {
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
}  // namespace

TEST_CASE("computes next minutely timeout", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
    const ct_time_t expected = create_test_time(2000, 1, 1, 0, 1, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, -1, -1, -1, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("computes next hourly timeout", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
    const ct_time_t expected = create_test_time(2000, 1, 1, 1, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, -1, -1, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("computes next daily timeout", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
    const ct_time_t expected = create_test_time(2000, 1, 2, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("computes next weekly timeout", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
    const ct_time_t expected = create_test_time(2000, 1, 2, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 0, -1);
    REQUIRE(expected == next);
}

TEST_CASE("computes next monthly timeout", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
    const ct_time_t expected = create_test_time(2000, 2, 1, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("computes next yearly timeout", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 1, 1, 0, 0, 0);
    const ct_time_t expected = create_test_time(2001, 1, 1, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);
    REQUIRE(expected == next);
}

TEST_CASE("returns -1 for invalid parameters", "[cron_next]") {
    const ct_time_t now = time(nullptr);
    REQUIRE(-1 == ct_cron_next_timeout(now, 60, -1, -1, -1, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, 24, -1, -1, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, 32, -1, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, -1, 7, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, -1, -1, 13));
}

TEST_CASE("handles month boundary correctly", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 1, 31, 23, 59, 0);
    const ct_time_t expected = create_test_time(2000, 2, 1, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("handles year boundary correctly", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 12, 31, 23, 59, 0);
    const ct_time_t expected = create_test_time(2001, 1, 1, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);
    REQUIRE(expected == next);
}

TEST_CASE("handles leap year correctly", "[cron_next]") {
    const ct_time_t before   = create_test_time(2000, 2, 28, 23, 59, 0);
    const ct_time_t expected = create_test_time(2000, 2, 29, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("handles non-leap year correctly", "[cron_next]") {
    const ct_time_t before   = create_test_time(2001, 2, 28, 23, 59, 0);
    const ct_time_t expected = create_test_time(2001, 3, 29, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("skips months without day 31", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 1, 31, 0, 0, 0);
    const ct_time_t expected = create_test_time(2023, 3, 31, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("skips months without day 30", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 1, 30, 0, 0, 0);
    const ct_time_t expected = create_test_time(2023, 3, 30, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 30, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("skips multiple months when day doesn't exist", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 4, 15, 0, 0, 0);
    const ct_time_t expected = create_test_time(2023, 5, 31, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("handles weekly schedule crossing month boundary", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 1, 30, 0, 0, 0);
    const ct_time_t expected = create_test_time(2023, 2, 1, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 3, -1);
    REQUIRE(expected == next);
}

TEST_CASE("handles weekly schedule crossing year boundary", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 12, 30, 0, 0, 0);
    const ct_time_t expected = create_test_time(2024, 1, 1, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("advances to next day when time has passed", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 6, 15, 14, 30, 0);
    const ct_time_t expected = create_test_time(2023, 6, 16, 10, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 10, -1, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("advances to next month when day has passed", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 6, 20, 0, 0, 0);
    const ct_time_t expected = create_test_time(2023, 7, 15, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 15, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("handles complex monthly schedule with specific time", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 3, 15, 10, 0, 0);
    const ct_time_t expected = create_test_time(2023, 3, 28, 14, 30, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 30, 14, 28, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("handles complex yearly schedule with specific time", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 6, 15, 10, 0, 0);
    const ct_time_t expected = create_test_time(2023, 12, 25, 9, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 9, 25, -1, 12);
    REQUIRE(expected == next);
}

TEST_CASE("handles yearly schedule in leap year", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 1, 15, 0, 0, 0);
    const ct_time_t expected = create_test_time(2024, 2, 29, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 29, -1, 2);
    REQUIRE(expected == next);
}

TEST_CASE("handles maximum day boundary", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 12, 15, 0, 0, 0);
    const ct_time_t expected = create_test_time(2023, 12, 31, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 31, -1, -1);
    REQUIRE(expected == next);
}

TEST_CASE("advances to next year when month has passed", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 6, 15, 0, 0, 0);
    const ct_time_t expected = create_test_time(2024, 1, 1, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, 1, -1, 1);
    REQUIRE(expected == next);
}

TEST_CASE("computes weekly schedule with specific day", "[cron_next]") {
    const ct_time_t before   = create_test_time(2023, 1, 1, 0, 0, 0);
    const ct_time_t expected = create_test_time(2023, 1, 7, 0, 0, 0);
    const ct_time_t next     = ct_cron_next_timeout(before, 0, 0, -1, 6, -1);
    REQUIRE(expected == next);
}
