#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/core/strings.h"
#include "coter/log/handler/file.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

namespace {
static constexpr int kTestThreads    = 4;
static constexpr int kTestThreadData = 10000;
static const char*   kOutputDir      = "test_log_out";
static const char*   kWithLogFile    = "test_log_out/with_log.log0";
static const char*   kWithoutLogFile = "test_log_out/without_log.log";

struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) std::fclose(f);
    }
};
using FilePtr = std::unique_ptr<FILE, FileDeleter>;

void thread_write_with_log(ct_logger_t* logger) {
    for (int i = 0; i < kTestThreadData; ++i) {
        CT_LOGGER_TRACE(logger,
                        "%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s "
                        "%02x/%02x/%02x/%02x",
                        1234, 1234, 1234, 1234, (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL,
                        (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL, "test1", "test2", "test3",
                        "test4", 0x00, 0x01, 0x02, 0x03);
    }
}

void thread_write_without_log(FILE* target) {
    REQUIRE(target != nullptr);
    for (int i = 0; i < kTestThreadData; ++i) {
        fprintf(target, "%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n",
                1234, 1234, 1234, 1234, (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL,
                (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL, "test1", "test2", "test3",
                "test4", 0x00, 0x01, 0x02, 0x03);
    }
}
}  // namespace

TEST_CASE("log_write_performance") {
    if (ct_access(kOutputDir, 0) == -1) { (void)ct_mkdir(kOutputDir); }
    REQUIRE(ct_access(kOutputDir, 0) == 0);

    SUBCASE("compare baseline vs logger") {
        // 1. Without Logger (Baseline)
        int time_without_log = 0;
        {
            FilePtr baseline_file(std::fopen(kWithoutLogFile, "wb"));
            REQUIRE(baseline_file != nullptr);

            auto                     start = ct_getuptime_ms();
            std::vector<std::thread> threads;
            for (int i = 0; i < kTestThreads; ++i) {
                threads.emplace_back(thread_write_without_log, baseline_file.get());
            }
            for (auto& t : threads) t.join();
            time_without_log = static_cast<int>(ct_getuptime_ms() - start);
        }

        // 2. With Logger
        int time_with_log = 0;
        {
            ct_logger_t logger;
            ct_logger_init(&logger);

            ct_log_file_handler_config_t file_config;
            ct_log_file_handler_config_default(&file_config);
            std::strncpy(file_config.dir, kOutputDir, sizeof(file_config.dir) - 1);
            std::strncpy(file_config.name, "with_log", sizeof(file_config.name) - 1);
            file_config.size_max  = 1024 * 1024 * 1024;
            file_config.count_max = 1;
            REQUIRE(ct_logger_add_handler(&logger, ct_log_file_handler_create(&file_config)) == 0);
            REQUIRE(ct_logger_start(&logger) == 0);

            auto                     start = ct_getuptime_ms();
            std::vector<std::thread> threads;
            for (int i = 0; i < kTestThreads; ++i) { threads.emplace_back(thread_write_with_log, &logger); }
            for (auto& t : threads) { t.join(); }
            ct_logger_close(&logger);
            time_with_log = static_cast<int>(ct_getuptime_ms() - start);
        }

        CAPTURE(time_with_log);
        CAPTURE(time_without_log);

        {
            char buf[256];
            ct_snprintf_s(buf, sizeof(buf), "Performance: with log %d ms, without log %d ms\n", time_with_log,
                          time_without_log);
            INFO(buf);
        }

        REQUIRE(true);

        // 3. Cleanup
        std::remove(kWithLogFile);
        std::remove(kWithoutLogFile);
        (void)ct_rmdir(kOutputDir);
    }
}
