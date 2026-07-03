/**
 * @file system_test.cpp
 * @brief 系统级集成与端到端测试
 *
 * 覆盖：
 *  - 默认 logger 初始化后的系统就绪状态验证
 *  - 高并发下海量 logger 实例的生命周期（创建、启动、日志提交、关闭）压力测试
 *  - 默认 logger 封印机制的并发安全性验证
 */
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("system is ready and end-to-end logging works after default logger initialization") {
    ct_logger_t* def = ct_logger_get_default();
    REQUIRE(def != nullptr);

    std::atomic<size_t> call_count{0};

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = [](const ct_log_record_t*, void* ud) { ++(*static_cast<std::atomic<size_t>*>(ud)); };
    config.userdata = &call_count;

    ct_logger_t logger;
    ct_logger_init(&logger);
    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    CT_LOGGER_INFO(&logger, "system ready test");

    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(call_count == 1);
}

TEST_CASE("concurrent logger creation, starting, and closing works reliably without dropping logs") {
    constexpr int kThreads = 16;
    constexpr int kIter    = 100;

    std::atomic<int>    errors{0};
    std::atomic<size_t> total_received{0};
    std::atomic<size_t> total_sent{0};

    std::vector<std::thread> workers;
    for (int t = 0; t < kThreads; ++t) {
        workers.emplace_back([&]() {
            for (int i = 0; i < kIter; ++i) {
                std::atomic<size_t> received{0};

                ct_log_record_handler_config_t config;
                ct_log_record_handler_config_default(&config);
                config.routine  = [](const ct_log_record_t*, void* ud) { ++(*static_cast<std::atomic<size_t>*>(ud)); };
                config.userdata = &received;

                ct_logger_t logger;
                ct_logger_init(&logger);

                if (ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) != 0) {
                    ++errors;
                    continue;
                }
                if (ct_logger_start(&logger) != 0) {
                    ++errors;
                    continue;
                }

                CT_LOGGER_DEBUG(&logger, "stress %d", i);
                ++total_sent;

                if (ct_logger_close(&logger) != 0) {
                    ++errors;
                    continue;
                }
                // close() 保证 flush，此时 received 必须等于 1
                if (received != 1) { ++errors; }
                total_received += received;
            }
        });
    }

    for (auto& w : workers) { w.join(); }
    REQUIRE(errors == 0);
    REQUIRE(total_received == total_sent);
}

TEST_CASE("concurrent attempts to set the default logger fail safely after it is sealed") {
    ct_logger_t* sealed = ct_logger_get_default();
    REQUIRE(sealed != nullptr);

    constexpr int            kThreads = 16;
    std::atomic<int>         success_count{0};
    std::vector<std::thread> workers;

    for (int t = 0; t < kThreads; ++t) {
        workers.emplace_back([&]() {
            ct_logger_t logger;
            ct_logger_init(&logger);
            ct_logger_start(&logger);

            if (ct_logger_set_default(&logger) == 0) { ++success_count; }

            ct_logger_close(&logger);
        });
    }

    for (auto& w : workers) { w.join(); }

    REQUIRE(success_count == 0);
    REQUIRE(ct_logger_get_default() == sealed);
}

TEST_SUITE_END();
