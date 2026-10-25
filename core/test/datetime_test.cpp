#include "coter/core/datetime.h"

#include <climits>
#include <cstdio>
#include <cstring>
#include <limits>

#include "coter/testing/doctest.h"

namespace {

bool is_zero_datetime(const ct_datetime_t& dt) {
    return dt.year == 0 && dt.month == 0 && dt.day == 0 && dt.hour == 0 && dt.min == 0 && dt.sec == 0 && dt.ms == 0;
}

bool same_datetime(const ct_datetime_t& a, const ct_datetime_t& b) {
    return a.year == b.year && a.month == b.month && a.day == b.day && a.hour == b.hour && a.min == b.min &&
           a.sec == b.sec && a.ms == b.ms;
}

}  // namespace

TEST_CASE("datetime: days_in_month" * doctest::test_suite("datetime")) {
    CHECK(ct_datetime_days_in_month(2024, 1) == 31);
    CHECK(ct_datetime_days_in_month(2023, 2) == 28);
    CHECK(ct_datetime_days_in_month(2024, 2) == 29);
    CHECK(ct_datetime_days_in_month(1900, 2) == 28);
    CHECK(ct_datetime_days_in_month(2000, 2) == 29);
    CHECK(ct_datetime_days_in_month(2024, 4) == 30);
    CHECK(ct_datetime_days_in_month(2024, 12) == 31);
    CHECK(ct_datetime_days_in_month(2024, 0) == 0);
    CHECK(ct_datetime_days_in_month(2024, 13) == 0);
}

TEST_CASE("datetime: formatting" * doctest::test_suite("datetime")) {
    const ct_datetime_t dt = {2024, 2, 29, 12, 34, 56, 789};

    char buf[CT_DATETIME_FMT_BUFLEN];
    CHECK(ct_datetime_fmt(&dt, buf, sizeof(buf)) == buf);
    CHECK(std::strcmp(buf, "2024-02-29 12:34:56") == 0);

    char iso[CT_DATETIME_FMT_ISO_BUFLEN];
    CHECK(ct_datetime_fmt_iso(&dt, iso, sizeof(iso)) == iso);
    CHECK(std::strcmp(iso, "2024-02-29T12:34:56.789") == 0);

    CHECK(ct_datetime_fmt(nullptr, buf, sizeof(buf)) == nullptr);
    CHECK(ct_datetime_fmt_iso(nullptr, iso, sizeof(iso)) == nullptr);
    CHECK(ct_datetime_fmt(&dt, nullptr, 0) == nullptr);
    CHECK(ct_datetime_fmt_iso(&dt, nullptr, 0) == nullptr);

    CHECK(ct_datetime_fmt(&dt, buf, 19) == nullptr);
    CHECK(buf[0] == '\0');
    CHECK(ct_datetime_fmt_iso(&dt, iso, 23) == nullptr);
    CHECK(iso[0] == '\0');

    const ct_datetime_t bad_date = {2024, 2, 30, 0, 0, 0, 0};
    CHECK(ct_datetime_fmt(&bad_date, buf, sizeof(buf)) == nullptr);
    CHECK(buf[0] == '\0');

    const ct_datetime_t bad_ms = {2024, 1, 1, 0, 0, 0, 1000};
    CHECK(ct_datetime_fmt_iso(&bad_ms, iso, sizeof(iso)) == nullptr);

    char duration[CT_DURATION_FMT_BUFLEN];
    CHECK(ct_duration_fmt(3661, duration, sizeof(duration)) == duration);
    CHECK(std::strcmp(duration, "01:01:01") == 0);
    CHECK(ct_duration_fmt(0, duration, sizeof(duration)) == duration);
    CHECK(std::strcmp(duration, "00:00:00") == 0);
    CHECK(ct_duration_fmt(-1, duration, sizeof(duration)) == nullptr);
    CHECK(duration[0] == '\0');
    CHECK(ct_duration_fmt(1, nullptr, 0) == nullptr);
    CHECK(ct_duration_fmt(3661, duration, 5) == nullptr);
    CHECK(duration[0] == '\0');

    char expected[CT_DURATION_FMT_BUFLEN];
    std::snprintf(expected, sizeof(expected), "%02d:%02d:%02d", INT_MAX / 3600, (INT_MAX / 60) % 60, INT_MAX % 60);
    CHECK(ct_duration_fmt(INT_MAX, duration, sizeof(duration)) == duration);
    CHECK(std::strcmp(duration, expected) == 0);
}

TEST_CASE("datetime: add_days" * doctest::test_suite("datetime")) {
    ct_datetime_t dt = {2024, 3, 1, 12, 0, 0, 0};
    CHECK(ct_datetime_add_days(&dt, -1));
    CHECK(dt.year == 2024);
    CHECK(dt.month == 2);
    CHECK(dt.day == 29);
    CHECK(dt.hour == 12);

    dt = {2024, 12, 31, 23, 59, 59, 999};
    CHECK(ct_datetime_add_days(&dt, 1));
    CHECK(dt.year == 2025);
    CHECK(dt.month == 1);
    CHECK(dt.day == 1);
    CHECK(dt.hour == 23);
    CHECK(dt.min == 59);
    CHECK(dt.sec == 59);
    CHECK(dt.ms == 999);

    dt = {2024, 3, 1, 12, 0, 0, 0};
    CHECK(ct_datetime_add_days(&dt, 1));
    CHECK(dt.year == 2024);
    CHECK(dt.month == 3);
    CHECK(dt.day == 2);

    dt = {2024, 1, 1, 0, 0, 0, 0};
    CHECK(ct_datetime_add_days(&dt, -1));
    CHECK(dt.year == 2023);
    CHECK(dt.month == 12);
    CHECK(dt.day == 31);

    dt = {2024, 2, 29, 12, 0, 0, 0};
    CHECK(ct_datetime_add_days(&dt, 0));
    CHECK(dt.year == 2024);
    CHECK(dt.month == 2);
    CHECK(dt.day == 29);

    ct_datetime_t       low        = {1, 1, 1, 0, 0, 0, 0};
    const ct_datetime_t low_before = low;
    CHECK_FALSE(ct_datetime_add_days(&low, -1));
    CHECK(same_datetime(low, low_before));

    ct_datetime_t       high        = {9999, 12, 31, 23, 59, 59, 999};
    const ct_datetime_t high_before = high;
    CHECK_FALSE(ct_datetime_add_days(&high, 1));
    CHECK(same_datetime(high, high_before));

    ct_datetime_t invalid = {2024, 2, 30, 0, 0, 0, 0};
    CHECK_FALSE(ct_datetime_add_days(&invalid, 1));
    CHECK_FALSE(ct_datetime_add_days(nullptr, 1));

    ct_datetime_t       huge        = {2024, 1, 1, 0, 0, 0, 0};
    const ct_datetime_t huge_before = huge;
    CHECK_FALSE(ct_datetime_add_days(&huge, INT_MAX));
    CHECK(same_datetime(huge, huge_before));
    CHECK_FALSE(ct_datetime_add_days(&huge, INT_MIN));
    CHECK(same_datetime(huge, huge_before));
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

    ct_datetime_t bad = {2024, 2, 30, 0, 0, 0, 0};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);
    bad = {2024, 13, 1, 0, 0, 0, 0};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);
    bad = {0, 1, 1, 0, 0, 0, 0};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);
    bad = {10000, 1, 1, 0, 0, 0, 0};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);
    bad = {2024, 1, 1, 24, 0, 0, 0};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);
    bad = {2024, 1, 1, 0, 60, 0, 0};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);
    bad = {2024, 1, 1, 0, 0, 61, 0};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);
    bad = {2024, 1, 1, 0, 0, 0, 1000};
    CHECK(ct_datetime_mktime(&bad) == (ct_time_t)-1);

    const ct_datetime_t dt = {2026, 1, 15, 12, 0, 0, 0};
    const ct_time_t     ts = ct_datetime_mktime(&dt);
    REQUIRE(ts != (ct_time_t)-1);

    const ct_datetime_t back = ct_datetime_localtime(ts);
    CHECK(back.year == dt.year);
    CHECK(back.month == dt.month);
    CHECK(back.day == dt.day);
    CHECK(back.hour == dt.hour);
    CHECK(back.min == dt.min);
    CHECK(back.sec == dt.sec);
    CHECK(back.ms == 0);

    ct_datetime_t with_ms = dt;
    with_ms.ms            = 999;
    CHECK(ct_datetime_mktime(&with_ms) == ts);

    if (sizeof(ct_time_t) >= 8) {
        CHECK(is_zero_datetime(ct_datetime_localtime((ct_time_t)std::numeric_limits<ct_time_t>::max())));
    }
}
