#include "coter/encoding/base64.h"

#include <catch.hpp>
#include <cstring>

struct ct_base64_test {
    const char* source;
    const char* target;
} ct_base64_test_all[] = {
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

TEST_CASE("base64_decode", "[base64]") {
    char buf[1024];
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 0) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 1) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 2) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 3) == 0);
    REQUIRE(ct_base64_decode("AAA=", 4, buf, 4) == 2);

    const size_t size = sizeof(ct_base64_test_all) / sizeof(ct_base64_test_all[0]);
    for (size_t i = 0; i < size; ++i) {
        const auto&  it     = ct_base64_test_all[i];
        const size_t length = std::strlen(it.target);
        ct_base64_decode(it.target, length, buf, sizeof(buf));
        REQUIRE(std::strcmp(buf, it.source) == 0);
    }
}

TEST_CASE("base64_encode", "[base64]") {
    char buf[1024];
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 0) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 1) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 2) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 3) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 4) == 0);
    REQUIRE(ct_base64_encode((uint8_t*)"a", 1, buf, 5) == 4);

    const size_t size = sizeof(ct_base64_test_all) / sizeof(ct_base64_test_all[0]);
    for (size_t i = 0; i < size; ++i) {
        const auto&  it     = ct_base64_test_all[i];
        const size_t length = std::strlen(it.source);

        size_t n = 0;
        for (size_t j = 0; j < length; ++j) { n = ct_base64_update(it.source[j], buf, n); }
        n = ct_base64_final(buf, n);
        REQUIRE(std::strcmp(buf, it.target) == 0);

        ct_base64_encode((uint8_t*)it.source, length, buf, sizeof(buf));
        REQUIRE(std::strcmp(buf, it.target) == 0);

        REQUIRE(n == CT_BASE64_ENCODE_LENGTH(length));
    }
}
