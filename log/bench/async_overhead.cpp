/**
 * @file bench_async_overhead.cpp
 * @brief async_handler 层开销基准测试
 *
 * 测试场景：
 * A. async(record) block 策略：入队延迟（队列未满时）
 * B. async(record) discard_new 策略：入队延迟
 * C. async(record) overrun_oldest 策略：入队延迟
 * D. 4 线程并发，模拟慢速 inner handler (1ms/条)，对比各策略吞吐特征与丢包率
 */
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

#include "coter/log/handler/async.h"
#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/nanobench.h"

namespace {

// 模拟慢速写入的计数器
struct Counter {
    std::atomic<int>          n{0};
    std::chrono::microseconds slow_delay{0};

    void on_record(const ct_log_record_t*) {
        n.fetch_add(1, std::memory_order_relaxed);
        if (slow_delay.count() > 0) { std::this_thread::sleep_for(slow_delay); }
    }
};

void counter_cb(const ct_log_record_t* r, void* ud) {
    static_cast<Counter*>(ud)->on_record(r);
}

ct_log_handler_t* make_inner(Counter* c) {
    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine  = counter_cb;
    cfg.userdata = c;
    return ct_log_record_handler_create(&cfg);
}

// 封装初始化与清理逻辑
struct Setup {
    ct_logger_t logger;
    Counter     counter;

    void init(ct_log_async_overflow_policy_t policy, int queue_size,
              std::chrono::microseconds slow = std::chrono::microseconds{0}) {
        counter.slow_delay = slow;
        ct_logger_init(&logger);

        ct_log_async_handler_config_t acfg;
        ct_log_async_handler_config_default(&acfg);
        acfg.inner           = make_inner(&counter);
        acfg.overflow_policy = policy;
        acfg.queue_size      = queue_size;

        ct_logger_add_handler(&logger, ct_log_async_handler_create(&acfg));
        ct_logger_start(&logger);
    }

    void close() { ct_logger_close(&logger); }
};

}  // namespace

int main() {
    std::printf("=== bench_async_overhead ===\n\n");

    constexpr int      kQueueSize  = 256;
    constexpr uint64_t kWarmup     = 50;
    constexpr uint64_t kIterations = 1000;

    /* ---- A. block 策略：单线程入队延迟 ---- */
    {
        Setup s;
        s.init(CT_LOG_ASYNC_OVERFLOW_BLOCK, kQueueSize);

        ankerl::nanobench::Bench()
            .warmup(kWarmup)
            .minEpochIterations(kIterations)
            .run("A. async(block) enqueue latency 1-thread", [&]() { CT_LOGGER_INFO(&s.logger, "bench"); });

        s.close();
    }

    /* ---- B. discard_new 策略：单线程入队延迟 ---- */
    {
        Setup s;
        s.init(CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW, kQueueSize);

        ankerl::nanobench::Bench()
            .warmup(kWarmup)
            .minEpochIterations(kIterations)
            .run("B. async(discard_new) enqueue latency 1-thread", [&]() { CT_LOGGER_INFO(&s.logger, "bench"); });

        s.close();
    }

    /* ---- C. overrun_oldest 策略：单线程入队延迟 ---- */
    {
        Setup s;
        s.init(CT_LOG_ASYNC_OVERFLOW_OVERRUN, kQueueSize);

        ankerl::nanobench::Bench()
            .warmup(kWarmup)
            .minEpochIterations(kIterations)
            .run("C. async(overrun) enqueue latency 1-thread", [&]() { CT_LOGGER_INFO(&s.logger, "bench"); });

        s.close();
    }

    std::printf("\n");

    /* ---- D. 4 线程高负载，模拟慢速 inner (1ms/条) ---- */
    constexpr int kThreads   = 4;
    constexpr int kPerThread = 200;

    auto run_slow = [&](const char* label, ct_log_async_overflow_policy_t policy) {
        std::atomic<int> total_received{0};
        int              runs = 0;

        ankerl::nanobench::Bench().warmup(1).minEpochIterations(3).batch(kThreads * kPerThread).run(label, [&]() {
            Setup s;
            s.init(policy, kQueueSize, std::chrono::microseconds{1000});

            std::vector<std::thread> ts;
            ts.reserve(kThreads);
            for (int i = 0; i < kThreads; ++i) {
                ts.emplace_back([&, i]() {
                    for (int j = 0; j < kPerThread; ++j) { CT_LOGGER_INFO(&s.logger, "bench t=%d j=%d", i, j); }
                });
            }

            for (auto& t : ts) { t.join(); }

            // 确保队列落盘，测算完整的阻塞耗时
            s.close();

            total_received += s.counter.n.load();
            runs++;
        });

        int    total_sent = runs * kThreads * kPerThread;
        double drop_rate  = 100.0 * (total_sent - total_received.load()) / total_sent;
        std::printf("   -> [Metrics] sent=%d, received=%d, drop=%.1f%%\n\n", total_sent, total_received.load(),
                    drop_rate);
    };

    run_slow("D. slow-inner block     (4t x 200, 1ms/rec)", CT_LOG_ASYNC_OVERFLOW_BLOCK);
    run_slow("D. slow-inner discard   (4t x 200, 1ms/rec)", CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW);
    run_slow("D. slow-inner overrun   (4t x 200, 1ms/rec)", CT_LOG_ASYNC_OVERFLOW_OVERRUN);

    std::printf("\ndone.\n");
    return 0;
}
