#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "coter/log/log.h"

namespace {
struct safety_callback_state {
    std::atomic<size_t> calls{0};
    std::atomic<bool>   destroyed{false};
};

void safety_log_callback(const ct_log_record_t* record, void* userdata) {
    (void)record;
    auto* state = static_cast<safety_callback_state*>(userdata);
    state->calls++;
    // Simulate slow processing
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
}  // namespace

TEST_CASE("log_safety_lifecycle", "[log][safety]") {
    REQUIRE(ct_log_init(NULL) == 0);

    safety_callback_state state;

    // 1. Create a logger on heap
    ct_logger_t* logger = (ct_logger_t*)malloc(sizeof(ct_logger_t));
    ct_logger_init(logger);

    ct_log_callback_handler_config_t config;
    ct_log_callback_handler_config_default(&config);
    config.routine  = safety_log_callback;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(logger, ct_log_callback_handler_create(&config)) == 0);
    REQUIRE(ct_logger_register(logger) == 0);

    // Verify sealing logic: cannot add handler after registration
    REQUIRE(ct_logger_add_handler(logger, ct_log_callback_handler_create(&config)) == -1);

    std::atomic<bool> saboteur_done{false};

    // 2. Start a Saboteur Thread that constantly creates and destroys loggers
    std::thread saboteur([&]() {
        for (int i = 0; i < 50; ++i) {
            ct_logger_t* temp_logger = (ct_logger_t*)malloc(sizeof(ct_logger_t));
            ct_logger_init(temp_logger);

            ct_log_callback_handler_config_t temp_config;
            ct_log_callback_handler_config_default(&temp_config);
            temp_config.routine  = safety_log_callback;
            temp_config.userdata = &state;

            ct_logger_add_handler(temp_logger, ct_log_callback_handler_create(&temp_config));
            ct_logger_register(temp_logger);

            CT_LOG_BASIC(TRACE, temp_logger, "Sabotage message %d\n", i);

            std::this_thread::yield();
            ct_logger_close(temp_logger);
            free(temp_logger);
        }
        saboteur_done = true;
    });

    // 3. Fire many logs on main thread
    for (int i = 0; i < 100; ++i) { CT_LOG_BASIC(TRACE, logger, "Safety test message %d\n", i); }

    // 4. Immediately close the primary logger while logs are still in flight
    ct_logger_close(logger);

    saboteur.join();

    // 5. Wait for processing to finish
    ct_log_flush();

    REQUIRE(state.calls >= 150);

    free(logger);

    ct_log_close();
}
