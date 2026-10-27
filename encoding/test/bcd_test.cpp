/**
 * @file bcd_test.cpp
 * @brief BCD conversion and validation tests.
 */
#include "coter/encoding/bcd.h"

#include "coter/testing/doctest.h"

namespace {

template <typename T>
bool nibbles_are_decimal(T value) {
    for (size_t i = 0; i < sizeof(T) * 2U; ++i) {
        if (((value >> (i * 4U)) & static_cast<T>(0x0FU)) > static_cast<T>(9U)) { return false; }
    }
    return true;
}

template <typename T>
T make_invalid_bcd(size_t index, uint8_t digit) {
    T value = 0;
    for (size_t i = 0; i < sizeof(T) * 2U; ++i) {
        value = static_cast<T>(value | (static_cast<T>(9) << (i * 4U)));
    }
    value = static_cast<T>(value & ~(static_cast<T>(0x0FU) << (index * 4U)));
    value = static_cast<T>(value | (static_cast<T>(digit) << (index * 4U)));
    return value;
}

}  // namespace

TEST_SUITE_BEGIN("bcd");

TEST_CASE("bcd validity") {
    SUBCASE("u8 examples") {
        CHECK(ct_bcd_is_valid_u8(0x00));
        CHECK(ct_bcd_is_valid_u8(0x09));
        CHECK(ct_bcd_is_valid_u8(0x10));
        CHECK(ct_bcd_is_valid_u8(0x99));
        CHECK_FALSE(ct_bcd_is_valid_u8(0x0A));
        CHECK_FALSE(ct_bcd_is_valid_u8(0x0F));
        CHECK_FALSE(ct_bcd_is_valid_u8(0xA0));
        CHECK_FALSE(ct_bcd_is_valid_u8(0xFF));
    }

    SUBCASE("u8 exhaustive") {
        bool all_match = true;
        for (uint32_t value = 0; value <= 0xFFU; ++value) {
            const uint8_t bcd = static_cast<uint8_t>(value);
            if (ct_bcd_is_valid_u8(bcd) != nibbles_are_decimal(bcd)) { all_match = false; }
        }
        CHECK(all_match);
    }

    SUBCASE("u16 exhaustive") {
        bool all_match = true;
        for (uint32_t value = 0; value <= 0xFFFFU; ++value) {
            const uint16_t bcd = static_cast<uint16_t>(value);
            if (ct_bcd_is_valid_u16(bcd) != nibbles_are_decimal(bcd)) { all_match = false; }
        }
        CHECK(all_match);
    }

    SUBCASE("u32 nibble positions") {
        CHECK(ct_bcd_is_valid_u32(0x00000000U));
        CHECK(ct_bcd_is_valid_u32(0x01234567U));
        CHECK(ct_bcd_is_valid_u32(0x99999999U));

        for (size_t i = 0; i < 8U; ++i) {
            CHECK_FALSE(ct_bcd_is_valid_u32(make_invalid_bcd<uint32_t>(i, 0xAU)));
            CHECK_FALSE(ct_bcd_is_valid_u32(make_invalid_bcd<uint32_t>(i, 0xFU)));
        }
    }

    SUBCASE("u64 nibble positions") {
        CHECK(ct_bcd_is_valid_u64(0x0000000000000000ULL));
        CHECK(ct_bcd_is_valid_u64(0x0123456789012345ULL));
        CHECK(ct_bcd_is_valid_u64(0x9999999999999999ULL));

        for (size_t i = 0; i < 16U; ++i) {
            CHECK_FALSE(ct_bcd_is_valid_u64(make_invalid_bcd<uint64_t>(i, 0xAU)));
            CHECK_FALSE(ct_bcd_is_valid_u64(make_invalid_bcd<uint64_t>(i, 0xFU)));
        }
    }
}

TEST_CASE("bcd conversion u8") {
    SUBCASE("ct_bcd_encode_u8") {
        CHECK(ct_bcd_encode_u8(0) == 0x00);
        CHECK(ct_bcd_encode_u8(9) == 0x09);
        CHECK(ct_bcd_encode_u8(10) == 0x10);
        CHECK(ct_bcd_encode_u8(12) == 0x12);
        CHECK(ct_bcd_encode_u8(99) == 0x99);
    }

    SUBCASE("ct_bcd_decode_u8") {
        CHECK(ct_bcd_decode_u8(0x00) == 0);
        CHECK(ct_bcd_decode_u8(0x09) == 9);
        CHECK(ct_bcd_decode_u8(0x10) == 10);
        CHECK(ct_bcd_decode_u8(0x12) == 12);
        CHECK(ct_bcd_decode_u8(0x99) == 99);
    }
}

TEST_CASE("bcd conversion u16") {
    SUBCASE("ct_bcd_encode_u16") {
        CHECK(ct_bcd_encode_u16(0) == 0x0000);
        CHECK(ct_bcd_encode_u16(11) == 0x0011);
        CHECK(ct_bcd_encode_u16(121) == 0x0121);
        CHECK(ct_bcd_encode_u16(1234) == 0x1234);
        CHECK(ct_bcd_encode_u16(9999) == 0x9999);
    }

    SUBCASE("ct_bcd_decode_u16") {
        CHECK(ct_bcd_decode_u16(0x0000) == 0);
        CHECK(ct_bcd_decode_u16(0x0011) == 11);
        CHECK(ct_bcd_decode_u16(0x0121) == 121);
        CHECK(ct_bcd_decode_u16(0x1234) == 1234);
        CHECK(ct_bcd_decode_u16(0x9999) == 9999);
    }
}

TEST_CASE("bcd conversion u32") {
    SUBCASE("ct_bcd_encode_u32") {
        CHECK(ct_bcd_encode_u32(0) == 0x00000000);
        CHECK(ct_bcd_encode_u32(11) == 0x00000011);
        CHECK(ct_bcd_encode_u32(1213) == 0x00001213);
        CHECK(ct_bcd_encode_u32(123456) == 0x0123456);
        CHECK(ct_bcd_encode_u32(12345678) == 0x12345678);
        CHECK(ct_bcd_encode_u32(99999999) == 0x99999999);
    }

    SUBCASE("ct_bcd_decode_u32") {
        CHECK(ct_bcd_decode_u32(0x00000000) == 0);
        CHECK(ct_bcd_decode_u32(0x00000011) == 11);
        CHECK(ct_bcd_decode_u32(0x00001213) == 1213);
        CHECK(ct_bcd_decode_u32(0x0123456) == 123456);
        CHECK(ct_bcd_decode_u32(0x12345678) == 12345678);
        CHECK(ct_bcd_decode_u32(0x99999999) == 99999999);
    }
}

TEST_CASE("bcd conversion u64") {
    SUBCASE("ct_bcd_encode_u64") {
        CHECK(ct_bcd_encode_u64(0) == 0x0000000000000000);
        CHECK(ct_bcd_encode_u64(11) == 0x0000000000000011);
        CHECK(ct_bcd_encode_u64(1213) == 0x000000000001213);
        CHECK(ct_bcd_encode_u64(123456) == 0x000000000123456);
        CHECK(ct_bcd_encode_u64(12345678) == 0x0000000012345678);
        CHECK(ct_bcd_encode_u64(1234567890) == 0x0000001234567890);
        CHECK(ct_bcd_encode_u64(123456789012) == 0x0000123456789012);
        CHECK(ct_bcd_encode_u64(12345678901234) == 0x0012345678901234);
        CHECK(ct_bcd_encode_u64(1234567890123456) == 0x1234567890123456);
        CHECK(ct_bcd_encode_u64(9999999999999999) == 0x9999999999999999);
    }

    SUBCASE("ct_bcd_decode_u64") {
        CHECK(ct_bcd_decode_u64(0x0000000000000000) == 0);
        CHECK(ct_bcd_decode_u64(0x0000000000000011) == 11);
        CHECK(ct_bcd_decode_u64(0x000000000001213) == 1213);
        CHECK(ct_bcd_decode_u64(0x000000000123456) == 123456);
        CHECK(ct_bcd_decode_u64(0x0000000012345678) == 12345678);
        CHECK(ct_bcd_decode_u64(0x0000001234567890) == 1234567890);
        CHECK(ct_bcd_decode_u64(0x0000123456789012) == 123456789012);
        CHECK(ct_bcd_decode_u64(0x0012345678901234) == 12345678901234);
        CHECK(ct_bcd_decode_u64(0x1234567890123456) == 1234567890123456);
        CHECK(ct_bcd_decode_u64(0x9999999999999999) == 9999999999999999);
    }
}

TEST_CASE("bcd encoding truncates to storage width") {
    CHECK(ct_bcd_encode_u8(100) == 0x00);
    CHECK(ct_bcd_encode_u8(123) == 0x23);
    CHECK(ct_bcd_encode_u8(255) == 0x55);

    CHECK(ct_bcd_encode_u16(10000) == 0x0000);
    CHECK(ct_bcd_encode_u16(12345) == 0x2345);
    CHECK(ct_bcd_encode_u16(65535) == 0x5535);

    CHECK(ct_bcd_encode_u32(100000000U) == 0x00000000);
    CHECK(ct_bcd_encode_u32(123456789U) == 0x23456789);

    CHECK(ct_bcd_encode_u64(10000000000000000ULL) == 0x0000000000000000);
    CHECK(ct_bcd_encode_u64(12345678901234567ULL) == 0x2345678901234567ULL);
}

TEST_CASE("bcd roundtrip") {
    SUBCASE("u8") {
        bool all_match = true;
        for (uint32_t value = 0; value < 100U; ++value) {
            const uint8_t bcd = ct_bcd_encode_u8(static_cast<uint8_t>(value));
            if (ct_bcd_decode_u8(bcd) != value) { all_match = false; }
        }
        CHECK(all_match);
    }

    SUBCASE("u16") {
        bool all_match = true;
        for (uint32_t value = 0; value < 10000U; ++value) {
            const uint16_t bcd = ct_bcd_encode_u16(static_cast<uint16_t>(value));
            if (ct_bcd_decode_u16(bcd) != value) { all_match = false; }
        }
        CHECK(all_match);
    }

    SUBCASE("u32") {
        bool all_match = true;
        for (uint32_t value = 0; value < 1000000U; ++value) {
            const uint32_t bcd = ct_bcd_encode_u32(value);
            if (ct_bcd_decode_u32(bcd) != value) { all_match = false; }
        }
        CHECK(all_match);
    }

    SUBCASE("u64 boundaries") {
        const uint64_t values[] = {
            0ULL,
            1ULL,
            9ULL,
            10ULL,
            99ULL,
            100ULL,
            9999ULL,
            10000ULL,
            9999999999999999ULL,
            1234567890123456ULL,
        };

        for (uint64_t value : values) {
            const uint64_t bcd = ct_bcd_encode_u64(value);
            CHECK(ct_bcd_decode_u64(bcd) == value);
        }
    }
}

TEST_CASE("bcd encoded values are valid") {
    SUBCASE("u8") {
        bool all_valid = true;
        for (uint32_t value = 0; value <= 0xFFU; ++value) {
            if (!ct_bcd_is_valid_u8(ct_bcd_encode_u8(static_cast<uint8_t>(value)))) { all_valid = false; }
        }
        CHECK(all_valid);
    }

    SUBCASE("u16") {
        bool all_valid = true;
        for (uint32_t value = 0; value <= 0xFFFFU; ++value) {
            if (!ct_bcd_is_valid_u16(ct_bcd_encode_u16(static_cast<uint16_t>(value)))) { all_valid = false; }
        }
        CHECK(all_valid);
    }
}

TEST_SUITE_END();
