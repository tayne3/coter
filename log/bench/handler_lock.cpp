/**
 * @file bench_handler_lock.cpp
 * @brief 同步 handler 加锁代价基准测试
 *
 * 测试场景与目的：
 * A/C: 单线程基线（Console/File）
 * B/D: 4 线程并发，用于测量锁竞争导致的性能损耗
 * E:   异步 handler 对照组
 */
#include <cstdio>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/core/strings.h"
#include "coter/log/handler/async.h"
#include "coter/log/handler/console.h"
#include "coter/log/handler/file.h"
#include "coter/log/log.h"
#include "coter/testing/nanobench.h"

namespace {

constexpr uint64_t kWarmup     = 200;
constexpr uint64_t kIterations = 2000;
constexpr int      kThreads    = 4;
const char*        kBenchDir   = "bench_handler_lock_out";
const char*        kBenchFile  = "bench_handler_lock_out/bench.log0";

// 仅负责多线程并发写入日志，耗时与吞吐量(OPS)计算交由 nanobench 在外部接管
void run_concurrent(ct_logger_t* logger, int threads, int per_thread) {
    std::vector<std::thread> ts;
    ts.reserve(threads);
    for (int i = 0; i < threads; ++i) {
        ts.emplace_back([&, i]() {
            for (int j = 0; j < per_thread; ++j) { CT_LOGGER_INFO(logger, "bench t=%d j=%d", i, j); }
        });
    }
    for (auto& t : ts) { t.join(); }
}

}  // namespace

int main() {
    std::printf("=== bench_handler_lock ===\n\n");

    if (ct_access(kBenchDir, 0) == -1) { ct_mkdir(kBenchDir); }

    {
        ct_logger_t logger;
        ct_logger_init(&logger);
        ct_log_console_handler_config_t cfg;
        ct_log_console_handler_config_default(&cfg);
        cfg.stream = stderr;
        ct_logger_add_handler(&logger, ct_log_console_handler_create(&cfg));
        ct_logger_start(&logger);

        // 单线程场景：直接配置预热和迭代次数
        ankerl::nanobench::Bench().warmup(kWarmup).minEpochIterations(kIterations).run("A. console 1-thread", [&]() {
            CT_LOGGER_INFO(&logger, "bench");
        });

        ct_logger_close(&logger);
    }

    {
        ct_logger_t logger;
        ct_logger_init(&logger);
        ct_log_console_handler_config_t cfg;
        ct_log_console_handler_config_default(&cfg);
        cfg.stream = stderr;
        ct_logger_add_handler(&logger, ct_log_console_handler_create(&cfg));
        ct_logger_start(&logger);

        // 多线程场景：使用 .batch() 声明单次 run 包含的实际操作总数，以正确计算全局 OPS
        ankerl::nanobench::Bench().warmup(10).batch(kThreads * kIterations).run("B. console 4-thread", [&]() {
            run_concurrent(&logger, kThreads, kIterations);
        });

        ct_logger_close(&logger);
    }

    {
        ct_logger_t logger;
        ct_logger_init(&logger);
        ct_log_file_handler_config_t cfg;
        ct_log_file_handler_config_default(&cfg);
        ct_snprintf_s(cfg.dir, sizeof(cfg.dir), "%s", kBenchDir);
        ct_snprintf_s(cfg.name, sizeof(cfg.name), "%s", "bench");
        cfg.size_max  = 512UL * 1024 * 1024;
        cfg.count_max = 1;
        ct_logger_add_handler(&logger, ct_log_file_handler_create(&cfg));
        ct_logger_start(&logger);

        ankerl::nanobench::Bench().warmup(kWarmup).minEpochIterations(kIterations).run("C. file 1-thread", [&]() {
            CT_LOGGER_INFO(&logger, "bench");
        });

        ct_logger_close(&logger);
    }

    {
        ct_logger_t logger;
        ct_logger_init(&logger);
        ct_log_file_handler_config_t cfg;
        ct_log_file_handler_config_default(&cfg);
        ct_snprintf_s(cfg.dir, sizeof(cfg.dir), "%s", kBenchDir);
        ct_snprintf_s(cfg.name, sizeof(cfg.name), "%s", "bench");
        cfg.size_max  = 512UL * 1024 * 1024;
        cfg.count_max = 1;
        ct_logger_add_handler(&logger, ct_log_file_handler_create(&cfg));
        ct_logger_start(&logger);

        ankerl::nanobench::Bench().warmup(10).batch(kThreads * kIterations).run("D. file 4-thread", [&]() {
            run_concurrent(&logger, kThreads, kIterations);
        });

        ct_logger_close(&logger);
    }

    {
        ct_logger_t logger;
        ct_logger_init(&logger);

        ct_log_console_handler_config_t ccfg;
        ct_log_console_handler_config_default(&ccfg);
        ccfg.stream             = stderr;
        ct_log_handler_t* inner = ct_log_console_handler_create(&ccfg);

        ct_log_async_handler_config_t acfg;
        ct_log_async_handler_config_default(&acfg);
        acfg.inner           = inner;
        acfg.overflow_policy = CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW;
        acfg.queue_size      = 512;
        ct_logger_add_handler(&logger, ct_log_async_handler_create(&acfg));
        ct_logger_start(&logger);

        ankerl::nanobench::Bench().warmup(10).batch(kThreads * kIterations).run("E. async(console) 4-thread", [&]() {
            run_concurrent(&logger, kThreads, kIterations);
        });

        ct_logger_close(&logger);
    }

    std::remove(kBenchFile);
    ct_rmdir(kBenchDir);

    std::printf("\n[Note] If the ratio of B/A < 0.5 or D/C < 0.5,\n"
                "       the sync handler in path C MUST be wrapped in async_handler.\n");
    return 0;
}
