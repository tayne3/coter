/**
 * @file logger_test.cpp
 * @brief ct_logger_t 核心生命周期及 API 测试
 *
 * 覆盖：
 *  - 默认单例 logger 的惰性初始化与封印机制
 *  - logger 对象的生命周期（init, start, close, flush）
 *  - 级别控制、非法级别过滤
 *  - handler 的独占性（单一所有者）
 *  - 并发安全性验证
 */
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("default logger is implicitly initialized and cannot be closed") {
    ct_logger_t* logger = ct_logger_get_default();
    REQUIRE(logger != nullptr);
    REQUIRE(ct_logger_is_enabled(logger, CT_LOG_LEVEL_TRACE));

    CT_TRACE("zero init");
    REQUIRE(ct_logger_flush(NULL) == 0);
    REQUIRE(ct_logger_close(logger) == -1);
}

TEST_CASE("default logger becomes sealed after being retrieved") {
    ct_logger_t* builtin = ct_logger_get_default();
    REQUIRE(builtin != nullptr);

    ct_logger_t unstarted;
    ct_logger_init(&unstarted);
    REQUIRE(ct_logger_set_default(&unstarted) == -1);

    struct State {
        std::atomic<size_t> calls{0};
        std::atomic<size_t> flushes{0};
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t* r, void* ud) {
        if (r && ud) static_cast<State*>(ud)->calls++;
    };
    cfg.flush = [](void* ud) {
        if (ud) static_cast<State*>(ud)->flushes++;
    };
    cfg.userdata = &st;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    REQUIRE(ct_logger_set_default(&logger) == -1);
    REQUIRE(ct_logger_set_default(NULL) == -1);
    REQUIRE(ct_logger_get_default() == builtin);
    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(st.calls == 0);
    REQUIRE(st.flushes == 1);

    REQUIRE(ct_logger_close(&unstarted) == 0);
}

TEST_CASE("logger object API allows setting levels and logging correctly") {
    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_logger_set_level(&logger, CT_LOG_LEVEL_WARNING);
    REQUIRE(ct_logger_get_level(&logger) == CT_LOG_LEVEL_WARNING);

    struct State {
        size_t     calls      = 0;
        size_t     bytes      = 0;
        int        last_level = -1;
        char       data[32]   = {};
        std::mutex mtx;
    } st;

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t* r, void* ud) {
        if (!r || !r->data || !ud) return;
        auto*                       s = static_cast<State*>(ud);
        std::lock_guard<std::mutex> lock(s->mtx);
        s->calls++;
        s->bytes += r->size;
        s->last_level = r->level;
        if (r->size < sizeof(s->data)) {
            std::memcpy(s->data, r->data, r->size);
            s->data[r->size] = '\0';
        }
    };
    cfg.userdata = &st;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    REQUIRE(!ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    ct_logger_set_level(&logger, CT_LOG_LEVEL_TRACE);
    REQUIRE(ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    ct_log_handler_t* default_handler = ct_log_record_handler_create(&cfg);
    REQUIRE(ct_logger_add_handler(ct_logger_get_default(), default_handler) == -1);
    ct_log_handler_destroy(default_handler);

    ct_log_handler_t* late_handler = ct_log_record_handler_create(&cfg);
    REQUIRE(ct_logger_add_handler(&logger, late_handler) == -1);
    ct_log_handler_destroy(late_handler);

    CT_LOGGER_TRACE(&logger, "object");
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(st.calls == 1);
    REQUIRE(st.bytes == 6);
    REQUIRE(st.last_level == CT_LOG_LEVEL_TRACE);
    REQUIRE(std::strcmp(st.data, "object") == 0);
}

TEST_CASE("a handler can only belong to a single logger") {
    ct_logger_t first;
    ct_logger_t second;
    ct_logger_init(&first);
    ct_logger_init(&second);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t*, void*) {};

    ct_log_handler_t* handler = ct_log_record_handler_create(&cfg);
    REQUIRE(handler != nullptr);
    REQUIRE(ct_logger_add_handler(&first, handler) == 0);
    REQUIRE(ct_logger_add_handler(&second, handler) == -1);

    REQUIRE(ct_logger_close(&first) == 0);
    REQUIRE(ct_logger_close(&second) == 0);
}

TEST_CASE("an unstarted logger does not dispatch logs to handlers") {
    struct State {
        size_t calls = 0;
        size_t bytes = 0;
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t* r, void* ud) {
        if (!r || !ud) return;
        auto* s = static_cast<State*>(ud);
        s->calls++;
        s->bytes += r->size;
    };
    cfg.userdata = &st;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);

    CT_LOGGER_TRACE(&logger, "sync");
    REQUIRE(st.calls == 0);
    REQUIRE(st.bytes == 0);

    REQUIRE(ct_logger_close(&logger) == 0);
}

TEST_CASE("invalid log levels are safely rejected by the logger") {
    struct State {
        size_t calls = 0;
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine  = [](const ct_log_record_t*, void* ud) { static_cast<State*>(ud)->calls++; };
    cfg.userdata = &st;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    REQUIRE(!ct_logger_is_enabled(&logger, -1));
    REQUIRE(!ct_logger_is_enabled(&logger, CT_LOG_LEVEL_COUNT));

    ct_logger_set_level(&logger, CT_LOG_LEVEL_ERROR);
    ct_logger_set_level(&logger, CT_LOG_LEVEL_COUNT);
    REQUIRE(ct_logger_get_level(&logger) == CT_LOG_LEVEL_ERROR);

    CT_LOGGER_LOG(&logger, -1, "bad");
    CT_LOGGER_LOG(&logger, CT_LOG_LEVEL_COUNT, "bad");

    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(st.calls == 0);
}

TEST_CASE("closing a logger flushes all its active handlers") {
    struct State {
        std::atomic<size_t> calls{0};
        std::atomic<size_t> flushes{0};
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t* r, void* ud) {
        if (r && ud) static_cast<State*>(ud)->calls++;
    };
    cfg.flush = [](void* ud) {
        if (ud) static_cast<State*>(ud)->flushes++;
    };
    cfg.userdata = &st;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    CT_LOGGER_TRACE(&logger, "deferred flush");
    REQUIRE(ct_logger_flush(&logger) == 0);
    REQUIRE(st.calls == 1);
    REQUIRE(st.flushes == 1);

    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(st.calls == 1);
    REQUIRE(st.flushes == 2);
}

TEST_CASE("adding a handler after the logger has started is rejected safely without data races") {
    ct_logger_t logger;
    ct_logger_init(&logger);

    struct State {
        std::atomic<size_t> calls{0};
    } first_st, second_st;

    ct_log_record_handler_config_t first_cfg;
    ct_log_record_handler_config_default(&first_cfg);
    first_cfg.routine  = [](const ct_log_record_t*, void* ud) { static_cast<State*>(ud)->calls++; };
    first_cfg.userdata = &first_st;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&first_cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    constexpr int kThreads = 4;
    constexpr int kRecords = 1000;

    std::atomic<int>         ready{0};
    std::atomic<bool>        start{false};
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&]() {
            ++ready;
            while (!start) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
            for (int j = 0; j < kRecords; ++j) {
                CT_LOGGER_TRACE(&logger, "line\n");
                if (j % 100 == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
            }
        });
    }

    while (ready != kThreads) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
    start = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    ct_log_record_handler_config_t second_cfg;
    ct_log_record_handler_config_default(&second_cfg);
    second_cfg.routine  = [](const ct_log_record_t*, void* ud) { static_cast<State*>(ud)->calls++; };
    second_cfg.userdata = &second_st;

    ct_log_handler_t* second_handler = ct_log_record_handler_create(&second_cfg);
    REQUIRE(ct_logger_add_handler(&logger, second_handler) == -1);
    ct_log_handler_destroy(second_handler);

    for (auto& t : threads) { t.join(); }

    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(first_st.calls > 0);
    REQUIRE(first_st.calls <= static_cast<size_t>(kThreads * kRecords));
    REQUIRE(second_st.calls == 0);
}

TEST_CASE("logger level can be read and written concurrently without data races") {
    ct_logger_t logger;
    ct_logger_init(&logger);
    REQUIRE(ct_logger_start(&logger) == 0);

    constexpr int            kThreads = 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 10000; ++j) {
                ct_logger_set_level(&logger, j % 2 == 0 ? CT_LOG_LEVEL_TRACE : CT_LOG_LEVEL_ERROR);
                (void)ct_logger_get_level(&logger);
            }
        });
    }

    for (auto& t : threads) { t.join(); }

    ct_logger_set_level(&logger, CT_LOG_LEVEL_ERROR);
    REQUIRE(ct_logger_get_level(&logger) == CT_LOG_LEVEL_ERROR);
    REQUIRE(!ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    REQUIRE(ct_logger_close(&logger) == 0);
}

TEST_SUITE_END();
