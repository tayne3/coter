#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/log/log.h"

#define test_basic_trace(...) CT_LOG_BASIC(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)

namespace {
static constexpr int kTestThreads    = 4;
static constexpr int kTestThreadData = 10000;
static const char*   kOutputDir      = "test_log_out";
static const char*   kWithLogFile    = "test_log_out/with_log.log0";
static const char*   kWithoutLogFile = "test_log_out/without_log.log";

// RAII File closer
struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) std::fclose(f);
    }
};
using FilePtr = std::unique_ptr<FILE, FileDeleter>;

void thread_write_with_log() {
    for (int i = 0; i < kTestThreadData; ++i) {
        test_basic_trace(
            "%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234,
            1234, 1234, (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL,
            (unsigned long long)0xFFFF0000ULL, (unsigned long long)0xFFFF0000ULL, "test1", "test2", "test3", "test4",
            0x00, 0x01, 0x02, 0x03);
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

void verify_files_identical(const char* path1, const char* path2) {
    FilePtr f1(std::fopen(path1, "rb"));
    FilePtr f2(std::fopen(path2, "rb"));

    REQUIRE(f1 != nullptr);
    REQUIRE(f2 != nullptr);

    REQUIRE(std::fseek(f1.get(), 0, SEEK_END) == 0);
    REQUIRE(std::fseek(f2.get(), 0, SEEK_END) == 0);

    const auto size1 = std::ftell(f1.get());
    const auto size2 = std::ftell(f2.get());

    REQUIRE(size1 > 0);
    REQUIRE(size1 == size2);

    REQUIRE(std::fseek(f1.get(), 0, SEEK_SET) == 0);
    REQUIRE(std::fseek(f2.get(), 0, SEEK_SET) == 0);

    std::vector<char> buf1(4096);
    std::vector<char> buf2(4096);

    while (true) {
        size_t n1 = std::fread(buf1.data(), 1, buf1.size(), f1.get());
        size_t n2 = std::fread(buf2.data(), 1, buf2.size(), f2.get());

        REQUIRE(n1 == n2);
        if (n1 == 0) {
            REQUIRE(std::feof(f1.get()));
            REQUIRE(std::feof(f2.get()));
            break;
        }
        REQUIRE(std::memcmp(buf1.data(), buf2.data(), n1) == 0);
    }
}
}  // namespace

TEST_CASE("log_write_performance", "[log][perf]") {
    if (ct_access(kOutputDir, 0) == -1) { (void)ct_mkdir(kOutputDir); }
    REQUIRE(ct_access(kOutputDir, 0) == 0);

    SECTION("compare baseline vs logger") {
        std::remove(kWithLogFile);

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
            REQUIRE(ct_log_init(NULL) == 0);
            ct_logger_t logger;
            ct_logger_init(&logger);

            ct_log_file_handler_config_t file_config;
            ct_log_file_handler_config_default(&file_config);
            std::strncpy(file_config.dir, kOutputDir, sizeof(file_config.dir) - 1);
            std::strncpy(file_config.name, "with_log", sizeof(file_config.name) - 1);
            file_config.size_max  = 1024 * 1024 * 1024;
            file_config.count_max = 1;
            REQUIRE(ct_logger_add_handler(&logger, ct_log_file_handler_create(&file_config)) == 0);
            ct_logger_register(&logger);
            ct_log_set_default(&logger);

            auto                     start = ct_getuptime_ms();
            std::vector<std::thread> threads;
            for (int i = 0; i < kTestThreads; ++i) { threads.emplace_back(thread_write_with_log); }
            for (auto& t : threads) { t.join(); }
            ct_log_flush();
            time_with_log = static_cast<int>(ct_getuptime_ms() - start);

            ct_log_close();
        }

        CAPTURE(time_with_log, time_without_log);

        {
            char buf[1024];
            snprintf(buf, sizeof(buf), "Performance: with log %d ms, without log %d ms\n", time_with_log,
                     time_without_log);
            INFO(buf);
        }

        // 3. Validation
        verify_files_identical(kWithLogFile, kWithoutLogFile);

        // 4. Cleanup
        std::remove(kWithLogFile);
        std::remove(kWithoutLogFile);
        (void)ct_rmdir(kOutputDir);
    }
}
