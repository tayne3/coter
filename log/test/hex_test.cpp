#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

#include "coter/core/fs.h"
#include "coter/core/macro.h"
#include "coter/log/log.h"

#define logVN(...) CT_LOG_BASIC(VERBOSE, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define logDN(...) CT_LOG_BASIC(DEBUG, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define logTN(...) CT_LOG_BASIC(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define logWN(...) CT_LOG_BASIC(WARNING, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define logEN(...) CT_LOG_BASIC(ERROR, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define logFN(...) CT_LOG_BASIC(FATAL, CT_DEFAULT_LOGGER, __VA_ARGS__)

#define logVH(__buf, __len) CT_LOG_HEX(VERBOSE, CT_DEFAULT_LOGGER, __buf, __len)
#define logDH(__buf, __len) CT_LOG_HEX(DEBUG, CT_DEFAULT_LOGGER, __buf, __len)
#define logTH(__buf, __len) CT_LOG_HEX(TRACE, CT_DEFAULT_LOGGER, __buf, __len)
#define logWH(__buf, __len) CT_LOG_HEX(WARNING, CT_DEFAULT_LOGGER, __buf, __len)
#define logEH(__buf, __len) CT_LOG_HEX(ERROR, CT_DEFAULT_LOGGER, __buf, __len)
#define logFH(__buf, __len) CT_LOG_HEX(FATAL, CT_DEFAULT_LOGGER, __buf, __len)

namespace {
static const char* kOutputDir      = "test_log_out";
const char*        kWithLogFile    = "test_log_out/with_log.log0";
const char*        kWithoutLogFile = "test_log_out/without_log.log";

struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) std::fclose(f);
    }
};
using FilePtr = std::unique_ptr<FILE, FileDeleter>;

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

TEST_CASE("log_hex_and_long_text", "[log][hex]") {
    if (ct_access(kOutputDir, 0) == -1) { (void)ct_mkdir(kOutputDir); }
    REQUIRE(ct_access(kOutputDir, 0) == 0);

    SECTION("hex format comparison") {
        std::remove(kWithLogFile);
        std::remove(kWithoutLogFile);

        {
            FilePtr f(std::fopen(kWithoutLogFile, "wb"));
            REQUIRE(f != nullptr);
            const uint8_t buf[16] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,
                                     0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0, 0x01};
            for (int i = 0; i < 60000; ++i) {
                fprintf(f.get(),
                        "%02X %02X %02X %02X %02X %02X %02X %02X "
                        "%02X %02X %02X %02X %02X %02X %02X %02X\n",
                        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10],
                        buf[11], buf[12], buf[13], buf[14], buf[15]);
            }
        }

        {
            REQUIRE(ct_log_init(NULL) == 0);
            ct_logger_t logger;
            ct_logger_init(&logger);

            ct_log_file_handler_config_t file_config;
            ct_log_file_handler_config_default(&file_config);
            std::strncpy(file_config.dir, kOutputDir, sizeof(file_config.dir) - 1);
            std::strncpy(file_config.name, "with_log", sizeof(file_config.name) - 1);
            file_config.size_max  = 4 * 1024 * 1024;
            file_config.count_max = 3;
            REQUIRE(ct_logger_add_handler(&logger, ct_log_file_handler_create(&file_config)) == 0);
            ct_logger_register(&logger);
            ct_log_set_default(&logger);

            const uint8_t buf[16] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,
                                     0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0, 0x01};
            for (int i = 0; i < 10000; ++i) {
                logVH(buf, 16);
                logVN("\n");
                logDH(buf, 16);
                logDN("\n");
                logTH(buf, 16);
                logTN("\n");
                logWH(buf, 16);
                logWN("\n");
                logEH(buf, 16);
                logEN("\n");
                logFH(buf, 16);
                logFN("\n");
            }

            ct_log_close();
        }

        verify_files_identical(kWithLogFile, kWithoutLogFile);
    }

    SECTION("long text splitting") {
        std::remove(kWithLogFile);
        std::remove(kWithoutLogFile);

        const size_t         text_size = 2048;
        std::vector<uint8_t> long_text(text_size);
        for (size_t i = 0; i < text_size; ++i) { long_text[i] = static_cast<uint8_t>(i % 0xFE + 1); }

        {
            FilePtr f(std::fopen(kWithoutLogFile, "wb"));
            REQUIRE(f != nullptr);
            for (int i = 0; i < 60; ++i) {
                for (size_t j = 0; j < text_size; ++j) {
                    fprintf(f.get(), "%02X%c", long_text[j], (j == text_size - 1 ? '\n' : ' '));
                }
            }
        }

        {
            REQUIRE(ct_log_init(NULL) == 0);
            ct_logger_t logger;
            ct_logger_init(&logger);

            ct_log_file_handler_config_t file_config;
            ct_log_file_handler_config_default(&file_config);
            std::strncpy(file_config.dir, kOutputDir, sizeof(file_config.dir) - 1);
            std::strncpy(file_config.name, "with_log", sizeof(file_config.name) - 1);
            REQUIRE(ct_logger_add_handler(&logger, ct_log_file_handler_create(&file_config)) == 0);
            ct_logger_register(&logger);
            ct_log_set_default(&logger);

            for (int i = 0; i < 10; ++i) {
                logVH(long_text.data(), text_size);
                logVN("\n");
                logDH(long_text.data(), text_size);
                logDN("\n");
                logTH(long_text.data(), text_size);
                logTN("\n");
                logWH(long_text.data(), text_size);
                logWN("\n");
                logEH(long_text.data(), text_size);
                logEN("\n");
                logFH(long_text.data(), text_size);
                logFN("\n");
            }

            ct_log_close();
        }

        verify_files_identical(kWithLogFile, kWithoutLogFile);
    }

    std::remove(kWithLogFile);
    std::remove(kWithoutLogFile);
    (void)ct_rmdir(kOutputDir);
}
