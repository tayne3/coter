/**
 * @file base64_test.cpp
 * @brief Base64 tests.
 */
#include "coter/encoding/base64.h"

#include <cstring>

#include "coter/testing/doctest.h"


namespace {

struct base64_test_case {
    const char* source_;
    const char* target_;
};

constexpr base64_test_case s_base64_cases[] = {
    {"", ""},
    {"a", "YQ=="},
    {"A", "QQ=="},
    {"x", "eA=="},
    {"xyz", "eHl6"},
    {"abcdef", "YWJjZGVm"},
    {"ы", "0Ys="},
    {"xy", "eHk="},
    {"test", "dGVzdA=="},
    {"abcde", "YWJjZGU="},
    {"ABCDE abcde 12345 ABCDE abcde 12345", "QUJDREUgYWJjZGUgMTIzNDUgQUJDREUgYWJjZGUgMTIzNDU="},
    {" ABCDE abcde 12345 ABCDE abcde 12345", "IEFCQ0RFIGFiY2RlIDEyMzQ1IEFCQ0RFIGFiY2RlIDEyMzQ1"},
    {"ABCDE abcde 12345 ABCDE abcde 12345 ", "QUJDREUgYWJjZGUgMTIzNDUgQUJDREUgYWJjZGUgMTIzNDUg"},
    {" ABCDE abcde 12345 ABCDE abcde 12345 ", "IEFCQ0RFIGFiY2RlIDEyMzQ1IEFCQ0RFIGFiY2RlIDEyMzQ1IA=="},
};

}  // namespace

TEST_CASE("base64_decode" * doctest::test_suite("base64")) {
    char buf[1024];
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 0) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 1) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 2) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 3) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 4) == 2);

    const size_t case_count = sizeof(s_base64_cases) / sizeof(s_base64_cases[0]);
    for (size_t i = 0; i < case_count; ++i) {
        const auto&  it     = s_base64_cases[i];
        const size_t length = std::strlen(it.target_);
        ct_base64_decode(it.target_, length, buf, sizeof(buf));
        REQUIRE(strcmp(buf, it.source_) == 0);
    }
}

TEST_CASE("base64_encode" * doctest::test_suite("base64")) {
    char buf[1024];
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 0) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 1) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 2) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 3) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 4) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 5) == 4);

    const size_t case_count = sizeof(s_base64_cases) / sizeof(s_base64_cases[0]);
    for (size_t i = 0; i < case_count; ++i) {
        const auto&  it     = s_base64_cases[i];
        const size_t length = std::strlen(it.source_);

        ct_base64_encoder_t encoder;
        ct_base64_encoder_init(&encoder);
        size_t n = 0;
        for (size_t j = 0; j < length; ++j) {
            n += ct_base64_encoder_update(&encoder, (const uint8_t*)&it.source_[j], 1, buf + n, sizeof(buf) - n);
        }
        n += ct_base64_encoder_final(&encoder, buf + n, sizeof(buf) - n);
        REQUIRE(strcmp(buf, it.target_) == 0);

        ct_base64_encode((uint8_t*)it.source_, length, buf, sizeof(buf));
        REQUIRE(strcmp(buf, it.target_) == 0);

        REQUIRE(n == CT_BASE64_ENCODE_LENGTH(length));
    }
}
