#include <ctime>

#include "coter/testing/doctest.h"
#include "coter/time/cron.h"

namespace {

struct CronExpr {
    int min_ = -1, hour_ = -1, day_ = -1, week_ = -1, month_ = -1;

    CronExpr& min(int v) {
        min_ = v;
        return *this;
    }
    CronExpr& hour(int v) {
        hour_ = v;
        return *this;
    }
    CronExpr& day(int v) {
        day_ = v;
        return *this;
    }
    CronExpr& week(int v) {
        week_ = v;
        return *this;
    }
    CronExpr& month(int v) {
        month_ = v;
        return *this;
    }

    ct_time_t next_after(ct_time_t now) const { return ct_cron_next_timeout(now, min_, hour_, day_, week_, month_); }
};

void check_cron(ct_time_t before, ct_time_t expected, const CronExpr& expr = CronExpr{}) {
    REQUIRE(expected == expr.next_after(before));
}

ct_time_t make_time(int year, int month, int day, int hour, int min, int sec) {
    struct tm tm;

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

TEST_SUITE_BEGIN("cron");

TEST_CASE("returns -1 for invalid parameters") {
    const ct_time_t now = ct_time(nullptr);
    REQUIRE(-1 == ct_cron_next_timeout(now, 60, -1, -1, -1, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, 24, -1, -1, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, 32, -1, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, -1, 7, -1));
    REQUIRE(-1 == ct_cron_next_timeout(now, -1, -1, -1, -1, 13));
}

TEST_CASE("computes next timeout correctly") {
    const ct_time_t y2k = make_time(2000, 1, 1, 0, 0, 0);

    SUBCASE("basic periodic timeouts") {
        // every minute (all default -1)
        check_cron(y2k, make_time(2000, 1, 1, 0, 1, 0));
        // every hour
        check_cron(y2k, make_time(2000, 1, 1, 1, 0, 0), CronExpr().min(0));
        // every day
        check_cron(y2k, make_time(2000, 1, 2, 0, 0, 0), CronExpr().min(0).hour(0));
        // every week
        check_cron(y2k, make_time(2000, 1, 2, 0, 0, 0), CronExpr().min(0).hour(0).week(0));
        // every month
        check_cron(y2k, make_time(2000, 2, 1, 0, 0, 0), CronExpr().min(0).hour(0).day(1));
        // every year
        check_cron(y2k, make_time(2001, 1, 1, 0, 0, 0), CronExpr().min(0).hour(0).day(1).month(1));

        // specific day of the week
        check_cron(make_time(2023, 1, 1, 0, 0, 0), make_time(2023, 1, 7, 0, 0, 0), CronExpr().min(0).hour(0).week(6));
    }

    SUBCASE("month and year boundaries") {
        check_cron(make_time(2000, 1, 31, 23, 59, 0), make_time(2000, 2, 1, 0, 0, 0), CronExpr().min(0).hour(0).day(1));

        check_cron(make_time(2000, 12, 31, 23, 59, 0), make_time(2001, 1, 1, 0, 0, 0),
                   CronExpr().min(0).hour(0).day(1).month(1));

        check_cron(make_time(2023, 1, 30, 0, 0, 0), make_time(2023, 2, 1, 0, 0, 0), CronExpr().min(0).hour(0).week(3));

        check_cron(make_time(2023, 12, 30, 0, 0, 0), make_time(2024, 1, 1, 0, 0, 0), CronExpr().min(0).hour(0).week(1));
    }

    SUBCASE("leap years vs non-leap years") {
        check_cron(make_time(2000, 2, 28, 23, 59, 0), make_time(2000, 2, 29, 0, 0, 0),
                   CronExpr().min(0).hour(0).day(29));

        check_cron(make_time(2001, 2, 28, 23, 59, 0), make_time(2001, 3, 29, 0, 0, 0),
                   CronExpr().min(0).hour(0).day(29));

        check_cron(make_time(2023, 1, 15, 0, 0, 0), make_time(2024, 2, 29, 0, 0, 0),
                   CronExpr().min(0).hour(0).day(29).month(2));
    }

    SUBCASE("skipping invalid calendar days") {
        // skips months without day 31
        check_cron(make_time(2023, 1, 31, 0, 0, 0), make_time(2023, 3, 31, 0, 0, 0), CronExpr().min(0).hour(0).day(31));

        // skips months without day 30
        check_cron(make_time(2023, 1, 30, 0, 0, 0), make_time(2023, 3, 30, 0, 0, 0), CronExpr().min(0).hour(0).day(30));

        // skips multiple months when day doesn't exist
        check_cron(make_time(2023, 4, 15, 0, 0, 0), make_time(2023, 5, 31, 0, 0, 0), CronExpr().min(0).hour(0).day(31));
    }

    SUBCASE("time advancements and complex schedules") {
        // advances to next day when time has passed
        check_cron(make_time(2023, 6, 15, 14, 30, 0), make_time(2023, 6, 16, 10, 0, 0), CronExpr().min(0).hour(10));

        // advances to next month when day has passed
        check_cron(make_time(2023, 6, 20, 0, 0, 0), make_time(2023, 7, 15, 0, 0, 0), CronExpr().min(0).hour(0).day(15));

        // advances to next year when month has passed
        check_cron(make_time(2023, 6, 15, 0, 0, 0), make_time(2024, 1, 1, 0, 0, 0),
                   CronExpr().min(0).hour(0).day(1).month(1));

        // complex monthly schedule with specific time
        check_cron(make_time(2023, 3, 15, 10, 0, 0), make_time(2023, 3, 28, 14, 30, 0),
                   CronExpr().min(30).hour(14).day(28));

        // complex yearly schedule with specific time
        check_cron(make_time(2023, 6, 15, 10, 0, 0), make_time(2023, 12, 25, 9, 0, 0),
                   CronExpr().min(0).hour(9).day(25).month(12));

        // maximum day boundary
        check_cron(make_time(2023, 12, 15, 0, 0, 0), make_time(2023, 12, 31, 0, 0, 0),
                   CronExpr().min(0).hour(0).day(31));
    }
}

TEST_SUITE_END();
