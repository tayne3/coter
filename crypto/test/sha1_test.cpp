#include "coter/crypto/sha1.h"

#include <cstring>
#include <string>

#include "coter/testing/doctest.h"

namespace {

std::string digest_to_hex(const uint8_t digest[20]) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string       result;
    result.reserve(40);
    for (int i = 0; i < 20; ++i) {
        result.push_back(hex_chars[digest[i] >> 4]);
        result.push_back(hex_chars[digest[i] & 0xf]);
    }
    return result;
}

}  // namespace

TEST_CASE("sha1" * doctest::test_suite("sha1")) {
    struct ct_sha1_test {
        const char* data;
        const char* target;
    } ct_sha1_test_all[] = {
        {
            "",
            "da39a3ee5e6b4b0d3255bfef95601890afd80709",
        },
        {
            " ",
            "b858cb282617fb0956d960215c8e84d1ccf909c6",
        },
        {
            "hello world",
            "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed",
        },
        {
            "AAAAA BBBBB CCCCC DDDDD EEEEE",
            "ec4e621944740fbd671b01118b92946091f3b6dd",
        },
        {
            "ABCDE abcde 12345 ABCDE abcde 12345",
            "53b9d25b26ceba3797ced1bcfdf17443d73d74e3",
        },
        {
            " ABCDE abcde 12345 ABCDE abcde 12345",
            "7e2abf30e9693af3dccb3725e242ebeaf993d748",
        },
        {
            "ABCDE abcde 12345 ABCDE abcde 12345 ",
            "c2c93670f63cfd733cbedbfa7b741348b318dc2a",
        },
        {
            " ABCDE abcde 12345 ABCDE abcde 12345 ",
            "98216df5b65942ea5c79aaf2b03bd013d9b84392",
        },
    };

    const int size = (int)(sizeof(ct_sha1_test_all) / sizeof(ct_sha1_test_all[0]));
    for (int i = 0; i < size; ++i) {
        const struct ct_sha1_test* it  = &ct_sha1_test_all[i];
        const size_t               len = std::strlen(it->data);
        INFO("i=" << i);

        // Test ct_sha1_sum
        {
            uint8_t digest[20] = {0};
            ct_sha1_sum(it->data, len, digest);
            REQUIRE(digest_to_hex(digest) == std::string(it->target));
        }

        // Test ct_sha1_init / ct_sha1_update / ct_sha1_final
        {
            ct_sha1_ctx_t ctx;
            ct_sha1_init(&ctx);
            ct_sha1_update(&ctx, it->data, len);
            uint8_t digest[20] = {0};
            ct_sha1_final(&ctx, digest);
            REQUIRE(digest_to_hex(digest) == std::string(it->target));
        }
    }
}
