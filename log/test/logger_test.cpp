#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

#include "coter/log/log.h"

#define test_trace(...)                  CT_LOG_BASIC(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define test_logger_trace(__logger, ...) CT_LOG_BASIC(TRACE, __logger, __VA_ARGS__)

namespace {
struct callback_state {
    size_t calls      = 0;
    size_t bytes      = 0;
    int    last_level = -1;
    char   data[32]   = {};
};

void collect_log(const ct_log_record_t* record, void* userdata) {
    REQUIRE(record != nullptr);
    REQUIRE(record->data != nullptr);
    static std::mutex           g_callback_mtx;
    std::lock_guard<std::mutex> lock(g_callback_mtx);
    auto*                       state = static_cast<callback_state*>(userdata);
    state->calls++;
    state->bytes += record->size;
    state->last_level = record->level;
    if (record->size < sizeof(state->data)) {
        std::memcpy(state->data, record->data, record->size);
        state->data[record->size] = '\0';
    }
}
}  // namespace

TEST_CASE("log_logger_lifecycle", "[log]") {
    ct_log_close();
    REQUIRE(ct_log_init(NULL) == 0);
    REQUIRE(ct_log_init(NULL) == -1);

    ct_log_close();
    REQUIRE(ct_log_init(NULL) == 0);

    ct_log_close();
    ct_log_close();
}

TEST_CASE("log_logger_object_api", "[log]") {
    ct_log_close();
    REQUIRE(ct_log_init(NULL) == 0);
    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_logger_set_level(&logger, CT_LOG_LEVEL_WARNING);
    REQUIRE(ct_logger_get_level(&logger) == CT_LOG_LEVEL_WARNING);

    callback_state                   state;
    ct_log_callback_handler_config_t config;
    ct_log_callback_handler_config_default(&config);
    config.routine  = collect_log;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&config)) == 0);
    REQUIRE(ct_logger_register(&logger) == 0);

    REQUIRE(!ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    ct_logger_set_level(&logger, CT_LOG_LEVEL_VERBOSE);
    REQUIRE(ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    REQUIRE(ct_logger_add_handler(ct_log_get_default(), ct_log_callback_handler_create(&config)) == -1);
    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&config)) == -1);
    test_logger_trace(&logger, "object");

    // In Phase 2, dispatcher is async
    REQUIRE(state.calls == 0);
    ct_log_flush();
    REQUIRE(state.calls == 1);
    REQUIRE(state.bytes == 6);
    REQUIRE(state.last_level == CT_LOG_LEVEL_TRACE);
    REQUIRE(std::strcmp(state.data, "object") == 0);

    ct_log_close();
}

TEST_CASE("log_default_fallback", "[log]") {
    ct_log_close();
    REQUIRE(ct_log_init(NULL) == 0);

    callback_state                   state;
    ct_log_callback_handler_config_t config;
    ct_log_callback_handler_config_default(&config);
    config.routine  = collect_log;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(ct_log_get_default(), ct_log_callback_handler_create(&config)) == -1);

    ct_logger_t logger;
    ct_logger_init(&logger);
    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&config)) == 0);
    ct_logger_register(&logger);
    ct_log_set_default(&logger);

    test_trace("abc");
    // In Phase 2, dispatcher is async
    REQUIRE(state.calls == 0);

    ct_log_flush();
    REQUIRE(state.calls == 1);
    REQUIRE(state.bytes == 3);

    ct_log_close();
}

TEST_CASE("log_logger_add_handler_while_writing", "[log]") {
    ct_log_close();
    REQUIRE(ct_log_init(NULL) == 0);

    ct_logger_t logger;
    ct_logger_init(&logger);

    callback_state                   first;
    ct_log_callback_handler_config_t first_config;
    ct_log_callback_handler_config_default(&first_config);
    first_config.routine  = collect_log;
    first_config.userdata = &first;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&first_config)) == 0);
    ct_logger_register(&logger);

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
                test_logger_trace(&logger, "line\n");
                if (j % 100 == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
            }
        });
    }

    while (ready != kThreads) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
    start = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    callback_state                   second;
    ct_log_callback_handler_config_t second_config;
    ct_log_callback_handler_config_default(&second_config);
    second_config.routine  = collect_log;
    second_config.userdata = &second;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&second_config)) == -1);

    for (auto& t : threads) { t.join(); }

    ct_log_flush();
    REQUIRE(first.calls == static_cast<size_t>(kThreads * kRecords));

    ct_log_close();
}

TEST_CASE("log_logger_level_is_atomic", "[log]") {
    ct_log_close();
    REQUIRE(ct_log_init(NULL) == 0);

    ct_logger_t logger;
    ct_logger_init(&logger);
    ct_logger_register(&logger);

    constexpr int            kThreads = 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 10000; ++j) {
                ct_logger_set_level(&logger, j % 2 == 0 ? CT_LOG_LEVEL_VERBOSE : CT_LOG_LEVEL_ERROR);
                (void)ct_logger_get_level(&logger);
            }
        });
    }

    for (auto& t : threads) { t.join(); }

    ct_logger_set_level(&logger, CT_LOG_LEVEL_ERROR);
    REQUIRE(ct_logger_get_level(&logger) == CT_LOG_LEVEL_ERROR);
    REQUIRE(!ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    ct_log_close();
}
