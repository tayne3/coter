#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "coter/log/handler/callback.h"
#include "coter/log/log.h"

namespace {
struct callback_state {
    size_t calls      = 0;
    size_t bytes      = 0;
    int    last_level = -1;
    char   data[32]   = {};
};

void collect_log(const ct_log_record_t* record, void* userdata) {
    if (!record || !record->data || !userdata) { return; }
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

void destroy_handler(ct_log_handler_t* handler) {
    if (handler && handler->vtable && handler->vtable->destroy) { handler->vtable->destroy(handler); }
}
}  // namespace

TEST_CASE("log_default_logger_zero_init", "[log]") {
    ct_logger_t* logger = ct_logger_default();
    REQUIRE(logger != nullptr);
    REQUIRE(ct_logger_is_enabled(logger, CT_LOG_LEVEL_VERBOSE));

    CT_TRACE("zero init");
    ct_logger_close(logger);
}

TEST_CASE("log_global_config_freezes_after_first_use", "[log]") {
    ct_logger_t* logger = ct_logger_default();
    REQUIRE(logger != nullptr);

    ct_logger_config_t config    = CT_LOGGER_CONFIG_INITIALIZER;
    config.dispatcher_queue_size = 2;
    REQUIRE(ct_logger_set_global_config(&config) == -1);
}

TEST_CASE("log_logger_object_api", "[log]") {
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
    REQUIRE(ct_logger_start(&logger) == 0);

    REQUIRE(!ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    ct_logger_set_level(&logger, CT_LOG_LEVEL_VERBOSE);
    REQUIRE(ct_logger_is_enabled(&logger, CT_LOG_LEVEL_TRACE));

    ct_log_handler_t* default_handler = ct_log_callback_handler_create(&config);
    REQUIRE(ct_logger_add_handler(ct_logger_default(), default_handler) == -1);
    destroy_handler(default_handler);

    ct_log_handler_t* late_handler = ct_log_callback_handler_create(&config);
    REQUIRE(ct_logger_add_handler(&logger, late_handler) == -1);
    destroy_handler(late_handler);

    CT_LOGGER_TRACE(&logger, "object");
    REQUIRE(state.calls == 0);

    ct_logger_close(&logger);
    REQUIRE(state.calls == 1);
    REQUIRE(state.bytes == 6);
    REQUIRE(state.last_level == CT_LOG_LEVEL_TRACE);
    REQUIRE(strcmp(state.data, "object") == 0);
}

TEST_CASE("log_unstarted_logger_uses_sync_path", "[log]") {
    callback_state state;
    ct_logger_t    logger;
    ct_logger_init(&logger);

    ct_log_callback_handler_config_t config;
    ct_log_callback_handler_config_default(&config);
    config.routine  = collect_log;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&config)) == 0);

    CT_LOGGER_TRACE(&logger, "sync");
    REQUIRE(state.calls == 1);
    REQUIRE(state.bytes == 4);

    ct_logger_close(&logger);
}

TEST_CASE("log_invalid_level_is_rejected", "[log]") {
    callback_state state;
    ct_logger_t    logger;
    ct_logger_init(&logger);

    ct_log_callback_handler_config_t config;
    ct_log_callback_handler_config_default(&config);
    config.routine  = collect_log;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    REQUIRE(!ct_logger_is_enabled(&logger, -1));
    REQUIRE(!ct_logger_is_enabled(&logger, CT_LOG_LEVEL_COUNT));

    ct_logger_set_level(&logger, CT_LOG_LEVEL_ERROR);
    ct_logger_set_level(&logger, CT_LOG_LEVEL_COUNT);
    REQUIRE(ct_logger_get_level(&logger) == CT_LOG_LEVEL_ERROR);

    CT_LOGGER_LOG(&logger, -1, "bad");
    CT_LOGGER_LOG(&logger, CT_LOG_LEVEL_COUNT, "bad");

    ct_logger_close(&logger);
    REQUIRE(state.calls == 0);
}

TEST_CASE("log_logger_add_handler_after_start_fails", "[log]") {
    ct_logger_t logger;
    ct_logger_init(&logger);

    callback_state                   first;
    ct_log_callback_handler_config_t first_config;
    ct_log_callback_handler_config_default(&first_config);
    first_config.routine  = collect_log;
    first_config.userdata = &first;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&first_config)) == 0);
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

    callback_state                   second;
    ct_log_callback_handler_config_t second_config;
    ct_log_callback_handler_config_default(&second_config);
    second_config.routine  = collect_log;
    second_config.userdata = &second;

    ct_log_handler_t* second_handler = ct_log_callback_handler_create(&second_config);
    REQUIRE(ct_logger_add_handler(&logger, second_handler) == -1);
    destroy_handler(second_handler);

    for (auto& t : threads) { t.join(); }

    ct_logger_close(&logger);
    REQUIRE(first.calls == static_cast<size_t>(kThreads * kRecords));
    REQUIRE(second.calls == 0);
}

TEST_CASE("log_logger_level_is_atomic", "[log]") {
    ct_logger_t logger;
    ct_logger_init(&logger);
    REQUIRE(ct_logger_start(&logger) == 0);

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

    ct_logger_close(&logger);
}
