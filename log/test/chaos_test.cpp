#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/log/log.h"

TEST_CASE("log_chaos_lifecycle", "[log][chaos]") {
    REQUIRE(ct_log_init(NULL) == 0);
    std::atomic<bool> stop{false};
    constexpr int     kWorkerThreads = 4;

    // Multiple threads constantly logging and creating/destroying custom loggers
    std::vector<std::thread> workers;
    for (int i = 0; i < kWorkerThreads; ++i) {
        workers.emplace_back([&, i]() {
            while (!stop) {
                // Logging to default logger
                CT_LOG_BASIC(TRACE, CT_DEFAULT_LOGGER, "Chaos message from worker %d\n", i);

                // Sabotage: Test custom logger lifecycle within chaos
                ct_logger_t* temp_logger = (ct_logger_t*)malloc(sizeof(ct_logger_t));
                ct_logger_init(temp_logger);

                ct_log_console_handler_config_t temp_config;
                ct_log_console_handler_config_default(&temp_config);

                ct_logger_add_handler(temp_logger, ct_log_console_handler_create(&temp_config));
                ct_logger_register(temp_logger);

                CT_LOG_BASIC(TRACE, temp_logger, "Sabotage message %d\n", i);

                std::this_thread::yield();
                ct_logger_close(temp_logger);
                free(temp_logger);
            }
        });
    }

    // Run for 3 seconds to ensure enough context switches
    std::this_thread::sleep_for(std::chrono::seconds(3));

    stop = true;
    for (auto& t : workers) { t.join(); }

    // Final state should be closed, but let's ensure it's clean
    ct_log_close();
    REQUIRE(true);
}
