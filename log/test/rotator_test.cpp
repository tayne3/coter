/**
 * @file rotator_test.cpp
 * @brief ct_log_rotator_t 测试
 *
 * 覆盖：
 *  - 日志文件大小超过限制时正确轮转
 *  - 存在旧文件时正确追加或覆写
 *  - 无效配置的拒绝
 *  - 多次轮转时的循环覆盖逻辑
 */
#include "../src/handler/rotator.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include "coter/core/fs.h"
#include "coter/core/strings.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("rotator handles file size boundaries and appending to existing files") {
    constexpr const char* kDir = "test_log_rotator_out";

    struct Fixture {
        Fixture() { cleanup(); }
        ~Fixture() { cleanup(); }

        void cleanup() const {
            ct_remove("test_log_rotator_out/rotate.log0");
            ct_remove("test_log_rotator_out/rotate.log1");
            ct_remove("test_log_rotator_out/append.log0");
            ct_rmdir("test_log_rotator_out");
        }

        std::string read_file(const char* path) const {
            FILE* file = std::fopen(path, "rb");
            if (!file) return "";
            std::string data;
            char        buffer[64];
            while (true) {
                size_t n = std::fread(buffer, 1, sizeof(buffer), file);
                if (n == 0) break;
                data.append(buffer, n);
            }
            std::fclose(file);
            return data;
        }
    } fixture;

    struct RotatorDeleter {
        void operator()(ct_log_rotator_t* r) const {
            if (r) ct_log_rotator_destroy(r);
        }
    };
    using RotatorPtr = std::unique_ptr<ct_log_rotator_t, RotatorDeleter>;

    SUBCASE("rotates across bounded files when writing beyond maximum size") {
        ct_log_rotator_config_t config = {};
        std::strncpy(config.dir, kDir, sizeof(config.dir) - 1);
        std::strncpy(config.name, "rotate", sizeof(config.name) - 1);
        config.size_max  = 5;
        config.count_max = 2;

        RotatorPtr rotator(ct_log_rotator_create(&config));
        REQUIRE(rotator != nullptr);
        REQUIRE(ct_log_rotator_write(rotator.get(), "abcdefghijkl", 12) == 12);
        REQUIRE(ct_log_rotator_index(rotator.get()) == 0);

        ct_log_rotator_flush(rotator.get());

        // kl goes to index 0, fghij goes to index 1, abcde was dropped (pushed out)
        REQUIRE(fixture.read_file("test_log_rotator_out/rotate.log0") == "kl");
        REQUIRE(fixture.read_file("test_log_rotator_out/rotate.log1") == "fghij");
    }

    SUBCASE("appends to the newest file when space is available") {
        REQUIRE(ct_mkdir(kDir) == 0);
        {
            FILE* file = std::fopen("test_log_rotator_out/append.log0", "wb");
            REQUIRE(file != nullptr);
            REQUIRE(std::fwrite("abc", 1, 3, file) == 3);
            std::fclose(file);
        }

        ct_log_rotator_config_t config = {};
        std::strncpy(config.dir, kDir, sizeof(config.dir) - 1);
        std::strncpy(config.name, "append", sizeof(config.name) - 1);
        config.size_max  = 10;
        config.count_max = 1;

        RotatorPtr rotator(ct_log_rotator_create(&config));
        REQUIRE(rotator != nullptr);
        REQUIRE(ct_log_rotator_write(rotator.get(), "de", 2) == 2);
        rotator.reset();  // trigger destroy

        REQUIRE(fixture.read_file("test_log_rotator_out/append.log0") == "abcde");
    }

    SUBCASE("rejects invalid configuration parameters") {
        ct_log_rotator_config_t config = {};
        REQUIRE(ct_log_rotator_create(&config) == nullptr);
    }
}

TEST_CASE("rotator wraps around and overwrites oldest files upon reaching maximum count") {
    struct WrapFixture {
        WrapFixture() { cleanup(); }
        ~WrapFixture() { cleanup(); }

        void cleanup() const {
            ct_remove("test_rotator_wraparound/wrap.log0");
            ct_remove("test_rotator_wraparound/wrap.log1");
            ct_rmdir("test_rotator_wraparound");
        }

        std::string read_file(const char* path) const {
            FILE* file = std::fopen(path, "rb");
            if (!file) return "";
            std::string data;
            char        buffer[64];
            while (true) {
                size_t n = std::fread(buffer, 1, sizeof(buffer), file);
                if (n == 0) break;
                data.append(buffer, n);
            }
            std::fclose(file);
            return data;
        }
    } fixture;

    ct_log_rotator_config_t config{};
    ct_snprintf_s(config.dir, sizeof(config.dir), "%s", "./test_rotator_wraparound");
    ct_snprintf_s(config.name, sizeof(config.name), "%s", "wrap");
    config.size_max  = 256;
    config.count_max = 2;

    ct_log_rotator_t* r = ct_log_rotator_create(&config);
    REQUIRE(r != nullptr);

    std::string chunk_a(200, 'A');
    std::string chunk_b(200, 'B');
    std::string chunk_c(200, 'C');

    REQUIRE(ct_log_rotator_write(r, chunk_a.data(), chunk_a.size()) > 0);
    REQUIRE(ct_log_rotator_write(r, chunk_b.data(), chunk_b.size()) > 0);
    REQUIRE(ct_log_rotator_write(r, chunk_c.data(), chunk_c.size()) > 0);

    ct_log_rotator_flush(r);

    // 三轮写入后必然回到 index 0
    REQUIRE(ct_log_rotator_index(r) == 0);

    ct_log_rotator_destroy(r);

    std::string log0 = fixture.read_file("./test_rotator_wraparound/wrap.log0");
    std::string log1 = fixture.read_file("./test_rotator_wraparound/wrap.log1");

    // log0 被覆盖重建，只含第三批的尾部 'C'
    REQUIRE(log0.size() == 88);
    REQUIRE(log0 == std::string(88, 'C'));

    // log1 被两批写入填满：先是批次2的尾部 'B'，再是批次3的头部 'C'
    REQUIRE(log1.size() == 256);
    REQUIRE(log1.substr(0, 144) == std::string(144, 'B'));
    REQUIRE(log1.substr(144) == std::string(112, 'C'));
}

TEST_SUITE_END();
