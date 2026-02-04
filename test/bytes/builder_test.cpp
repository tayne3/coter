#include "coter/bytes/builder.h"

#include <catch.hpp>
#include <cstring>

struct BuilderFixture {
	ct_builder_t* builder;
	BuilderFixture() : builder(nullptr) {
		REQUIRE(builder == nullptr);
		builder = ct_builder_create(0);
		REQUIRE(builder != nullptr);
	}
	~BuilderFixture() {
		REQUIRE(builder != nullptr);
		ct_builder_destroy(builder);
		builder = nullptr;
	}
};

TEST_CASE_METHOD(BuilderFixture, "Builder Lifecycle", "[builder][lifecycle]") {
	SECTION("Initialization") {
		REQUIRE(ct_builder_capacity(builder) == 64);
		REQUIRE(ct_builder_count(builder) == 0);
		REQUIRE(ct_builder_is_empty(builder));
	}

	SECTION("Auto-grow from Zero") {
		ct_builder_put_u8(builder, 0xFF);
		REQUIRE(ct_builder_count(builder) == 1);
		REQUIRE(ct_builder_capacity(builder) >= 64);
		ct_builder_rewind(builder);
		REQUIRE(ct_builder_take_u8(builder) == 0xFF);
	}

	SECTION("Capacity Doubling") {
		for (int i = 0; i < 64; i++) { ct_builder_put_u8(builder, (uint8_t)i); }
		REQUIRE(ct_builder_capacity(builder) >= 64);
		size_t cap_after_64 = ct_builder_capacity(builder);
		ct_builder_put_u8(builder, 0xFF);
		REQUIRE(ct_builder_capacity(builder) >= cap_after_64 * 2);
		ct_builder_rewind(builder);
		for (int i = 0; i < 64; i++) { REQUIRE(ct_builder_take_u8(builder) == (uint8_t)i); }
		REQUIRE(ct_builder_take_u8(builder) == 0xFF);
	}

	SECTION("Large Data Handling") {
		for (uint32_t i = 0; i < 1000; i++) { ct_builder_put_u32(builder, i); }
		REQUIRE(ct_builder_count(builder) == 4000);
		REQUIRE(ct_builder_capacity(builder) >= 4000);
		ct_builder_rewind(builder);
		for (uint32_t i = 0; i < 1000; i++) { REQUIRE(ct_builder_take_u32(builder) == i); }
	}

	SECTION("Manual Growth") {
		ct_builder_put_u32(builder, 0x12345678);
		REQUIRE(ct_builder_grow(builder, 100) == 0);
		REQUIRE(ct_builder_capacity(builder) >= 104);
		size_t cap_before = ct_builder_capacity(builder);
		for (int i = 0; i < 25; i++) { ct_builder_put_u32(builder, i); }
		REQUIRE(ct_builder_capacity(builder) == cap_before);
	}

	SECTION("Capacity Reservation") {
		REQUIRE(ct_builder_reserve(builder, 1024) == 0);
		REQUIRE(ct_builder_capacity(builder) >= 1024);
		size_t cap_before = ct_builder_capacity(builder);
		for (uint32_t i = 0; i < 256; i++) { ct_builder_put_u32(builder, i); }
		REQUIRE(ct_builder_count(builder) == 1024);
		REQUIRE(ct_builder_capacity(builder) == cap_before);
	}

	SECTION("Redundant Reservation") {
		ct_builder_reserve(builder, 200);
		size_t cap = ct_builder_capacity(builder);
		ct_builder_reserve(builder, 100);
		REQUIRE(ct_builder_capacity(builder) == cap);
	}
}

TEST_CASE_METHOD(BuilderFixture, "Builder Operations", "[builder][ops]") {
	SECTION("Raw Bytes Write") {
		uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
		REQUIRE(ct_builder_write(builder, data, sizeof(data)) == 5);
		REQUIRE(ct_builder_count(builder) == 5);
		ct_builder_rewind(builder);
		uint8_t out[5];
		ct_builder_read(builder, out, 5);
		REQUIRE(std::memcmp(out, data, 5) == 0);
	}

	SECTION("Fill Pattern") {
		REQUIRE(ct_builder_fill(builder, 0xAA, 10) == 10);
		REQUIRE(ct_builder_count(builder) == 10);
		ct_builder_rewind(builder);
		for (int i = 0; i < 10; i++) { REQUIRE(ct_builder_take_u8(builder) == 0xAA); }
	}

	SECTION("Primitive Types") {
		ct_builder_put_u8(builder, 0x12);
		ct_builder_put_u16(builder, 0x3456);
		ct_builder_put_u32(builder, 0x789ABCDE);
		ct_builder_put_u64(builder, 0xFEDCBA9876543210ULL);
		REQUIRE(ct_builder_count(builder) == 15);
		ct_builder_rewind(builder);
		REQUIRE(ct_builder_take_u8(builder) == 0x12);
		REQUIRE(ct_builder_take_u16(builder) == 0x3456);
		REQUIRE(ct_builder_take_u32(builder) == 0x789ABCDE);
		REQUIRE(ct_builder_take_u64(builder) == 0xFEDCBA9876543210ULL);
	}

	SECTION("Array Types") {
		uint32_t data[] = {0x11223344, 0x55667788, 0x99AABBCC};
		ct_builder_put_arr32(builder, data, 3);
		REQUIRE(ct_builder_count(builder) == 12);
		ct_builder_rewind(builder);
		uint32_t out[3];
		ct_builder_take_arr32(builder, out, 3);
		REQUIRE(out[0] == 0x11223344);
		REQUIRE(out[1] == 0x55667788);
		REQUIRE(out[2] == 0x99AABBCC);
	}

	SECTION("Read & Peek") {
		ct_builder_put_u32(builder, 0x12345678);
		ct_builder_put_u32(builder, 0xAABBCCDD);
		ct_builder_rewind(builder);
		REQUIRE(ct_builder_peek_u32(builder, 0) == 0x12345678);
		REQUIRE(ct_builder_peek_u32(builder, 4) == 0xAABBCCDD);
		REQUIRE(ct_builder_pos(builder) == 0);
		REQUIRE(ct_builder_take_u32(builder) == 0x12345678);
		REQUIRE(ct_builder_pos(builder) == 4);
	}

	SECTION("Overwrite") {
		ct_builder_put_u32(builder, 0x11111111);
		ct_builder_put_u32(builder, 0x22222222);
		ct_builder_overwrite_u32(builder, 0, 0xAAAAAAAA);
		ct_builder_rewind(builder);
		REQUIRE(ct_builder_take_u32(builder) == 0xAAAAAAAA);
		REQUIRE(ct_builder_take_u32(builder) == 0x22222222);
	}

	SECTION("Seg Views") {
		ct_builder_put_u32(builder, 0x11223344);
		ct_builder_put_u32(builder, 0x55667788);
		ct_seg_t seg;
		ct_builder_rewind(builder);
		ct_builder_readable_seg(builder, &seg);
		REQUIRE(ct_seg_count(&seg) == 8);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_take_u32(&seg) == 0x11223344);
		ct_builder_seek(builder, 4);
		ct_builder_writable_seg(builder, &seg);
		ct_seg_put_u32(&seg, 0xAABBCCDD);
		ct_builder_rewind(builder);
		REQUIRE(ct_builder_take_u32(builder) == 0x11223344);
		REQUIRE(ct_builder_take_u32(builder) == 0xAABBCCDD);
		ct_builder_rewind(builder);
		REQUIRE(ct_builder_seg(builder, &seg, 2, 6) == 0);
		REQUIRE(ct_seg_count(&seg) == 4);
		uint8_t buf[4];
		ct_seg_read(&seg, buf, 4);
		uint8_t expected[] = {0x33, 0x44, 0xAA, 0xBB};
		REQUIRE(std::memcmp(buf, expected, 4) == 0);
		size_t builder_pos = ct_builder_pos(builder);
		ct_seg_put_u8(&seg, 0xFF);
		REQUIRE(ct_builder_pos(builder) == builder_pos);
	}
}

TEST_CASE_METHOD(BuilderFixture, "Builder Configuration", "[builder][config]") {
	SECTION("Endianness Control") {
		ct_builder_set_endian(builder, CT_ENDIAN_LITTLE);
		ct_builder_put_u32(builder, 0x12345678);
		uint8_t expected[] = {0x78, 0x56, 0x34, 0x12};
		REQUIRE(std::memcmp(ct_builder_data(builder), expected, 4) == 0);
	}

	SECTION("High-Low Swap") {
		ct_builder_set_endian(builder, CT_ENDIAN_BIG);
		ct_builder_set_hlswap(builder, 1);
		ct_builder_put_u32(builder, 0x11223344);
		uint8_t expected[] = {0x22, 0x11, 0x44, 0x33};
		REQUIRE(std::memcmp(ct_builder_data(builder), expected, 4) == 0);
	}

	SECTION("Position Control") {
		ct_builder_put_u32(builder, 0x12345678);
		ct_builder_put_u32(builder, 0xAABBCCDD);
		ct_builder_rewind(builder);
		REQUIRE(ct_builder_pos(builder) == 0);
		ct_builder_seek(builder, 4);
		REQUIRE(ct_builder_pos(builder) == 4);
		ct_builder_rewind(builder);
		ct_builder_skip(builder, 2);
		REQUIRE(ct_builder_pos(builder) == 2);
	}
}
