#include "coter/encoding/hex.h"

#include <catch.hpp>
#include <cstring>

TEST_CASE("hex_encode", "[hex]") {
    char    output[32];
    uint8_t data[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f};
    int     len    = ct_hex_encode(data, sizeof(data), output, sizeof(output));
    REQUIRE(len == 10);
    REQUIRE(std::strcmp(output, "48656c6c6f") == 0);
    char small_output[11];
    len = ct_hex_encode(data, sizeof(data), small_output, 11);
    REQUIRE(len == 10);
    REQUIRE(std::strcmp(small_output, "48656c6c6f") == 0);
    char tiny_output[5];
    len = ct_hex_encode(data, sizeof(data), tiny_output, 5);
    REQUIRE(len == 4);
    REQUIRE(std::strcmp(tiny_output, "4865") == 0);
}

TEST_CASE("hex_decode", "[hex]") {
    uint8_t     output[32];
    const char* hex = "48656c6c6f";
    int         len = ct_hex_decode(hex, 0, output, sizeof(output));
    REQUIRE(len == 5);
    REQUIRE(output[0] == 0x48);
    REQUIRE(output[1] == 0x65);
    REQUIRE(output[2] == 0x6c);
    REQUIRE(output[3] == 0x6c);
    REQUIRE(output[4] == 0x6f);
    const char* hex_upper = "48656C6C6F";
    len                   = ct_hex_decode(hex_upper, 0, output, sizeof(output));
    REQUIRE(len == 5);
    REQUIRE(output[4] == 0x6f);
    const char* hex_invalid = "4865GG";
    len                     = ct_hex_decode(hex_invalid, 0, output, sizeof(output));
    REQUIRE(len == -1);
    const char* hex_odd = "486";
    len                 = ct_hex_decode(hex_odd, 0, output, sizeof(output));
    REQUIRE(len == -1);
    uint8_t small_output[2];
    len = ct_hex_decode(hex, 0, small_output, 2);
    REQUIRE(len == 2);
    REQUIRE(small_output[0] == 0x48);
    REQUIRE(small_output[1] == 0x65);
    REQUIRE(ct_hex_decode(nullptr, 0, output, sizeof(output)) == -1);
    REQUIRE(ct_hex_decode(hex, 0, nullptr, sizeof(output)) == -1);
    REQUIRE(ct_hex_decode(hex, 0, output, 0) == -1);
    REQUIRE(ct_hex_decode("", 0, output, sizeof(output)) == 0);
    const char* hex_with_space = "48 65";
    len                        = ct_hex_decode(hex_with_space, 0, output, sizeof(output));
    REQUIRE(len == -1);
}

TEST_CASE("hex_encode_edge", "[hex]") {
    char    output[32];
    uint8_t data[] = {0x48, 0x65};
    REQUIRE(ct_hex_encode(nullptr, sizeof(data), output, sizeof(output)) == -1);
    REQUIRE(ct_hex_encode(data, sizeof(data), nullptr, sizeof(output)) == -1);
    REQUIRE(ct_hex_encode(data, sizeof(data), output, 0) == -1);
    REQUIRE(ct_hex_encode(data, 0, output, sizeof(output)) == 0);
}
