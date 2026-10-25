#include "coter/core/time.h"

#include <cstdlib>
#include <ctime>
#include <thread>
#include <vector>

#include "coter/testing/doctest.h"

TEST_CASE("uptime_monotonic") {
    const ct_time64_t t1 = ct_gettick_ms();
    const ct_time64_t t2 = ct_gettick_ms();
    REQUIRE(t2 >= t1);
    ct_time64_t prev = ct_gettick_ms();
    for (int i = 0; i < 10; ++i) {
        const ct_time64_t curr = ct_gettick_ms();
        REQUIRE(curr >= prev);
        prev = curr;
    }
}

TEST_CASE("sleep_timing" * doctest::test_suite("timing")) {
    const int durations[] = {10, 50, 100, 200};
    for (int i = 0; i < 4; ++i) {
        const ct_time64_t start_us = ct_gethrtime_us();
        ct_msleep(durations[i]);
        const ct_time64_t end_us     = ct_gethrtime_us();
        const ct_time64_t elapsed_ms = (end_us - start_us) / 1000;
        REQUIRE(elapsed_ms >= (ct_time64_t)durations[i]);
    }
    const ct_time64_t start_us = ct_gethrtime_us();
    ct_usleep(0);
    const ct_time64_t end_us   = ct_gethrtime_us();
    const ct_time64_t quick_ms = (end_us - start_us) / 1000;
    REQUIRE(quick_ms <= 5);
}

TEST_CASE("hrtime_monotonic") {
    ct_time64_t prev = ct_gethrtime_us();
    for (int i = 0; i < 10; ++i) {
        const ct_time64_t curr = ct_gethrtime_us();
        REQUIRE(curr >= 0);
        REQUIRE(curr >= prev);
        prev = curr;
    }
}

TEST_CASE("timeofday_consistency" * doctest::test_suite("timing")) {
    const ct_time64_t us1 = ct_unixtime_us();
    const ct_time64_t ms1 = us1 / 1000;
    const ct_time64_t ms2 = ct_unixtime_ms();
    REQUIRE(ms2 >= ms1);
    REQUIRE(ms2 <= ms1 + 10);
}

TEST_CASE("time_now_consistency" * doctest::test_suite("timing")) {
    const ct_time_t   now = ct_unixtime_sec();
    const ct_time64_t ms  = ct_unixtime_ms();
    REQUIRE(std::llabs((long long)now - (long long)(ms / 1000)) <= 2);
}

TEST_CASE("localtime_r_consistency" * doctest::test_suite("concurrency")) {
    struct tm       tmv{};
    const ct_time_t now = ct_unixtime_sec();
    REQUIRE(ct_localtime_r(&now, &tmv) != nullptr);
    const ct_time_t t_mk = std::mktime(&tmv);
    REQUIRE(t_mk != (ct_time_t)-1);
    REQUIRE(std::llabs((long long)t_mk - (long long)now) <= 1);
    REQUIRE(tmv.tm_mon >= 0);
    REQUIRE(tmv.tm_mon <= 11);
    REQUIRE(tmv.tm_mday >= 1);
    REQUIRE(tmv.tm_mday <= 31);
    REQUIRE(tmv.tm_hour >= 0);
    REQUIRE(tmv.tm_hour <= 23);
    REQUIRE(tmv.tm_min >= 0);
    REQUIRE(tmv.tm_min <= 59);
    REQUIRE(tmv.tm_sec >= 0);
    REQUIRE(tmv.tm_sec <= 60);
}

TEST_CASE("localtime_r_concurrent" * doctest::test_suite("concurrency")) {
    const int                threads = 4;
    const int                loops   = 20;
    std::vector<std::thread> ts;
    ts.reserve(threads);
    for (int t = 0; t < threads; ++t) {
        ts.emplace_back([&]() {
            for (int i = 0; i < loops; ++i) {
                struct tm       tmv{};
                const ct_time_t now = ct_unixtime_sec();
                REQUIRE(ct_localtime_r(&now, &tmv) != nullptr);
                const ct_time_t mk = std::mktime(&tmv);
                REQUIRE(mk != (ct_time_t)-1);
                REQUIRE(std::llabs((long long)mk - (long long)now) <= 1);
            }
        });
    }
    for (auto& th : ts) { th.join(); }
}

TEST_CASE("sleep_seconds_quick" * doctest::test_suite("timing")) {
    const ct_time64_t start_us = ct_gethrtime_us();
    ct_sleep(0);
    const ct_time64_t end_us   = ct_gethrtime_us();
    const ct_time64_t quick_ms = (end_us - start_us) / 1000;
    REQUIRE(quick_ms <= 5);
}

TEST_CASE("usleep_minimum_behavior" * doctest::test_suite("timing")) {
    const ct_time64_t s1 = ct_gethrtime_us();
    ct_usleep(1);
    const ct_time64_t e1  = ct_gethrtime_us();
    const ct_time64_t ms1 = (e1 - s1) / 1000;
#ifdef CT_OS_WIN
    REQUIRE(ms1 >= 1);
#else
    REQUIRE(ms1 >= 0);
#endif
    REQUIRE(ms1 <= 20);

    const ct_time64_t s2 = ct_gethrtime_us();
    ct_usleep(500);
    const ct_time64_t e2  = ct_gethrtime_us();
    const ct_time64_t ms2 = (e2 - s2) / 1000;
#ifdef CT_OS_WIN
    REQUIRE(ms2 >= 1);
#else
    REQUIRE(ms2 >= 0);
#endif
    REQUIRE(ms2 <= 20);
}
