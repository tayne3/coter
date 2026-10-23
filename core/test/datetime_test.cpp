#include <climits>
#include <cstdio>
#include <cstring>
#include <string>

#include "coter/core/datetime.h"
#include "coter/testing/doctest.h"

namespace {

bool is_zero_datetime(const ct_datetime_t& dt) {
    return dt.year == 0 && dt.month == 0 && dt.day == 0 && dt.wday == 0 && dt.hour == 0 && dt.min == 0 &&
           dt.sec == 0 && dt.ms == 0;
}

}  // namespace

TEST_CASE("datetime: days_of_month" * doctest::test_suite("datetime")) {
    CHECK(ct_datetime_days_of_month(1, 2024) == 31);
    CHECK(ct_datetime_days_of_month(2, 2023) == 28);
    CHECK(ct_datetime_days_of_month(2, 2024) == 29);
    CHECK(ct_datetime_days_of_month(2, 1900) == 28);
    CHECK(ct_datetime_days_of_month(2, 2000) == 29);
    CHECK(ct_datetime_days_of_month(4, 2024) == 30);
    CHECK(ct_datetime_days_of_month(12, 2024) == 31);
    CHECK(ct_datetime_days_of_month(0, 2024) == 0);
    CHECK(ct_datetime_days_of_month(13, 2024) == 0);
}

TEST_CASE("datetime: month conversion" * doctest::test_suite("datetime")) {
    CHECK(ct_datetime_month_atoi("Jan") == 1);
    CHECK(ct_datetime_month_atoi("January") == 1);
    CHECK(ct_datetime_month_atoi("feb") == 2);
    CHECK(ct_datetime_month_atoi("December") == 12);
    CHECK(ct_datetime_month_atoi("") == 0);
    CHECK(ct_datetime_month_atoi("Ma") == 0);
    CHECK(ct_datetime_month_atoi("Mar") == 3);
    CHECK(ct_datetime_month_atoi("May") == 5);
    CHECK(ct_datetime_month_atoi("Janx") == 0);
    CHECK(std::strcmp(ct_datetime_month_itoa(1), "January") == 0);
    CHECK(std::strcmp(ct_datetime_month_itoa(2), "February") == 0);
    CHECK(std::strcmp(ct_datetime_month_itoa(12), "December") == 0);
}

TEST_CASE("datetime: weekday conversion" * doctest::test_suite("datetime")) {
    CHECK(ct_datetime_weekday_atoi("Sun") == 0);
    CHECK(ct_datetime_weekday_atoi("Sunday") == 0);
    CHECK(ct_datetime_weekday_atoi("Mon") == 1);
    CHECK(ct_datetime_weekday_atoi("Monday") == 1);
    CHECK(ct_datetime_weekday_atoi("Saturday") == 6);
    CHECK(ct_datetime_weekday_atoi("") == -1);
    CHECK(ct_datetime_weekday_atoi("Mo") == -1);
    CHECK(ct_datetime_weekday_atoi("Monx") == -1);
    CHECK(std::strcmp(ct_datetime_weekday_itoa(0), "Sunday") == 0);
    CHECK(std::strcmp(ct_datetime_weekday_itoa(1), "Monday") == 0);
    CHECK(std::strcmp(ct_datetime_weekday_itoa(6), "Saturday") == 0);
    CHECK(std::strcmp(ct_datetime_weekday_itoa(7), "Sunday") == 0);
}

TEST_CASE("datetime: formatting" * doctest::test_suite("datetime")) {
    const ct_datetime_t dt = {2024, 2, 29, 4, 12, 34, 56, 789};
    char                buf[CT_DATETIME_FMT_BUFLEN];

    CHECK(ct_datetime_fmt(&dt, buf) == buf);
    CHECK(std::strcmp(buf, "2024-02-29 12:34:56") == 0);
    CHECK(ct_datetime_fmt(nullptr, buf) == nullptr);
    CHECK(ct_datetime_fmt_iso(nullptr, buf) == nullptr);

    char duration[CT_TIME_FMT_BUFLEN];
    CHECK(ct_datetime_duration_fmt(3661, duration) == duration);
    CHECK(std::strcmp(duration, "01:01:01") == 0);
    CHECK(ct_datetime_duration_fmt(-1, duration) == nullptr);
    CHECK(ct_tm_duration_fmt(-1, duration) == nullptr);

    char expected[CT_TIME_FMT_BUFLEN];
    std::snprintf(expected, sizeof(expected), "%02d:%02d:%02d", INT_MAX / 3600, (INT_MAX / 60) % 60,
                  INT_MAX % 60);
    CHECK(ct_datetime_duration_fmt(INT_MAX, duration) == duration);
    CHECK(std::strcmp(duration, expected) == 0);
    CHECK(ct_tm_duration_fmt(INT_MAX, duration) == duration);
    CHECK(std::strcmp(duration, expected) == 0);

    char gmt[CT_GMTIME_FMT_BUFLEN];
    CHECK(ct_datetime_gmtime_fmt(0, gmt) == gmt);
    CHECK(std::strcmp(gmt, "Thu, 01 Jan 1970 00:00:00 GMT") == 0);

    struct tm tm    = {};
    tm.tm_year      = 124;
    tm.tm_mon       = 1;
    tm.tm_mday      = 29;
    tm.tm_hour      = 12;
    tm.tm_min       = 34;
    tm.tm_sec       = 56;
    char tm_buf[CT_TM_FMT_MAX];
    CHECK(ct_tm_fmt(&tm, tm_buf) == tm_buf);
    CHECK(std::strcmp(tm_buf, "2024-02-29 12:34:56") == 0);
    CHECK(ct_tm_fmt(nullptr, tm_buf) == nullptr);
}

TEST_CASE("datetime: past and future" * doctest::test_suite("datetime")) {
    ct_datetime_t dt = {2024, 3, 1, 5, 12, 0, 0, 0};

    CHECK(ct_datetime_past(&dt, 1) == &dt);
    CHECK(dt.year == 2024);
    CHECK(dt.month == 2);
    CHECK(dt.day == 29);
    CHECK(dt.wday == 4);

    dt = {2024, 12, 31, 2, 23, 59, 59, 999};
    CHECK(ct_datetime_future(&dt, 1) == &dt);
    CHECK(dt.year == 2025);
    CHECK(dt.month == 1);
    CHECK(dt.day == 1);
    CHECK(dt.wday == 3);

    CHECK(ct_datetime_past(nullptr, 1) == nullptr);
    CHECK(ct_datetime_future(nullptr, 1) == nullptr);
}

TEST_CASE("datetime: now and mktime" * doctest::test_suite("datetime")) {
    const ct_datetime_t now = ct_datetime_now();

    CHECK(now.year >= 2000);
    CHECK(now.month >= 1);
    CHECK(now.month <= 12);
    CHECK(now.day >= 1);
    CHECK(now.day <= 31);
    CHECK(now.hour >= 0);
    CHECK(now.hour <= 23);
    CHECK(now.min >= 0);
    CHECK(now.min <= 59);
    CHECK(now.sec >= 0);
    CHECK(now.sec <= 60);
    CHECK(now.ms >= 0);
    CHECK(now.ms <= 999);

    CHECK(ct_datetime_mktime(nullptr) == (ct_time_t)-1);

    const ct_datetime_t dt = {2026, 1, 15, 4, 12, 0, 0, 0};
    const ct_time_t     ts = ct_datetime_mktime(&dt);
    REQUIRE(ts != (ct_time_t)-1);

    const ct_datetime_t back = ct_datetime_localtime(ts);
    CHECK(back.year == dt.year);
    CHECK(back.month == dt.month);
    CHECK(back.day == dt.day);
    CHECK(back.hour == dt.hour);
    CHECK(back.min == dt.min);
    CHECK(back.sec == dt.sec);
    CHECK(back.wday == dt.wday);
}

TEST_CASE("datetime: compile-time parsing" * doctest::test_suite("datetime")) {
    const ct_datetime_t dt = ct_datetime_compile_impl("Jan 15 2026", "12:00:00");

    CHECK(dt.year == 2026);
    CHECK(dt.month == 1);
    CHECK(dt.day == 15);
    CHECK(dt.wday == 4);
    CHECK(dt.hour == 12);
    CHECK(dt.min == 0);
    CHECK(dt.sec == 0);
    CHECK(dt.ms == 0);

    CHECK(is_zero_datetime(ct_datetime_compile_impl(nullptr, "12:00:00")));
    CHECK(is_zero_datetime(ct_datetime_compile_impl("Jan 15 2026", nullptr)));
    CHECK(is_zero_datetime(ct_datetime_compile_impl("Feb 30 2026", "12:00:00")));
    CHECK(is_zero_datetime(ct_datetime_compile_impl("Jan 15 2026", "25:00:00")));
    CHECK(is_zero_datetime(ct_datetime_compile_impl("Jan 15 2026x", "12:00:00")));
    CHECK(is_zero_datetime(ct_datetime_compile_impl("Jan 15 2026", "12:00:00x")));

    std::string long_date(1024, 'A');
    long_date += " 15 2026";
    CHECK(is_zero_datetime(ct_datetime_compile_impl(long_date.c_str(), "12:00:00")));

    std::string long_year = "Jan 15 ";
    long_year.append(64, '9');
    CHECK(is_zero_datetime(ct_datetime_compile_impl(long_year.c_str(), "12:00:00")));

    const ct_datetime_t padded = ct_datetime_compile_impl("Jan  5 2026", "03:04:05");
    CHECK(padded.year == 2026);
    CHECK(padded.month == 1);
    CHECK(padded.day == 5);
    CHECK(padded.hour == 3);
    CHECK(padded.min == 4);
    CHECK(padded.sec == 5);

    const ct_datetime_t compiled = ct_datetime_compile();
    CHECK(compiled.year >= 2000);
    CHECK(compiled.month >= 1);
    CHECK(compiled.month <= 12);
    CHECK(compiled.day >= 1);
    CHECK(compiled.day <= 31);
    CHECK(compiled.wday >= 0);
    CHECK(compiled.wday <= 6);
    CHECK(compiled.ms == 0);
}
