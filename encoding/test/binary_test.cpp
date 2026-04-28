#include "coter/encoding/binary.h"

#include <catch.hpp>

TEST_CASE("binary_le_uint16", "[binary]") {
    uint8_t buf[2] = {0};
    ct_le_set_u16(buf, 0x1234);
    REQUIRE(buf[0] == 0x34);
    REQUIRE(buf[1] == 0x12);
    uint16_t val = ct_le_get_u16(buf);
    REQUIRE(val == 0x1234);
}

TEST_CASE("binary_le_uint32", "[binary]") {
    uint8_t buf[4] = {0};
    ct_le_set_u32(buf, 0x12345678);
    REQUIRE(buf[0] == 0x78);
    REQUIRE(buf[1] == 0x56);
    REQUIRE(buf[2] == 0x34);
    REQUIRE(buf[3] == 0x12);
    uint32_t val = ct_le_get_u32(buf);
    REQUIRE(val == 0x12345678);
}

TEST_CASE("binary_le_uint64", "[binary]") {
    uint8_t buf[8] = {0};
    ct_le_set_u64(buf, 0x0102030405060708ULL);
    REQUIRE(buf[0] == 0x08);
    REQUIRE(buf[1] == 0x07);
    REQUIRE(buf[2] == 0x06);
    REQUIRE(buf[3] == 0x05);
    REQUIRE(buf[4] == 0x04);
    REQUIRE(buf[5] == 0x03);
    REQUIRE(buf[6] == 0x02);
    REQUIRE(buf[7] == 0x01);
    uint64_t val = ct_le_get_u64(buf);
    REQUIRE(val == 0x0102030405060708ULL);
}

TEST_CASE("binary_be_uint16", "[binary]") {
    uint8_t buf[2] = {0};
    ct_be_set_u16(buf, 0x1234);
    REQUIRE(buf[0] == 0x12);
    REQUIRE(buf[1] == 0x34);
    uint16_t val = ct_be_get_u16(buf);
    REQUIRE(val == 0x1234);
}

TEST_CASE("binary_be_uint32", "[binary]") {
    uint8_t buf[4] = {0};
    ct_be_set_u32(buf, 0x12345678);
    REQUIRE(buf[0] == 0x12);
    REQUIRE(buf[1] == 0x34);
    REQUIRE(buf[2] == 0x56);
    REQUIRE(buf[3] == 0x78);
    uint32_t val = ct_be_get_u32(buf);
    REQUIRE(val == 0x12345678);
}

TEST_CASE("binary_be_uint64", "[binary]") {
    uint8_t buf[8] = {0};
    ct_be_set_u64(buf, 0x0102030405060708ULL);
    REQUIRE(buf[0] == 0x01);
    REQUIRE(buf[1] == 0x02);
    REQUIRE(buf[2] == 0x03);
    REQUIRE(buf[3] == 0x04);
    REQUIRE(buf[4] == 0x05);
    REQUIRE(buf[5] == 0x06);
    REQUIRE(buf[6] == 0x07);
    REQUIRE(buf[7] == 0x08);
    uint64_t val = ct_be_get_u64(buf);
    REQUIRE(val == 0x0102030405060708ULL);
}

TEST_CASE("binary_zero_max", "[binary]") {
    uint8_t buf[8] = {0};
    ct_le_set_u16(buf, 0);
    REQUIRE(ct_le_get_u16(buf) == 0);
    ct_le_set_u32(buf, 0);
    REQUIRE(ct_le_get_u32(buf) == 0);
    ct_le_set_u64(buf, 0);
    REQUIRE(ct_le_get_u64(buf) == 0);
    ct_le_set_u16(buf, UINT16_MAX);
    REQUIRE(ct_le_get_u16(buf) == UINT16_MAX);
    ct_le_set_u32(buf, UINT32_MAX);
    REQUIRE(ct_le_get_u32(buf) == UINT32_MAX);
    ct_le_set_u64(buf, UINT64_MAX);
    REQUIRE(ct_le_get_u64(buf) == UINT64_MAX);
}

TEST_CASE("binary_unaligned", "[binary]") {
    uint8_t  storage[16] = {0};
    uint8_t* buf         = storage + 1;
    ct_le_set_u32(buf, 0xDEADBEEF);
    REQUIRE(ct_le_get_u32(buf) == 0xDEADBEEF);
    ct_be_set_u64(buf, 0x0123456789ABCDEFULL);
    REQUIRE(ct_be_get_u64(buf) == 0x0123456789ABCDEFULL);
}

TEST_CASE("binary_bswap16_batch", "[binary]") {
    uint16_t data[16] = {0x0001, 0x0102, 0x0203, 0x0304, 0x0405, 0x0506, 0x0607, 0x0708,
                         0x0809, 0x090A, 0x0A0B, 0x0B0C, 0x0C0D, 0x0D0E, 0x0E0F, 0x0F10};
    ct_binary_bswap16_batch(data, 16);
    REQUIRE(data[0] == 0x0100);
    REQUIRE(data[1] == 0x0201);
    REQUIRE(data[15] == 0x100F);
}

TEST_CASE("binary_bswap32_batch", "[binary]") {
    uint32_t data[8] = {0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10, 0x11121314, 0x15161718, 0x191A1B1C, 0x1D1E1F20};
    ct_binary_bswap32_batch(data, 8);
    REQUIRE(data[0] == 0x04030201);
    REQUIRE(data[7] == 0x201F1E1D);
}

TEST_CASE("binary_bswap64_batch", "[binary]") {
    uint64_t data[4] = {0x0102030405060708ULL, 0x090A0B0C0D0E0F10ULL, 0x1112131415161718ULL, 0x191A1B1C1D1E1F20ULL};
    ct_binary_bswap64_batch(data, 4);
    REQUIRE(data[0] == 0x0807060504030201ULL);
    REQUIRE(data[3] == 0x201F1E1D1C1B1A19ULL);
}

TEST_CASE("binary_batch_odd_count", "[binary]") {
    uint32_t data[7] = {1, 2, 3, 4, 5, 6, 7};
    ct_binary_bswap32_batch(data, 7);
    for (int i = 0; i < 7; ++i) { REQUIRE(data[i] != (uint32_t)(i + 1)); }
    ct_binary_bswap32_batch(data, 7);
    for (int i = 0; i < 7; ++i) { REQUIRE(data[i] == (uint32_t)(i + 1)); }
}

TEST_CASE("binary_bswap16_x2", "[binary]") {
    REQUIRE(ct_binary_bswap16_lanes32(0x11223344) == 0x22114433);
    REQUIRE(ct_binary_bswap16_lanes32(0x00000000) == 0x00000000);
    REQUIRE(ct_binary_bswap16_lanes32(0xFFFFFFFF) == 0xFFFFFFFF);
}

TEST_CASE("binary_bswap16_x4", "[binary]") {
    REQUIRE(ct_binary_bswap16_lanes64(0x1122334455667788ULL) == 0x2211443366558877ULL);
    REQUIRE(ct_binary_bswap16_lanes64(0x0000000000000000ULL) == 0x0000000000000000ULL);
}

TEST_CASE("binary_bswap16_x2_batch", "[binary]") {
    uint32_t data[8] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10};
    ct_binary_bswap16_lanes32_batch(data, 8);
    REQUIRE(data[0] == 0x22114433);
    REQUIRE(data[1] == 0x66558877);
    REQUIRE(data[2] == 0xAA99CCBB);
    REQUIRE(data[3] == 0xEEDD00FF);
    REQUIRE(data[4] == 0x02010403);
    REQUIRE(data[7] == 0x0E0D100F);
}

TEST_CASE("binary_bswap16_x4_batch", "[binary]") {
    uint64_t data[4] = {0x1122334455667788ULL, 0x99AABBCCDDEEFF00ULL, 0x0102030405060708ULL, 0x090A0B0C0D0E0F10ULL};
    ct_binary_bswap16_lanes64_batch(data, 4);
    REQUIRE(data[0] == 0x2211443366558877ULL);
    REQUIRE(data[1] == 0xAA99CCBBEEDD00FFULL);
    REQUIRE(data[2] == 0x0201040306050807ULL);
    REQUIRE(data[3] == 0x0A090C0B0E0D100FULL);
}

TEST_CASE("binary_reverse_words32", "[binary]") {
    REQUIRE(ct_binary_reverse16_lanes32(0x11223344) == 0x33441122);
    REQUIRE(ct_binary_reverse16_lanes32(0x00000000) == 0x00000000);
    REQUIRE(ct_binary_reverse16_lanes32(0xFFFFFFFF) == 0xFFFFFFFF);
    REQUIRE(ct_binary_reverse16_lanes32(0x01020304) == 0x03040102);
}

TEST_CASE("binary_reverse_words64", "[binary]") {
    REQUIRE(ct_binary_reverse16_lanes64(0x1122334455667788ULL) == 0x7788556633441122ULL);
    REQUIRE(ct_binary_reverse16_lanes64(0x0000000000000000ULL) == 0x0000000000000000ULL);
    REQUIRE(ct_binary_reverse16_lanes64(0xFFFFFFFFFFFFFFFFULL) == 0xFFFFFFFFFFFFFFFFULL);
}

TEST_CASE("binary_reverse_words32_batch", "[binary]") {
    uint32_t data[8] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10};
    ct_binary_reverse16_lanes32_batch(data, 8);
    REQUIRE(data[0] == 0x33441122);
    REQUIRE(data[1] == 0x77885566);
    REQUIRE(data[2] == 0xBBCC99AA);
    REQUIRE(data[3] == 0xFF00DDEE);
    REQUIRE(data[4] == 0x03040102);
    REQUIRE(data[7] == 0x0F100D0E);
}

TEST_CASE("binary_reverse_words64_batch", "[binary]") {
    uint64_t data[4] = {0x1122334455667788ULL, 0x99AABBCCDDEEFF00ULL, 0x0102030405060708ULL, 0x090A0B0C0D0E0F10ULL};
    ct_binary_reverse16_lanes64_batch(data, 4);
    REQUIRE(data[0] == 0x7788556633441122ULL);
    REQUIRE(data[1] == 0xFF00DDEEBBCC99AAULL);
    REQUIRE(data[2] == 0X0708050603040102ULL);
    REQUIRE(data[3] == 0X0F100D0E0B0C090AULL);
}

TEST_CASE("binary_reverse_words_batch_odd", "[binary]") {
    uint32_t data32[7] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x01020304, 0x05060708, 0x090A0B0C};
    ct_binary_reverse16_lanes32_batch(data32, 7);
    REQUIRE(data32[0] == 0x33441122);
    REQUIRE(data32[1] == 0x77885566);
    REQUIRE(data32[2] == 0xBBCC99AA);
    REQUIRE(data32[3] == 0xFF00DDEE);
    REQUIRE(data32[4] == 0x03040102);
    REQUIRE(data32[5] == 0x07080506);
    REQUIRE(data32[6] == 0x0B0C090A);
    ct_binary_reverse16_lanes32_batch(data32, 7);
    REQUIRE(data32[0] == 0x11223344);
    REQUIRE(data32[1] == 0x55667788);
    REQUIRE(data32[2] == 0x99AABBCC);
    REQUIRE(data32[3] == 0xDDEEFF00);
    REQUIRE(data32[4] == 0x01020304);
    REQUIRE(data32[5] == 0x05060708);
    REQUIRE(data32[6] == 0x090A0B0C);
}

TEST_CASE("binary_at_and_roundtrip", "[binary]") {
    uint8_t buf[8] = {0};
    for (uint16_t i = 0; i < 1000; ++i) {
        ct_le_set_u16(buf, i);
        REQUIRE(ct_le_get_u16(buf) == i);
        ct_be_set_u16(buf, i);
        REQUIRE(ct_be_get_u16(buf) == i);
    }

    ct_be_set_u16_at(buf, 1, 0x1234);
    REQUIRE(ct_be_get_u16_at(buf, 1) == 0x1234);
    ct_le_set_u32_at(buf, 2, 0x89ABCDEFU);
    REQUIRE(ct_le_get_u32_at(buf, 2) == 0x89ABCDEFU);

    ct_ne_set_u64(buf, 0x0102030405060708ULL);
    REQUIRE(ct_ne_get_u64(buf) == 0x0102030405060708ULL);
    ct_ne_set_u32_at(buf, 4, 0xAABBCCDDU);
    REQUIRE(ct_ne_get_u32_at(buf, 4) == 0xAABBCCDDU);
}
