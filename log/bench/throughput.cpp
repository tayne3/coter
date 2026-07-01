/**
 * @file bench_throughput.cpp
 * @brief 吞吐量基准测试：4 线程并发写入，有/无 logger 性能对比
 */
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/core/strings.h"
#include "coter/log/handler/file.h"
#include "coter/log/log.h"
#include "coter/testing/nanobench.h"

namespace {

constexpr int kThreads        = 4;
constexpr int kPerThread      = 10000;
const char*   kOutputDir      = "bench_log_out";
const char*   kWithLogFile    = "bench_log_out/with_log.log0";
const char*   kWithoutLogFile = "bench_log_out/without_log.log";

const char* kPayloadFmt = "%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x";

struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) std::fclose(f);
    }
};
using FilePtr = std::unique_ptr<FILE, FileDeleter>;

void write_with_logger(ct_logger_t* logger) {
    for (int i = 0; i < kPerThread; ++i) {
        CT_LOGGER_TRACE(logger, kPayloadFmt, 1234, 1234, 1234, 1234, (unsigned long long)0xFFFF0000ULL,
                        (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL,
                        (unsigned long long)0xFFFF0000ULL, "test1", "test2", "test3", "test4", 0x00, 0x01, 0x02, 0x03);
    }
}

void write_without_logger(FILE* f) {
    for (int i = 0; i < kPerThread; ++i) { std::fprintf(f, "%s\n", "bench_payload_line_without_logger"); }
}

}  // namespace

int main() {
    std::printf("=== bench_throughput: %d threads x %d records ===\n\n", kThreads, kPerThread);

    if (ct_access(kOutputDir, 0) == -1) { ct_mkdir(kOutputDir); }

    // 使用同一个 Bench 实例并开启 relative(true)
    // 这样 nanobench 会自动把第一个 run 作为 100% 基线，后续测试直接在表格中输出性能百分比
    ankerl::nanobench::Bench bench;
    bench
        .warmup(3)  // 纯 I/O 写盘较重，降低预热和迭代次数避免落盘过大
        .minEpochIterations(5)
        .batch(kThreads * kPerThread)
        .relative(true);

    /* --- 1. 无 logger 基线 --- */
    {
        FilePtr f(std::fopen(kWithoutLogFile, "wb"));
        if (!f) {
            std::fprintf(stderr, "cannot open %s\n", kWithoutLogFile);
            return 1;
        }

        bench.run("1. baseline (no logger, fprintf)", [&]() {
            std::vector<std::thread> threads;
            threads.reserve(kThreads);
            for (int i = 0; i < kThreads; ++i) { threads.emplace_back(write_without_logger, f.get()); }
            for (auto& t : threads) { t.join(); }
        });
    }

    /* --- 2. 有 logger（file handler） --- */
    {
        ct_logger_t logger;
        ct_logger_init(&logger);

        ct_log_file_handler_config_t cfg;
        ct_log_file_handler_config_default(&cfg);
        ct_snprintf_s(cfg.dir, sizeof(cfg.dir), "%s", kOutputDir);
        ct_snprintf_s(cfg.name, sizeof(cfg.name), "%s", "with_log");
        cfg.size_max  = 1024UL * 1024 * 1024;
        cfg.count_max = 1;

        ct_logger_add_handler(&logger, ct_log_file_handler_create(&cfg));
        ct_logger_start(&logger);

        bench.run("2. with logger (file handler)", [&]() {
            std::vector<std::thread> threads;
            threads.reserve(kThreads);
            for (int i = 0; i < kThreads; ++i) { threads.emplace_back(write_with_logger, &logger); }
            for (auto& t : threads) { t.join(); }
        });

        ct_logger_close(&logger);
    }

    std::remove(kWithLogFile);
    std::remove(kWithoutLogFile);
    ct_rmdir(kOutputDir);

    std::printf("\ndone.\n");
    return 0;
}
