/**
 * @file bench_submit.cpp
 * @brief submit 路径端到端延迟基准测试
 *
 * 测试场景：
 * A. 单线程：CT_LOGGER_INFO → handler 收到回调的端到端延迟（包含 flush 开销）
 * B. 4 线程：并发写入吞吐量（条/秒）
 * C. 对照组：纯回调函数调用的理论极限开销（无 logger 构造，无分发逻辑）
 */
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

#include "coter/core/time.h"
#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/nanobench.h"

namespace {

// 最简 callback：仅计数，不做任何 I/O，用于隔离并测量框架本身的纯粹开销
struct Counter {
    std::atomic<int> n{0};
    void             on_record(const ct_log_record_t*) { n.fetch_add(1, std::memory_order_relaxed); }
};

void counter_cb(const ct_log_record_t* r, void* ud) {
    static_cast<Counter*>(ud)->on_record(r);
}

ct_log_handler_t* make_record_handler(Counter* c) {
    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine  = counter_cb;
    cfg.userdata = c;
    return ct_log_record_handler_create(&cfg);
}

}  // namespace

int main() {
    std::printf("=== bench_submit ===\n\n");

    /* ---- A. 单线程端到端延迟 ---- */
    {
        Counter     c;
        ct_logger_t logger;
        ct_logger_init(&logger);
        ct_logger_add_handler(&logger, make_record_handler(&c));
        ct_logger_start(&logger);

        // 每轮测量：发送一条日志并强制 flush，确保完整端到端生命周期
        ankerl::nanobench::Bench().warmup(100).minEpochIterations(1000).run("A. single-thread end-to-end (w/ flush)",
                                                                            [&]() {
                                                                                CT_LOGGER_INFO(&logger, "bench");
                                                                                ct_logger_flush(&logger);
                                                                            });

        ct_logger_close(&logger);
    }

    /* ---- B. 4 线程并发吞吐量 ---- */
    {
        constexpr int kThreads   = 4;
        constexpr int kPerThread = 5000;

        Counter     c;
        ct_logger_t logger;
        ct_logger_init(&logger);
        ct_logger_add_handler(&logger, make_record_handler(&c));
        ct_logger_start(&logger);

        ankerl::nanobench::Bench().warmup(10).batch(kThreads * kPerThread).run("B. 4-thread throughput", [&]() {
            std::vector<std::thread> ts;
            ts.reserve(kThreads);
            for (int i = 0; i < kThreads; ++i) {
                ts.emplace_back([&, i]() {
                    for (int j = 0; j < kPerThread; ++j) { CT_LOGGER_INFO(&logger, "bench t=%d j=%d", i, j); }
                });
            }
            for (auto& t : ts) { t.join(); }
        });

        ct_logger_close(&logger);

        std::printf("   -> [Metrics] total logs processed=%d\n\n", c.n.load());
    }

    /* ---- C. 纯回调基线（无 logger） ---- */
    {
        Counter         c;
        ct_log_record_t rec{};
        rec.level = CT_LOG_LEVEL_INFO;
        rec.file  = __FILE__;
        rec.line  = __LINE__;
        rec.data  = "bench";
        rec.size  = 5;
        rec.time  = ct_gettimeofday_us() / 1000;
        rec.tid   = 0;

        ankerl::nanobench::Bench().warmup(100).minEpochIterations(5000).run("C. baseline (direct callback, no logger)",
                                                                            [&]() { counter_cb(&rec, &c); });
    }

    std::printf("\ndone.\n");
    return 0;
}
