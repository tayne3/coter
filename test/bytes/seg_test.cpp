#include "coter/bytes/seg.h"

#include <catch.hpp>

TEST_CASE("seg Basic Operations", "[seg][basic]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Read/Write Primitives") {
		ct_seg_put_u8(&seg, 0x12);
		ct_seg_put_u16(&seg, 0x3456);
		ct_seg_put_u32(&seg, 0x789ABCDE);
		ct_seg_put_u64(&seg, 0xFEDCBA9876543210ULL);

		REQUIRE(ct_seg_pos(&seg) == 1 + 2 + 4 + 8);

		ct_seg_rewind(&seg);

		REQUIRE(ct_seg_take_u8(&seg) == 0x12);
		REQUIRE(ct_seg_take_u16(&seg) == 0x3456);
		REQUIRE(ct_seg_take_u32(&seg) == 0x789ABCDE);
		REQUIRE(ct_seg_take_u64(&seg) == 0xFEDCBA9876543210ULL);
	}

	SECTION("Read/Write Arrays") {
		uint32_t data[] = {0x11223344, 0x55667788};
		ct_seg_put_arr32(&seg, data, 2);

		ct_seg_rewind(&seg);

		uint32_t out[2];
		ct_seg_take_arr32(&seg, out, 2);

		REQUIRE(out[0] == 0x11223344);
		REQUIRE(out[1] == 0x55667788);
	}
}

TEST_CASE("seg Configuration", "[seg][config]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Endianness") {
		// Test Little Endian
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u32(&seg, 0x12345678);

		uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12};
		REQUIRE(memcmp(buffer, expected_le, 4) == 0);

		// Test Big Endian
		ct_seg_rewind(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0x12345678);

		uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78};
		REQUIRE(memcmp(buffer, expected_be, 4) == 0);
	}

	SECTION("High-Low Swap") {
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);  // Use Big Endian for clearer byte order check
		ct_seg_set_hlswap(&seg, 1);

		// u32: 0x11223344 -> 0x22114433
		ct_seg_put_u32(&seg, 0x11223344);
		uint8_t expected_32[] = {0x22, 0x11, 0x44, 0x33};
		REQUIRE(memcmp(buffer, expected_32, 4) == 0);

		ct_seg_rewind(&seg);

		// u64: 0x1122334455667788 -> 0x2211443366558877
		ct_seg_put_u64(&seg, 0x1122334455667788ULL);
		uint8_t expected_64[] = {0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77};
		REQUIRE(memcmp(buffer, expected_64, 8) == 0);
	}

	SECTION("Array Swap") {
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_set_hlswap(&seg, 1);

		uint32_t data[] = {0x11223344};
		// Little Endian + HL Swap
		// 0x11223344
		// HL Swap -> 0x22114433
		// Little Endian Write -> 33 44 11 22
		ct_seg_put_arr32(&seg, data, 1);

		uint8_t expected[] = {0x33, 0x44, 0x11, 0x22};
		REQUIRE(memcmp(buffer, expected, 4) == 0);

		ct_seg_rewind(&seg);
		uint32_t out[1];
		ct_seg_take_arr32(&seg, out, 1);
		REQUIRE(out[0] == 0x11223344);
	}
}

TEST_CASE("seg Peek Operations", "[seg][peek]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Peek Primitives") {
		// Write some data
		ct_seg_put_u8(&seg, 0x12);
		ct_seg_put_u16(&seg, 0x3456);
		ct_seg_put_u32(&seg, 0x789ABCDE);
		ct_seg_put_u64(&seg, 0xFEDCBA9876543210ULL);

		// Peek from start (pos=0, offset=0)
		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_peek_u8(&seg, 0) == 0x12);
		REQUIRE(ct_seg_peek_u16(&seg, 1) == 0x3456);
		REQUIRE(ct_seg_peek_u32(&seg, 3) == 0x789ABCDE);
		REQUIRE(ct_seg_peek_u64(&seg, 7) == 0xFEDCBA9876543210ULL);

		// Position should not change
		REQUIRE(ct_seg_pos(&seg) == 0);

		// Peek with negative offset
		ct_seg_seek(&seg, 5);
		REQUIRE(ct_seg_peek_u8(&seg, -5) == 0x12);
		REQUIRE(ct_seg_peek_u16(&seg, -4) == 0x3456);
	}

	SECTION("Peek Arrays") {
		uint32_t data[] = {0x11223344, 0x55667788, 0x99AABBCC};
		ct_seg_put_arr32(&seg, data, 3);

		ct_seg_rewind(&seg);

		// Peek array at offset 0
		uint32_t out[2];
		REQUIRE(ct_seg_peek_arr32(&seg, 0, out, 2) == 0);
		REQUIRE(out[0] == 0x11223344);
		REQUIRE(out[1] == 0x55667788);

		// Position should not change
		REQUIRE(ct_seg_pos(&seg) == 0);

		// Peek array at offset 4 (skip first element)
		REQUIRE(ct_seg_peek_arr32(&seg, 4, out, 2) == 0);
		REQUIRE(out[0] == 0x55667788);
		REQUIRE(out[1] == 0x99AABBCC);
	}

	SECTION("Peek Bounds") {
		// Re-init with small buffer for bounds testing
		ct_seg_init(&seg, buffer, 10);

		ct_seg_put_u32(&seg, 0x12345678);

		ct_seg_rewind(&seg);

		// Valid peek
		REQUIRE(ct_seg_peek_u32(&seg, 0) == 0x12345678);

		// Out of bounds peek (should return 0)
		REQUIRE(ct_seg_peek_u32(&seg, 10) == 0);
		REQUIRE(ct_seg_peek_u64(&seg, 0) == 0);  // Not enough data

		// Negative offset out of bounds
		REQUIRE(ct_seg_peek_u32(&seg, -1) == 0);

		// Array peek out of bounds
		uint32_t out[2];
		REQUIRE(ct_seg_peek_arr32(&seg, 0, out, 2) == -1);  // Only 1 u32 available
	}
}

TEST_CASE("seg Overwrite Operations", "[seg][overwrite]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Overwrite Primitives") {
		// Write initial data
		ct_seg_put_u32(&seg, 0x11111111);
		ct_seg_put_u32(&seg, 0x22222222);
		ct_seg_put_u32(&seg, 0x33333333);

		size_t pos_before = ct_seg_pos(&seg);

		// Overwrite first u32
		REQUIRE(ct_seg_overwrite_u32(&seg, 0, 0xAABBCCDD) == 0);

		// Position should not change
		REQUIRE(ct_seg_pos(&seg) == pos_before);

		// Verify overwrite
		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0xAABBCCDD);
		REQUIRE(ct_seg_take_u32(&seg) == 0x22222222);
		REQUIRE(ct_seg_take_u32(&seg) == 0x33333333);
	}

	SECTION("Overwrite Arrays") {
		// Write initial data
		uint32_t data[] = {0x11111111, 0x22222222, 0x33333333};
		ct_seg_put_arr32(&seg, data, 3);

		// Overwrite middle element
		uint32_t new_data[] = {0xAAAAAAAA, 0xBBBBBBBB};
		REQUIRE(ct_seg_overwrite_arr32(&seg, 4, new_data, 2) == 0);

		// Verify
		ct_seg_rewind(&seg);
		uint32_t out[3];
		ct_seg_take_arr32(&seg, out, 3);
		REQUIRE(out[0] == 0x11111111);
		REQUIRE(out[1] == 0xAAAAAAAA);
		REQUIRE(out[2] == 0xBBBBBBBB);
	}

	SECTION("Overwrite Endianness") {
		// Write with Big Endian
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0x12345678);

		// Overwrite with Little Endian
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		REQUIRE(ct_seg_overwrite_u32(&seg, 0, 0xAABBCCDD) == 0);

		// Check bytes (Little Endian: DD CC BB AA)
		uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
		REQUIRE(memcmp(buffer, expected, 4) == 0);

		// Read back with Little Endian
		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0xAABBCCDD);
	}

	SECTION("Overwrite Bounds") {
		// Re-init with small buffer
		ct_seg_init(&seg, buffer, 10);

		ct_seg_put_u32(&seg, 0x12345678);

		// Valid overwrite
		REQUIRE(ct_seg_overwrite_u8(&seg, 0, 0xAA) == 0);

		// Out of bounds overwrite
		REQUIRE(ct_seg_overwrite_u32(&seg, 10, 0xBBBBBBBB) == -1);
		REQUIRE(ct_seg_overwrite_u64(&seg, 0, 0x1122334455667788ULL) == -1);  // Not enough space

		// Array overwrite out of bounds
		uint32_t data[] = {0x11111111, 0x22222222};
		REQUIRE(ct_seg_overwrite_arr32(&seg, 0, data, 2) == -1);  // Only 1 u32 available
	}
}
