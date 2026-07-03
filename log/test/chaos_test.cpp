/**
 * @file chaos_test.cpp
 * @brief 极端条件下的生命周期混沌测试
 *
 * 覆盖：
 *  - 高并发下的随意创建、挂载、启动、关闭 logger 实例
 *  - 验证极端竞态条件下的稳定性（不崩溃、不死锁）
 */
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("logger withstands chaotic concurrent lifecycle mutations without crashing or deadlocking") {
    constexpr int kWorkerThreads = 16;

    struct State {
        std::atomic<bool> stop{false};
        std::atomic<int>  error_count{0};
        std::atomic<int>  callback_count{0};
    } state;

    ct_logger_t* default_logger = ct_logger_get_default();
    const int    default_level  = ct_logger_get_level(default_logger);
    ct_logger_set_level(default_logger, CT_LOG_LEVEL_FATAL);

    std::vector<std::thread> workers;
    for (int i = 0; i < kWorkerThreads; ++i) {
        workers.emplace_back([&, i]() {
            while (!state.stop) {
                CT_TRACE("Chaos message from worker %d", i);

                ct_logger_t* temp_logger = (ct_logger_t*)std::malloc(sizeof(ct_logger_t));
                if (!temp_logger) {
                    ++state.error_count;
                    break;
                }
                ct_logger_init(temp_logger);

                ct_log_record_handler_config_t temp_config;
                ct_log_record_handler_config_default(&temp_config);
                temp_config.routine = [](const ct_log_record_t*, void* userdata) {
                    auto* s = static_cast<State*>(userdata);
                    ++s->callback_count;
                };
                temp_config.userdata = &state;

                ct_log_handler_t* handler = ct_log_record_handler_create(&temp_config);
                if (!handler || ct_logger_add_handler(temp_logger, handler) != 0) {
                    ++state.error_count;
                    if (handler) ct_log_handler_destroy(handler);
                    std::free(temp_logger);
                    continue;
                }

                if (ct_logger_start(temp_logger) != 0) {
                    ++state.error_count;
                    if (ct_logger_close(temp_logger) != 0) { ++state.error_count; }
                    std::free(temp_logger);
                    continue;
                }

                ct_log_handler_t* late_handler = ct_log_record_handler_create(&temp_config);
                if (!late_handler || ct_logger_add_handler(temp_logger, late_handler) != -1) { ++state.error_count; }
                if (late_handler) ct_log_handler_destroy(late_handler);

                CT_LOGGER_TRACE(temp_logger, "temp message from worker %d", i);
                if (ct_logger_close(temp_logger) != 0) { ++state.error_count; }
                std::free(temp_logger);
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
    state.stop = true;
    for (auto& t : workers) { t.join(); }
    ct_logger_set_level(default_logger, default_level);

    REQUIRE(state.error_count == 0);
    REQUIRE(state.callback_count > 0);
}

TEST_SUITE_END();
