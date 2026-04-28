#include "coter/encoding/bcd.h"

#include <catch.hpp>

TEST_CASE("bcd conversion u8", "[bcd][u8]") {
    SECTION("ct_bcd_from_u8") {
        CHECK(ct_bcd_from_u8(0) == 0x00);
        CHECK(ct_bcd_from_u8(9) == 0x09);
        CHECK(ct_bcd_from_u8(10) == 0x10);
        CHECK(ct_bcd_from_u8(12) == 0x12);
        CHECK(ct_bcd_from_u8(99) == 0x99);
    }

    SECTION("ct_bcd_to_u8") {
        CHECK(ct_bcd_to_u8(0x00) == 0);
        CHECK(ct_bcd_to_u8(0x09) == 9);
        CHECK(ct_bcd_to_u8(0x10) == 10);
        CHECK(ct_bcd_to_u8(0x12) == 12);
        CHECK(ct_bcd_to_u8(0x99) == 99);
    }
}

TEST_CASE("bcd conversion u16", "[bcd][u16]") {
    SECTION("ct_bcd_from_u16") {
        CHECK(ct_bcd_from_u16(0) == 0x0000);
        CHECK(ct_bcd_from_u16(11) == 0x0011);
        CHECK(ct_bcd_from_u16(121) == 0x0121);
        CHECK(ct_bcd_from_u16(1234) == 0x1234);
        CHECK(ct_bcd_from_u16(9999) == 0x9999);
    }

    SECTION("ct_bcd_to_u16") {
        CHECK(ct_bcd_to_u16(0x0000) == 0);
        CHECK(ct_bcd_to_u16(0x0011) == 11);
        CHECK(ct_bcd_to_u16(0x0121) == 121);
        CHECK(ct_bcd_to_u16(0x1234) == 1234);
        CHECK(ct_bcd_to_u16(0x9999) == 9999);
    }
}

TEST_CASE("bcd conversion u32", "[bcd][u32]") {
    SECTION("ct_bcd_from_u32") {
        CHECK(ct_bcd_from_u32(0) == 0x00000000);
        CHECK(ct_bcd_from_u32(11) == 0x00000011);
        CHECK(ct_bcd_from_u32(1213) == 0x00001213);
        CHECK(ct_bcd_from_u32(123456) == 0x0123456);
        CHECK(ct_bcd_from_u32(12345678) == 0x12345678);
        CHECK(ct_bcd_from_u32(99999999) == 0x99999999);
    }

    SECTION("ct_bcd_to_u32") {
        CHECK(ct_bcd_to_u32(0x00000000) == 0);
        CHECK(ct_bcd_to_u32(0x00000011) == 11);
        CHECK(ct_bcd_to_u32(0x00001213) == 1213);
        CHECK(ct_bcd_to_u32(0x0123456) == 123456);
        CHECK(ct_bcd_to_u32(0x12345678) == 12345678);
        CHECK(ct_bcd_to_u32(0x99999999) == 99999999);
    }
}

TEST_CASE("bcd conversion u64", "[bcd][u64]") {
    SECTION("ct_bcd_from_u64") {
        CHECK(ct_bcd_from_u64(0) == 0x0000000000000000);
        CHECK(ct_bcd_from_u64(11) == 0x0000000000000011);
        CHECK(ct_bcd_from_u64(1213) == 0x000000000001213);
        CHECK(ct_bcd_from_u64(123456) == 0x000000000123456);
        CHECK(ct_bcd_from_u64(12345678) == 0x0000000012345678);
        CHECK(ct_bcd_from_u64(1234567890) == 0x0000001234567890);
        CHECK(ct_bcd_from_u64(123456789012) == 0x0000123456789012);
        CHECK(ct_bcd_from_u64(12345678901234) == 0x0012345678901234);
        CHECK(ct_bcd_from_u64(1234567890123456) == 0x1234567890123456);
        CHECK(ct_bcd_from_u64(9999999999999999) == 0x9999999999999999);
    }

    SECTION("ct_bcd_to_u64") {
        CHECK(ct_bcd_to_u64(0x0000000000000000) == 0);
        CHECK(ct_bcd_to_u64(0x0000000000000011) == 11);
        CHECK(ct_bcd_to_u64(0x000000000001213) == 1213);
        CHECK(ct_bcd_to_u64(0x000000000123456) == 123456);
        CHECK(ct_bcd_to_u64(0x0000000012345678) == 12345678);
        CHECK(ct_bcd_to_u64(0x0000001234567890) == 1234567890);
        CHECK(ct_bcd_to_u64(0x0000123456789012) == 123456789012);
        CHECK(ct_bcd_to_u64(0x0012345678901234) == 12345678901234);
        CHECK(ct_bcd_to_u64(0x1234567890123456) == 1234567890123456);
        CHECK(ct_bcd_to_u64(0x9999999999999999) == 9999999999999999);
    }
}
