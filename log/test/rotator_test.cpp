#include "../src/handler/rotator.h"

#include <catch.hpp>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "coter/core/fs.h"
#include "coter/core/strings.h"

namespace {
static const char* kDir = "test_log_rotator_out";

struct RotatorFixture {
    RotatorFixture() { cleanup(); }
    ~RotatorFixture() { cleanup(); }

    void cleanup() {
        ct_remove("test_log_rotator_out/rotate.log0");
        ct_remove("test_log_rotator_out/rotate.log1");
        ct_remove("test_log_rotator_out/append.log0");
        ct_rmdir(kDir);
    }

    static std::string read_file(const char* path) {
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
};

struct RotatorDeleter {
    void operator()(ct_log_rotator_t* r) const {
        if (r) ct_log_rotator_destroy(r);
    }
};
using RotatorPtr = std::unique_ptr<ct_log_rotator_t, RotatorDeleter>;
}  // namespace

TEST_CASE("log_rotator_logic", "[log][rotator]") {
    RotatorFixture fixture;

    SECTION("rotates across bounded files") {
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
        REQUIRE(RotatorFixture::read_file("test_log_rotator_out/rotate.log0") == "kl");
        REQUIRE(RotatorFixture::read_file("test_log_rotator_out/rotate.log1") == "fghij");
    }

    SECTION("appends newest file when it has space") {
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

        REQUIRE(RotatorFixture::read_file("test_log_rotator_out/append.log0") == "abcde");
    }

    SECTION("rejects invalid config") {
        ct_log_rotator_config_t config = {};
        REQUIRE(ct_log_rotator_create(&config) == nullptr);
    }
}

TEST_CASE("log_rotator_wraps_around_correctly", "[log][rotator]") {
    // 配置极小的轮转（每个文件 256 字节，最多 2 个文件）
    // 写入超过 512 字节，验证文件从 index 0 -> 1 -> 0 循环覆盖

    struct WrapFixture {
        WrapFixture() { cleanup(); }
        ~WrapFixture() { cleanup(); }
        void cleanup() {
            ct_remove("test_rotator_wraparound/wrap.log0");
            ct_remove("test_rotator_wraparound/wrap.log1");
            ct_rmdir("test_rotator_wraparound");
        }
    } fixture;

    ct_log_rotator_config_t config{};
    ct_snprintf_s(config.dir, sizeof(config.dir), "%s", "./test_rotator_wraparound");
    ct_snprintf_s(config.name, sizeof(config.name), "%s", "wrap");
    config.size_max  = 256;
    config.count_max = 2;

    ct_log_rotator_t* r = ct_log_rotator_create(&config);
    REQUIRE(r != nullptr);

    // 写入 3 批，每批 200 字节，触发至少 2 次轮转：
    //   批次 1 (200B): 写入 log0，current_size=200
    //   批次 2 (200B): log0 满 (200+200>256)，轮转到 log1，写入 200B
    //   批次 3 (200B): log1 满 (200+200>256)，轮转回 log0（覆盖），写入 200B
    std::string chunk_a(200, 'A');
    std::string chunk_b(200, 'B');
    std::string chunk_c(200, 'C');

    REQUIRE(ct_log_rotator_write(r, chunk_a.data(), chunk_a.size()) > 0);
    REQUIRE(ct_log_rotator_write(r, chunk_b.data(), chunk_b.size()) > 0);
    REQUIRE(ct_log_rotator_write(r, chunk_c.data(), chunk_c.size()) > 0);

    ct_log_rotator_flush(r);

    // 精确断言：三轮写入后必然回到 index 0
    REQUIRE(ct_log_rotator_index(r) == 0);

    ct_log_rotator_destroy(r);

    // 追踪实际写入路径（size_max=256, count_max=2）：
    //   批次1 (200'A'): log0 可用256，写200'A' → log0=[200'A'], size=200
    //   批次2 (200'B'): log0 剩56，先写56'B'填满log0，轮转到log1，再写144'B' → log1=[144'B'], size=144
    //   批次3 (200'C'): log1 剩112，先写112'C'填满log1，轮转回log0（删除旧log0），再写88'C' → log0=[88'C'], size=88
    //
    // 最终状态：file_index=0, log0=88'C', log1=144'B'+112'C'(=256字节)

    std::string log0 = RotatorFixture::read_file("./test_rotator_wraparound/wrap.log0");
    std::string log1 = RotatorFixture::read_file("./test_rotator_wraparound/wrap.log1");

    // log0 被覆盖重建，只含第三批的尾部 'C'
    REQUIRE(log0.size() == 88);
    REQUIRE(log0 == std::string(88, 'C'));

    // log1 被两批写入填满：先是批次2的尾部 'B'，再是批次3的头部 'C'
    REQUIRE(log1.size() == 256);
    REQUIRE(log1.substr(0, 144) == std::string(144, 'B'));
    REQUIRE(log1.substr(144) == std::string(112, 'C'));
}
