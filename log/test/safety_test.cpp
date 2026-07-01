#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/log/handler/console.h"
#include "coter/log/handler/file.h"
#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"


namespace {
struct safety_callback_state {
    std::atomic<size_t> calls{0};
    std::atomic<bool>   destroyed{false};
};

struct recursive_callback_state {
    ct_logger_t*        logger{nullptr};
    std::atomic<size_t> calls{0};
};

struct close_callback_state {
    ct_logger_t*        logger{nullptr};
    std::atomic<size_t> calls{0};
    std::atomic<int>    close_result{0};
};

void safety_log_callback(const ct_log_record_t* record, void* userdata) {
    (void)record;
    auto* state = static_cast<safety_callback_state*>(userdata);
    state->calls++;
    // Simulate slow processing
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
    while (std::chrono::steady_clock::now() < deadline) { CT_PAUSE(); }
}

void slow_log_callback(const ct_log_record_t* record, void* userdata) {
    (void)record;
    auto* state = static_cast<safety_callback_state*>(userdata);
    state->calls++;
    // Simulate slow processing
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
    while (std::chrono::steady_clock::now() < deadline) { CT_PAUSE(); }
}

void recursive_log_callback(const ct_log_record_t* record, void* userdata) {
    (void)record;
    auto* state = static_cast<recursive_callback_state*>(userdata);
    if (++state->calls == 1) { CT_LOGGER_TRACE(state->logger, "recursive handler log"); }
}

void close_log_callback(const ct_log_record_t* record, void* userdata) {
    (void)record;
    auto* state = static_cast<close_callback_state*>(userdata);
    state->calls++;
    state->close_result = ct_logger_close(state->logger);
}
}  // namespace

TEST_CASE("log_safety_lifecycle" * doctest::test_suite("log") * doctest::test_suite("safety")) {
    safety_callback_state state;

    // 1. Create a logger on heap
    ct_logger_t* logger = (ct_logger_t*)malloc(sizeof(ct_logger_t));
    ct_logger_init(logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = safety_log_callback;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(logger) == 0);

    // Verify sealing logic: cannot add handler after registration
    ct_log_handler_t* late_handler = ct_log_record_handler_create(&config);
    REQUIRE(ct_logger_add_handler(logger, late_handler) == -1);
    ct_log_handler_destroy(late_handler);

    std::atomic<bool> saboteur_done{false};
    std::atomic<int>  close_errors{0};

    // 2. Start a Saboteur Thread that constantly creates and destroys loggers
    std::thread saboteur([&]() {
        for (int i = 0; i < 50; ++i) {
            ct_logger_t* temp_logger = (ct_logger_t*)malloc(sizeof(ct_logger_t));
            ct_logger_init(temp_logger);

            ct_log_record_handler_config_t temp_config;
            ct_log_record_handler_config_default(&temp_config);
            temp_config.routine  = safety_log_callback;
            temp_config.userdata = &state;

            ct_logger_add_handler(temp_logger, ct_log_record_handler_create(&temp_config));
            ct_logger_start(temp_logger);

            CT_LOGGER_TRACE(temp_logger, "Sabotage message %d\n", i);

            std::this_thread::yield();
            if (ct_logger_close(temp_logger) != 0) { ++close_errors; }
            free(temp_logger);
        }
        saboteur_done = true;
    });

    // 3. Fire many logs on main thread
    for (int i = 0; i < 100; ++i) { CT_LOGGER_TRACE(logger, "Safety test message %d\n", i); }

    // 4. Immediately close the primary logger while logs are still in flight
    REQUIRE(ct_logger_close(logger) == 0);

    saboteur.join();

    REQUIRE(state.calls >= 150);
    REQUIRE(close_errors == 0);

    free(logger);
}

TEST_CASE("log_close_rejects_concurrent_producers" * doctest::test_suite("log") * doctest::test_suite("safety")) {
    safety_callback_state state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = safety_log_callback;
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

TEST_CASE("log_close_with_slow_handler_does_not_hang" * doctest::test_suite("log") * doctest::test_suite("safety")) {
    constexpr int         kRecords = 300;
    safety_callback_state state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = slow_log_callback;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_TRACE(&logger, "slow close %d", i); }

    auto start = std::chrono::steady_clock::now();
    REQUIRE(ct_logger_close(&logger) == 0);
    auto elapsed = std::chrono::steady_clock::now() - start;

    REQUIRE(elapsed < std::chrono::seconds(2));
    REQUIRE(state.calls == static_cast<size_t>(kRecords));
}

TEST_CASE("log_queue_full_blocks_without_dropping" * doctest::test_suite("log") * doctest::test_suite("safety")) {
    constexpr int         kRecords = 1500;
    safety_callback_state state;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = slow_log_callback;
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

TEST_CASE("log_recursive_handler_log_does_not_deadlock" * doctest::test_suite("log") * doctest::test_suite("safety")) {
    ct_logger_t logger;
    ct_logger_init(&logger);

    recursive_callback_state state;
    state.logger = &logger;

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = recursive_log_callback;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    CT_LOGGER_TRACE(&logger, "outer handler log");
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(state.calls == 1);
}

TEST_CASE("log_close_inside_handler_does_not_deadlock" * doctest::test_suite("log") * doctest::test_suite("safety")) {
    ct_logger_t logger;
    ct_logger_init(&logger);

    close_callback_state state;
    state.logger = &logger;

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = close_log_callback;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    CT_LOGGER_TRACE(&logger, "close inside handler");
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(state.calls == 1);
    REQUIRE(state.close_result == -1);
}

// ---------------------------------------------------------------------------
// T4-A: console handler 并发写安全
// ---------------------------------------------------------------------------

TEST_CASE("log_console_handler_concurrent_write" * doctest::test_suite("log") * doctest::test_suite("safety") *
          doctest::test_suite("handler")) {
    constexpr int kThreads   = 4;
    constexpr int kPerThread = 500;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_console_handler_config_t cfg;
    ct_log_console_handler_config_default(&cfg);
    cfg.stream = stderr;  // 避免干扰 doctest 的 stdout 捕获
    REQUIRE(ct_logger_add_handler(&logger, ct_log_console_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < kPerThread; ++j) { CT_LOGGER_INFO(&logger, "t=%d s=%d", i, j); }
        });
    }
    for (auto& t : threads) { t.join(); }

    // close 内部调用 flush + destroy，正常退出即证明无死锁、无崩溃
    REQUIRE(ct_logger_close(&logger) == 0);
}

// ---------------------------------------------------------------------------
// T4-B: file handler 并发写不丢行
// ---------------------------------------------------------------------------

TEST_CASE("log_file_handler_concurrent_write" * doctest::test_suite("log") * doctest::test_suite("safety") *
          doctest::test_suite("handler")) {
    constexpr int kThreads   = 4;
    constexpr int kPerThread = 1000;
    constexpr int kExpected  = kThreads * kPerThread;
    const char*   kDir       = "test_concurrent_file_out";
    const char*   kName      = "concurrent";
    const char*   kFilePath  = "test_concurrent_file_out/concurrent.log0";

    std::remove(kFilePath);
    ct_rmdir(kDir);

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_file_handler_config_t cfg;
    ct_log_file_handler_config_default(&cfg);
    std::strncpy(cfg.dir, kDir, sizeof(cfg.dir) - 1);
    std::strncpy(cfg.name, kName, sizeof(cfg.name) - 1);
    cfg.size_max  = 256UL * 1024 * 1024;  // 256 MB，不触发轮转
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

    // 统计文件行数：必须等于总发送条数（有锁且正确时不丢行）
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
