#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/log/log.h"

#define test_basic_trace(...) CT_LOG_BASIC(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)

namespace {
static constexpr int kTestThreads    = 4;
static constexpr int kTestThreadData = 50000;
static const char*   kOutputDir      = "test_log_out";
static const char*   kWithLogFile    = "test_log_out/callback_with_log.log";
static const char*   kWithoutLogFile = "test_log_out/callback_without_log.log";

struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) std::fclose(f);
    }
};
using FilePtr = std::unique_ptr<FILE, FileDeleter>;

static std::atomic<bool> g_is_exit{false};
static FILE*             g_file_with_log = nullptr;

void thread_log_schedule() {
    while (!g_is_exit) {
        ct_log_schedule(ct_getuptime_ms());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void log_callback(const ct_log_record_t* record, void* userdata) {
    (void)userdata;
    if (record && record->data && g_file_with_log) { std::fwrite(record->data, 1, record->size, g_file_with_log); }
}

void thread_callback_with_log() {
    for (int i = 0; i < kTestThreadData; ++i) {
        test_basic_trace(
            "%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234,
            1234, 1234, 0xFFFF0000ULL, 0xFFFF0000ULL, 0xFFFF0000ULL, 0xFFFF0000ULL, "test1", "test2", "test3", "test4",
            0x00, 0x01, 0x02, 0x03);
    }
}

void thread_callback_without_log(FILE* target, std::mutex& mtx) {
    for (int i = 0; i < kTestThreadData; ++i) {
        char buffer[1024];
        int  size = std::snprintf(
            buffer, sizeof(buffer),
            "%04d/%05d/%06d/%07d %016llx/%016llx/%016llx/%016llx %10s/%11s/%12s/%13s %02x/%02x/%02x/%02x\n", 1234, 1234,
            1234, 1234, 0xFFFF0000ULL, 0xFFFF0000ULL, 0xFFFF0000ULL, 0xFFFF0000ULL, "test1", "test2", "test3", "test4",
            0x00, 0x01, 0x02, 0x03);
        std::lock_guard<std::mutex> lock(mtx);
        std::fwrite(buffer, 1, static_cast<size_t>(size), target);
    }
}

void verify_files_identical(const char* path1, const char* path2) {
    FilePtr f1(std::fopen(path1, "rb"));
    FilePtr f2(std::fopen(path2, "rb"));
    REQUIRE(f1 != nullptr);
    REQUIRE(f2 != nullptr);

    std::fseek(f1.get(), 0, SEEK_END);
    std::fseek(f2.get(), 0, SEEK_END);
    REQUIRE(std::ftell(f1.get()) == std::ftell(f2.get()));
    REQUIRE(std::ftell(f1.get()) > 0);

    std::rewind(f1.get());
    std::rewind(f2.get());

    std::vector<char> buf1(4096), buf2(4096);
    while (true) {
        size_t n1 = std::fread(buf1.data(), 1, buf1.size(), f1.get());
        size_t n2 = std::fread(buf2.data(), 1, buf2.size(), f2.get());
        REQUIRE(n1 == n2);
        if (n1 == 0) break;
        REQUIRE(std::memcmp(buf1.data(), buf2.data(), n1) == 0);
    }
}
}  // namespace

TEST_CASE("log_callback_integrity", "[log]") {
    if (ct_access(kOutputDir, 0) == -1) { (void)ct_mkdir(kOutputDir); }

    SECTION("verify callback output matches direct write") {
        std::mutex baseline_mtx;
        {
            FilePtr f(std::fopen(kWithoutLogFile, "wb"));
            REQUIRE(f != nullptr);
            std::vector<std::thread> threads;
            for (int i = 0; i < kTestThreads; ++i)
                threads.emplace_back(thread_callback_without_log, f.get(), std::ref(baseline_mtx));
            for (auto& t : threads) t.join();
        }

        {
            REQUIRE(ct_log_init() == 0);
            FilePtr f(std::fopen(kWithLogFile, "wb"));
            REQUIRE(f != nullptr);
            g_file_with_log = f.get();

            ct_logger_t logger;
            ct_logger_init(&logger);

            ct_log_callback_handler_config_t config;
            ct_log_callback_handler_config_default(&config);
            config.routine = log_callback;
            config.limit   = 100;  // test some buffering
            REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&config)) == 0);
            ct_logger_register(&logger);
            ct_log_set_default(&logger);

            g_is_exit = false;
            std::thread schedule_thread(thread_log_schedule);

            std::vector<std::thread> threads;
            for (int i = 0; i < kTestThreads; ++i) threads.emplace_back(thread_callback_with_log);
            for (auto& t : threads) t.join();

            g_is_exit = true;
            schedule_thread.join();
            ct_log_close();
            g_file_with_log = nullptr;
        }

        verify_files_identical(kWithLogFile, kWithoutLogFile);
    }

    std::remove(kWithLogFile);
    std::remove(kWithoutLogFile);
    (void)ct_rmdir(kOutputDir);
}
