/**
 * @file handler_test.cpp
 * @brief handler 层并发与文本格式化测试
 *
 * 覆盖：
 *  - Text Handler 格式化与颜色配置的连通性
 *  - Record Handler 多线程写入时的序列安全
 *  - Text Handler 并发写入安全
 */
#include <atomic>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "coter/core/time.h"
#include "coter/log/handler/record.h"
#include "coter/log/handler/text.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("record handler safely captures all payloads during high concurrent writes") {
    constexpr int kThreads   = 16;
    constexpr int kPerThread = 500;
    constexpr int kExpected  = kThreads * kPerThread;

    struct State {
        std::atomic<int> calls{0};
        std::atomic<int> valid{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t* r, void* ud) {
        auto* s = static_cast<State*>(ud);
        s->calls.fetch_add(1, std::memory_order_relaxed);
        if (r && r->data && r->size > 0) { s->valid.fetch_add(1, std::memory_order_relaxed); }
    };
    cfg.userdata = &state;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < kPerThread; ++j) {
                CT_LOGGER_INFO(&logger, "thread=%d seq=%d payload=0x%08X", i, j, i * kPerThread + j);
            }
        });
    }
    for (auto& t : threads) { t.join(); }
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(state.calls.load() == kExpected);
    REQUIRE(state.valid.load() == kExpected);
}

TEST_CASE("text handler formats basic strings and flushes correctly") {
    struct State {
        std::string last;
        int         calls{0};
        int         flushes{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_text_handler_config_t cfg;
    ct_log_text_handler_config_default(&cfg);
    cfg.routine = [](const char* buf, size_t len, void* ud) {
        auto* s = static_cast<State*>(ud);
        s->last.assign(buf, len);
        s->calls++;
    };
    cfg.flush        = [](void* ud) { static_cast<State*>(ud)->flushes++; };
    cfg.userdata     = &state;
    cfg.enable_color = false;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_text_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    SUBCASE("write produces formatted output in text buffer") {
        CT_LOGGER_INFO(&logger, "hello text handler");
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(state.calls >= 1);
        REQUIRE(state.last.find("hello text handler") != std::string::npos);
        REQUIRE(state.flushes >= 1);
        REQUIRE(ct_logger_close(&logger) == 0);
    }

    SUBCASE("flush and close succeed without data") {
        REQUIRE(ct_logger_close(&logger) == 0);
    }
}

TEST_CASE("text handler safely handles concurrent writes from multiple threads") {
    constexpr int kThreads   = 16;
    constexpr int kPerThread = 300;

    struct State {
        std::atomic<int> calls{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_text_handler_config_t cfg;
    ct_log_text_handler_config_default(&cfg);
    cfg.routine = [](const char*, size_t, void* ud) {
        static_cast<State*>(ud)->calls.fetch_add(1, std::memory_order_relaxed);
    };
    cfg.userdata     = &state;
    cfg.enable_color = false;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_text_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < kPerThread; ++j) { CT_LOGGER_INFO(&logger, "t=%d j=%d", i, j); }
        });
    }
    for (auto& t : threads) { t.join(); }
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(state.calls.load() == kThreads * kPerThread);
}

TEST_SUITE_END();
