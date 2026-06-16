#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"

TEST_CASE("log_system_ready_after_first_use", "[log][system]") {
    // 触发懒初始化
    ct_logger_t* def = ct_logger_get_default();
    REQUIRE(def != nullptr);

    // 系统就绪后，用户自定义 logger 可以正常启动，且日志路径端到端连通
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
    // close() 内部调用 dispatcher_sync(FLUSH)，保证上面的消息已经被处理
    REQUIRE(call_count == 1);
}

TEST_CASE("log_concurrent_start_close_stress", "[log][system]") {
    // 大量并发创建/启动/关闭 logger，验证 dispatcher 全局队列无竞态，
    // 并验证每条提交的日志均被处理（端到端连通）
    constexpr int kThreads = 8;
    constexpr int kIter    = 50;

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

TEST_CASE("log_seal_default_concurrent", "[log][system]") {
    // default logger 在首次 get_default() 调用后即被封印（全局一次性副作用）。
    // 本测试验证封印后并发调用 set_default() 全部失败的稳态行为。
    // 注意：封印可能已由进程内其他测试（如 logger_test）更早触发，这是预期的。
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

    // 封印后任何 set_default 均应失败
    REQUIRE(success_count == 0);
    REQUIRE(ct_logger_get_default() == sealed);
}
