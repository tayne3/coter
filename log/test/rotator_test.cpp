#include "../src/handler/rotator.h"

#include <catch.hpp>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "coter/core/fs.h"

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
