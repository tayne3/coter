#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"

namespace {
constexpr int kWorkerThreads = 4;

void chaos_callback(const ct_log_record_t* record, void* userdata) {
    (void)record;
    auto* count = static_cast<std::atomic<int>*>(userdata);
    ++(*count);
}
}  // namespace

TEST_CASE("log_chaos_lifecycle", "[log][chaos]") {
    std::atomic<bool> stop{false};
    std::atomic<int>  error_count{0};
    std::atomic<int>  callback_count{0};

    ct_logger_t* default_logger = ct_logger_get_default();
    const int    default_level  = ct_logger_get_level(default_logger);
    ct_logger_set_level(default_logger, CT_LOG_LEVEL_FATAL);

    std::vector<std::thread> workers;
    for (int i = 0; i < kWorkerThreads; ++i) {
        workers.emplace_back([&, i]() {
            while (!stop) {
                CT_TRACE("Chaos message from worker %d", i);

                ct_logger_t* temp_logger = (ct_logger_t*)malloc(sizeof(ct_logger_t));
                if (!temp_logger) {
                    ++error_count;
                    break;
                }
                ct_logger_init(temp_logger);

                ct_log_record_handler_config_t temp_config;
                ct_log_record_handler_config_default(&temp_config);
                temp_config.routine  = chaos_callback;
                temp_config.userdata = &callback_count;

                ct_log_handler_t* handler = ct_log_record_handler_create(&temp_config);
                if (!handler || ct_logger_add_handler(temp_logger, handler) != 0) {
                    ++error_count;
                    ct_log_handler_destroy(handler);
                    free(temp_logger);
                    continue;
                }

                if (ct_logger_start(temp_logger) != 0) {
                    ++error_count;
                    if (ct_logger_close(temp_logger) != 0) { ++error_count; }
                    free(temp_logger);
                    continue;
                }

                ct_log_handler_t* late_handler = ct_log_record_handler_create(&temp_config);
                if (!late_handler || ct_logger_add_handler(temp_logger, late_handler) != -1) { ++error_count; }
                ct_log_handler_destroy(late_handler);

                CT_LOGGER_TRACE(temp_logger, "temp message from worker %d", i);
                if (ct_logger_close(temp_logger) != 0) { ++error_count; }
                free(temp_logger);
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
    stop = true;
    for (auto& t : workers) { t.join(); }
    ct_logger_set_level(default_logger, default_level);

    REQUIRE(error_count == 0);
    REQUIRE(callback_count > 0);
}
