/**
 * @file safety_test.cpp
 * @brief logger 及各类 handler 的并发安全和生命周期边界测试
 *
 * 覆盖：
 *  - 高并发下的 handler 动态挂载、销毁与日志记录的混合操作
 *  - 关闭过程中的并发写入拦截
 *  - 耗时 handler 对 close 语义的影响（排空、不挂死）
 *  - 递归日志拦截与防死锁
 *  - 在 handler 回调中尝试关闭 logger 的防御
 *  - console / file handler 在多线程并发写入时的线程安全（不丢行、不崩溃）
 */
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/log/handler/console.h"
#include "coter/log/handler/file.h"
#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("logger safely handles concurrent handler creation, destruction, and logging") {
    struct State {
        std::atomic<size_t> calls{0};
    } state;

    auto safety_callback = [](const ct_log_record_t*, void* ud) {
        auto* s = static_cast<State*>(ud);
        s->calls++;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
        while (std::chrono::steady_clock::now() < deadline) { CT_PAUSE(); }
    };

    ct_logger_t* logger = (ct_logger_t*)std::malloc(sizeof(ct_logger_t));
    ct_logger_init(logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = safety_callback;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(logger) == 0);

    ct_log_handler_t* late_handler = ct_log_record_handler_create(&config);
    REQUIRE(ct_logger_add_handler(logger, late_handler) == -1);
    ct_log_handler_destroy(late_handler);

    std::atomic<bool> saboteur_done{false};
    std::atomic<int>  close_errors{0};

    std::thread saboteur([&]() {
        for (int i = 0; i < 50; ++i) {
            ct_logger_t* temp_logger = (ct_logger_t*)std::malloc(sizeof(ct_logger_t));
            ct_logger_init(temp_logger);

            ct_log_record_handler_config_t temp_config;
            ct_log_record_handler_config_default(&temp_config);
            temp_config.routine  = safety_callback;
            temp_config.userdata = &state;

            ct_logger_add_handler(temp_logger, ct_log_record_handler_create(&temp_config));
            ct_logger_start(temp_logger);

            CT_LOGGER_TRACE(temp_logger, "Sabotage message %d\n", i);

            std::this_thread::yield();
            if (ct_logger_close(temp_logger) != 0) { ++close_errors; }
            std::free(temp_logger);
        }
        saboteur_done = true;
    });

    for (int i = 0; i < 100; ++i) { CT_LOGGER_TRACE(logger, "Safety test message %d\n", i); }

    REQUIRE(ct_logger_close(logger) == 0);

    saboteur.join();

    REQUIRE(state.calls >= 150);
    REQUIRE(close_errors == 0);

    std::free(logger);
}

TEST_CASE("logger correctly rejects logs from concurrent producers during close") {
    struct State {
        std::atomic<size_t> calls{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine = [](const ct_log_record_t*, void* ud) {
        auto* s = static_cast<State*>(ud);
        s->calls++;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
        while (std::chrono::steady_clock::now() < deadline) { CT_PAUSE(); }
    };
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::atomic<bool>        stop{false};
    std::atomic<int>         ready{0};
    std::vector<std::thread> producers;
    for (int i = 0; i < 4; ++i) {
        producers.emplace_back([&]() {
            ++ready;
            while (!stop) {
                CT_LOGGER_TRACE(&logger, "concurrent close");
                std::this_thread::yield();
            }
        });
    }

    while (ready != 4) { std::this_thread::yield(); }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    REQUIRE(ct_logger_close(&logger) == 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stop = true;
    for (auto& producer : producers) { producer.join(); }

    REQUIRE(state.calls > 0);
}

TEST_CASE("closing logger waits for slow handlers without hanging indefinitely") {
    constexpr int kRecords = 300;

    struct State {
        std::atomic<size_t> calls{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine = [](const ct_log_record_t*, void* ud) {
        auto* s = static_cast<State*>(ud);
        s->calls++;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
        while (std::chrono::steady_clock::now() < deadline) { CT_PAUSE(); }
    };
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_TRACE(&logger, "slow close %d", i); }

    auto start = std::chrono::steady_clock::now();
    REQUIRE(ct_logger_close(&logger) == 0);
    auto elapsed = std::chrono::steady_clock::now() - start;

    REQUIRE(elapsed < std::chrono::seconds(5));
    REQUIRE(state.calls == static_cast<size_t>(kRecords));
}

TEST_CASE("full log queue correctly blocks without dropping messages") {
    constexpr int kRecords = 1500;

    struct State {
        std::atomic<size_t> calls{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine = [](const ct_log_record_t*, void* ud) {
        auto* s = static_cast<State*>(ud);
        s->calls++;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
        while (std::chrono::steady_clock::now() < deadline) { CT_PAUSE(); }
    };
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_TRACE(&logger, "queue full %d", i); }
    auto elapsed = std::chrono::steady_clock::now() - start;

    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(elapsed > std::chrono::milliseconds(50));
    REQUIRE(state.calls == static_cast<size_t>(kRecords));
}

TEST_CASE("recursive logging within a handler does not cause deadlock") {
    struct State {
        ct_logger_t*        logger{nullptr};
        std::atomic<size_t> calls{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);
    state.logger = &logger;

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine = [](const ct_log_record_t*, void* ud) {
        auto* s = static_cast<State*>(ud);
        if (++s->calls == 1) { CT_LOGGER_TRACE(s->logger, "recursive handler log"); }
    };
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    CT_LOGGER_TRACE(&logger, "outer handler log");
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(state.calls == 1);
}

TEST_CASE("closing the logger from within a handler is safely rejected without deadlock") {
    struct State {
        ct_logger_t*        logger{nullptr};
        std::atomic<size_t> calls{0};
        std::atomic<int>    close_result{0};
    } state;

    ct_logger_t logger;
    ct_logger_init(&logger);
    state.logger = &logger;

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine = [](const ct_log_record_t*, void* ud) {
        auto* s = static_cast<State*>(ud);
        s->calls++;
        s->close_result = ct_logger_close(s->logger);
    };
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    CT_LOGGER_TRACE(&logger, "close inside handler");
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(state.calls == 1);
    REQUIRE(state.close_result == -1);
}

TEST_CASE("console handler safely handles concurrent writes from multiple threads") {
    constexpr int kThreads   = 16;
    constexpr int kPerThread = 500;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_console_handler_config_t cfg;
    ct_log_console_handler_config_default(&cfg);
    cfg.stream = stderr;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_console_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < kPerThread; ++j) { CT_LOGGER_INFO(&logger, "t=%d s=%d", i, j); }
        });
    }
    for (auto& t : threads) { t.join(); }

    REQUIRE(ct_logger_close(&logger) == 0);
}

TEST_CASE("file handler safely handles concurrent writes from multiple threads without dropping lines") {
    constexpr int kThreads   = 16;
    constexpr int kPerThread = 1000;
    constexpr int kExpected  = kThreads * kPerThread;

    const char* kDir      = "test_concurrent_file_out";
    const char* kName     = "concurrent";
    const char* kFilePath = "test_concurrent_file_out/concurrent.log0";

    std::remove(kFilePath);
    ct_rmdir(kDir);

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_file_handler_config_t cfg;
    ct_log_file_handler_config_default(&cfg);
    std::strncpy(cfg.dir, kDir, sizeof(cfg.dir) - 1);
    std::strncpy(cfg.name, kName, sizeof(cfg.name) - 1);
    cfg.size_max  = 256UL * 1024 * 1024;
    cfg.count_max = 1;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_file_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < kPerThread; ++j) { CT_LOGGER_INFO(&logger, "t%d-%d", i, j); }
        });
    }
    for (auto& t : threads) { t.join(); }
    REQUIRE(ct_logger_close(&logger) == 0);

    FILE* f = std::fopen(kFilePath, "rb");
    REQUIRE(f != nullptr);
    int lines = 0;
    int c;
    while ((c = std::fgetc(f)) != EOF) {
        if (c == '\n') { ++lines; }
    }
    std::fclose(f);

    std::remove(kFilePath);
    ct_rmdir(kDir);

    REQUIRE(lines == kExpected);
}

TEST_SUITE_END();
