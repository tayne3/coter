/**
 * @file print_test.cpp
 * @brief 日志打印格式性能与稳定性测试
 */
#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/core/macro.h"
#include "coter/log/log.h"

#define test_basic_trace(...)  CT_LOGGER_BASIC(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define test_brief_trace(...)  CT_LOGGER_BRIEF(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define test_detail_trace(...) CT_LOGGER_DETAIL(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)

namespace {
static constexpr int kTestThreads    = 4;
static constexpr int kTestThreadData = 100;

template <typename Func>
int run_parallel_test(Func test_func) {
    std::atomic<int>         ready{0};
    std::atomic<bool>        start{false};
    std::vector<std::thread> threads;

    auto start_time = ct_getuptime_ms();
    for (int i = 0; i < kTestThreads; ++i) {
        threads.emplace_back([&]() {
            ++ready;
            while (!start) { std::this_thread::yield(); }
            for (int j = 0; j < kTestThreadData; ++j) { test_func(); }
        });
    }

    while (ready != kTestThreads) { std::this_thread::yield(); }
    start = true;

    for (auto& t : threads) { t.join(); }
    return static_cast<int>(ct_getuptime_ms() - start_time);
}
}  // namespace

TEST_CASE("log_print_performance", "[log][perf]") {
    REQUIRE(ct_log_init(NULL) == 0);
    REQUIRE(ct_logger_is_enabled(ct_log_get_default(), CT_LOG_LEVEL_VERBOSE));

#define PRINT_CALL(F)                                                                                                  \
    run_parallel_test([&]() {                                                                                          \
        F("%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234, \
          1234, 1234, 0xFFFF0000ULL, 0xFFFF0000ULL, 0xFFFF0000ULL, 0xFFFF0000ULL, "test1", "test2", "test3", "test4",  \
          0x00, 0x01, 0x02, 0x03);                                                                                     \
    })

    SECTION("performance comparison between formats") {
        auto time_without = PRINT_CALL(printf);
        auto time_basic   = PRINT_CALL(test_basic_trace);
        auto time_brief   = PRINT_CALL(test_brief_trace);
        auto time_detail  = PRINT_CALL(test_detail_trace);

        {
            char buf[1024];
            snprintf(buf, sizeof(buf),
                     "Execution time:\n Without:  %d ms\n  Basic:  %d ms\n  Brief:  %d ms\n  Detail: %d ms\n",
                     time_without, time_basic, time_brief, time_detail);
            INFO(buf);
        }

        REQUIRE(true);
    }

    ct_log_close();
}
