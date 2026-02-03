#include "coter/bytes/seg.h"

#include <catch.hpp>

TEST_CASE("seg Initialization", "[seg][init]") {
	uint8_t buffer[64];
	memset(buffer, 0xAA, sizeof(buffer));

	SECTION("CT_SEG_INIT macro") {
		ct_seg_t seg = CT_SEG_INIT(buffer, sizeof(buffer));
		REQUIRE(seg.bytes == buffer);
		REQUIRE(seg.cap == sizeof(buffer));
		REQUIRE(seg.len == 0);
		REQUIRE(seg.pos == 0);
		REQUIRE(seg.endian == CT_ENDIAN_BIG);
		REQUIRE(seg.hlswap == 0);
	}

	SECTION("CT_SEG_FROM macro") {
		ct_seg_t seg = CT_SEG_FROM(buffer, sizeof(buffer), 32);
		REQUIRE(seg.bytes == buffer);
		REQUIRE(seg.cap == sizeof(buffer));
		REQUIRE(seg.len == 32);
		REQUIRE(seg.pos == 0);
		REQUIRE(seg.endian == CT_ENDIAN_BIG);
		REQUIRE(seg.hlswap == 0);
	}

	SECTION("ct_seg_init function") {
		ct_seg_t seg;
		ct_seg_init(&seg, buffer, sizeof(buffer));
		REQUIRE(seg.bytes == buffer);
		REQUIRE(seg.cap == sizeof(buffer));
		REQUIRE(seg.len == 0);
		REQUIRE(seg.pos == 0);
		REQUIRE(seg.endian == CT_ENDIAN_BIG);
		REQUIRE(seg.hlswap == 0);
	}

	SECTION("ct_seg_from function") {
		ct_seg_t seg;
		ct_seg_from(&seg, buffer, sizeof(buffer), 48);
		REQUIRE(seg.bytes == buffer);
		REQUIRE(seg.cap == sizeof(buffer));
		REQUIRE(seg.len == 48);
		REQUIRE(seg.pos == 0);
		REQUIRE(seg.endian == CT_ENDIAN_BIG);
		REQUIRE(seg.hlswap == 0);
	}

	SECTION("ct_seg_from len > cap protection") {
		ct_seg_t seg;
		ct_seg_from(&seg, buffer, sizeof(buffer), 1000);
		REQUIRE(seg.len == sizeof(buffer));
	}
}

TEST_CASE("seg State Queries", "[seg][state]") {
	uint8_t  buffer[64];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Empty buffer state") {
		REQUIRE(ct_seg_is_empty(&seg) == true);
		REQUIRE(ct_seg_is_full(&seg) == false);
		REQUIRE(ct_seg_capacity(&seg) == sizeof(buffer));
		REQUIRE(ct_seg_count(&seg) == 0);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_readable(&seg) == 0);
		REQUIRE(ct_seg_writable(&seg) == sizeof(buffer));
		REQUIRE(ct_seg_appendable(&seg) == sizeof(buffer));
	}

	SECTION("Partial fill state") {
		ct_seg_put_u32(&seg, 0x12345678);
		REQUIRE(ct_seg_is_empty(&seg) == false);
		REQUIRE(ct_seg_is_full(&seg) == false);
		REQUIRE(ct_seg_count(&seg) == 4);
		REQUIRE(ct_seg_pos(&seg) == 4);
		REQUIRE(ct_seg_readable(&seg) == 0);
		REQUIRE(ct_seg_writable(&seg) == sizeof(buffer) - 4);
		REQUIRE(ct_seg_appendable(&seg) == sizeof(buffer) - 4);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_readable(&seg) == 4);
		REQUIRE(ct_seg_writable(&seg) == sizeof(buffer));
	}

	SECTION("Full buffer state") {
		ct_seg_from(&seg, buffer, sizeof(buffer), sizeof(buffer));
		REQUIRE(ct_seg_is_empty(&seg) == false);
		REQUIRE(ct_seg_is_full(&seg) == true);
		REQUIRE(ct_seg_count(&seg) == sizeof(buffer));
		REQUIRE(ct_seg_appendable(&seg) == 0);
	}
}

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

	SECTION("Default config values") {
		REQUIRE(ct_seg_get_endian(&seg) == CT_ENDIAN_BIG);
		REQUIRE(ct_seg_get_hlswap(&seg) == 0);
	}

	SECTION("Endianness") {
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		REQUIRE(ct_seg_get_endian(&seg) == CT_ENDIAN_LITTLE);
		ct_seg_put_u32(&seg, 0x12345678);

		uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12};
		REQUIRE(memcmp(buffer, expected_le, 4) == 0);

		ct_seg_rewind(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		REQUIRE(ct_seg_get_endian(&seg) == CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0x12345678);

		uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78};
		REQUIRE(memcmp(buffer, expected_be, 4) == 0);
	}

	SECTION("High-Low Swap") {
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_set_hlswap(&seg, 1);
		REQUIRE(ct_seg_get_hlswap(&seg) == 1);

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

		const uint32_t data[] = {0x11223344};
		ct_seg_put_arr32(&seg, data, 1);
		const uint8_t expected[] = {0x33, 0x44, 0x11, 0x22};
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
		ct_seg_put_u8(&seg, 0x12);
		ct_seg_put_u16(&seg, 0x3456);
		ct_seg_put_u32(&seg, 0x789ABCDE);
		ct_seg_put_u64(&seg, 0xFEDCBA9876543210ULL);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_peek_u8(&seg, 0) == 0x12);
		REQUIRE(ct_seg_peek_u16(&seg, 1) == 0x3456);
		REQUIRE(ct_seg_peek_u32(&seg, 3) == 0x789ABCDE);
		REQUIRE(ct_seg_peek_u64(&seg, 7) == 0xFEDCBA9876543210ULL);
		REQUIRE(ct_seg_pos(&seg) == 0);

		ct_seg_seek(&seg, 5);
		REQUIRE(ct_seg_peek_u8(&seg, -5) == 0x12);
		REQUIRE(ct_seg_peek_u16(&seg, -4) == 0x3456);
	}

	SECTION("Peek Arrays") {
		uint32_t data[] = {0x11223344, 0x55667788, 0x99AABBCC};
		ct_seg_put_arr32(&seg, data, 3);
		ct_seg_rewind(&seg);

		uint32_t out[2];
		REQUIRE(ct_seg_peek_arr32(&seg, 0, out, 2) == 0);
		REQUIRE(out[0] == 0x11223344);
		REQUIRE(out[1] == 0x55667788);

		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_peek_arr32(&seg, 4, out, 2) == 0);
		REQUIRE(out[0] == 0x55667788);
		REQUIRE(out[1] == 0x99AABBCC);
	}

	SECTION("Peek Bounds") {
		ct_seg_init(&seg, buffer, 10);
		ct_seg_put_u32(&seg, 0x12345678);
		ct_seg_rewind(&seg);

		REQUIRE(ct_seg_peek_u32(&seg, 0) == 0x12345678);

		REQUIRE(ct_seg_peek_u32(&seg, 10) == 0);
		REQUIRE(ct_seg_peek_u64(&seg, 0) == 0);
		REQUIRE(ct_seg_peek_u32(&seg, -1) == 0);

		uint32_t out[2];
		REQUIRE(ct_seg_peek_arr32(&seg, 0, out, 2) == -1);
	}
}

TEST_CASE("seg Overwrite Operations", "[seg][overwrite]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Overwrite Primitives") {
		ct_seg_put_u32(&seg, 0x11111111);
		ct_seg_put_u32(&seg, 0x22222222);
		ct_seg_put_u32(&seg, 0x33333333);

		size_t pos_before = ct_seg_pos(&seg);
		REQUIRE(ct_seg_overwrite_u32(&seg, 0, 0xAABBCCDD) == 0);
		REQUIRE(ct_seg_pos(&seg) == pos_before);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0xAABBCCDD);
		REQUIRE(ct_seg_take_u32(&seg) == 0x22222222);
		REQUIRE(ct_seg_take_u32(&seg) == 0x33333333);
	}

	SECTION("Overwrite Arrays") {
		uint32_t data[] = {0x11111111, 0x22222222, 0x33333333};
		ct_seg_put_arr32(&seg, data, 3);

		uint32_t new_data[] = {0xAAAAAAAA, 0xBBBBBBBB};
		REQUIRE(ct_seg_overwrite_arr32(&seg, 4, new_data, 2) == 0);

		ct_seg_rewind(&seg);
		uint32_t out[3];
		ct_seg_take_arr32(&seg, out, 3);
		REQUIRE(out[0] == 0x11111111);
		REQUIRE(out[1] == 0xAAAAAAAA);
		REQUIRE(out[2] == 0xBBBBBBBB);
	}

	SECTION("Overwrite Endianness") {
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0x12345678);

		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		REQUIRE(ct_seg_overwrite_u32(&seg, 0, 0xAABBCCDD) == 0);

		uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
		REQUIRE(memcmp(buffer, expected, 4) == 0);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0xAABBCCDD);
	}

	SECTION("Overwrite Bounds") {
		ct_seg_init(&seg, buffer, 10);
		ct_seg_put_u32(&seg, 0x12345678);
		REQUIRE(ct_seg_overwrite_u8(&seg, 0, 0xAA) == 0);
		REQUIRE(ct_seg_overwrite_u32(&seg, 10, 0xBBBBBBBB) == -1);
		REQUIRE(ct_seg_overwrite_u64(&seg, 0, 0x1122334455667788ULL) == -1);

		uint32_t data[] = {0x11111111, 0x22222222};
		REQUIRE(ct_seg_overwrite_arr32(&seg, 0, data, 2) == -1);
	}

	SECTION("Overwrite All Types") {
		ct_seg_put_u64(&seg, 0);
		ct_seg_put_u64(&seg, 0);

		REQUIRE(ct_seg_overwrite_u8(&seg, 0, 0xAB) == 0);
		REQUIRE(ct_seg_overwrite_u16(&seg, 2, 0xCDEF) == 0);
		REQUIRE(ct_seg_overwrite_u64(&seg, 8, 0x1122334455667788ULL) == 0);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u8(&seg) == 0xAB);
		ct_seg_seek(&seg, 2);
		REQUIRE(ct_seg_take_u16(&seg) == 0xCDEF);
		ct_seg_seek(&seg, 8);
		REQUIRE(ct_seg_take_u64(&seg) == 0x1122334455667788ULL);
	}
}

TEST_CASE("seg Position Control", "[seg][position]") {
	uint8_t  buffer[64];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));

	SECTION("seek valid") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_seek(&seg, 16) == 0);
		REQUIRE(ct_seg_pos(&seg) == 16);
		REQUIRE(ct_seg_seek(&seg, 0) == 0);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_seek(&seg, 32) == 0);
		REQUIRE(ct_seg_pos(&seg) == 32);
	}

	SECTION("seek out of bounds") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_seek(&seg, 33) == -1);
		REQUIRE(ct_seg_pos(&seg) == 0);
	}

	SECTION("reseek from end") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_reseek(&seg, 0) == 0);
		REQUIRE(ct_seg_pos(&seg) == 32);
		REQUIRE(ct_seg_reseek(&seg, 10) == 0);
		REQUIRE(ct_seg_pos(&seg) == 22);
		REQUIRE(ct_seg_reseek(&seg, 32) == 0);
		REQUIRE(ct_seg_pos(&seg) == 0);
	}

	SECTION("reseek out of bounds") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_reseek(&seg, 33) == -1);
		REQUIRE(ct_seg_pos(&seg) == 0);
	}

	SECTION("skip forward") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_skip(&seg, 10) == 10);
		REQUIRE(ct_seg_pos(&seg) == 10);
		REQUIRE(ct_seg_skip(&seg, 5) == 5);
		REQUIRE(ct_seg_pos(&seg) == 15);
	}

	SECTION("skip partial") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 10);
		REQUIRE(ct_seg_skip(&seg, 100) == 10);
		REQUIRE(ct_seg_pos(&seg) == 10);
	}

	SECTION("commit extends len") {
		ct_seg_init(&seg, buffer, sizeof(buffer));
		REQUIRE(ct_seg_count(&seg) == 0);

		REQUIRE(ct_seg_commit(&seg, 10) == 10);
		REQUIRE(ct_seg_pos(&seg) == 10);
		REQUIRE(ct_seg_count(&seg) == 10);

		REQUIRE(ct_seg_commit(&seg, 5) == 5);
		REQUIRE(ct_seg_pos(&seg) == 15);
		REQUIRE(ct_seg_count(&seg) == 15);
	}

	SECTION("commit capped by capacity") {
		ct_seg_init(&seg, buffer, sizeof(buffer));
		REQUIRE(ct_seg_commit(&seg, 1000) == (int)sizeof(buffer));
		REQUIRE(ct_seg_pos(&seg) == sizeof(buffer));
		REQUIRE(ct_seg_count(&seg) == sizeof(buffer));
	}

	SECTION("clear resets pos and len") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		ct_seg_seek(&seg, 16);
		REQUIRE(ct_seg_pos(&seg) == 16);
		REQUIRE(ct_seg_count(&seg) == 32);

		ct_seg_clear(&seg);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_count(&seg) == 0);
	}

	SECTION("rewind only resets pos") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		ct_seg_seek(&seg, 16);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_count(&seg) == 32);
	}
}

TEST_CASE("seg IO Operations", "[seg][io]") {
	uint8_t  buffer[64];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("write bytes") {
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
		REQUIRE(ct_seg_write(&seg, data, 5) == 5);
		REQUIRE(ct_seg_pos(&seg) == 5);
		REQUIRE(ct_seg_count(&seg) == 5);
		REQUIRE(memcmp(buffer, data, 5) == 0);
	}

	SECTION("write partial") {
		ct_seg_init(&seg, buffer, 4);
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		REQUIRE(ct_seg_write(&seg, data, 8) == 4);
		REQUIRE(ct_seg_pos(&seg) == 4);
		REQUIRE(ct_seg_count(&seg) == 4);
	}

	SECTION("read bytes") {
		uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
		ct_seg_write(&seg, data, 4);
		ct_seg_rewind(&seg);

		uint8_t out[4] = {0};
		REQUIRE(ct_seg_read(&seg, out, 4) == 4);
		REQUIRE(memcmp(out, data, 4) == 0);
		REQUIRE(ct_seg_pos(&seg) == 4);
	}

	SECTION("read partial") {
		uint8_t data[] = {0xAA, 0xBB};
		ct_seg_write(&seg, data, 2);
		ct_seg_rewind(&seg);

		uint8_t out[8] = {0};
		REQUIRE(ct_seg_read(&seg, out, 8) == 2);
		REQUIRE(ct_seg_pos(&seg) == 2);
	}

	SECTION("peek bytes") {
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
		ct_seg_write(&seg, data, 4);
		ct_seg_rewind(&seg);

		uint8_t out[4] = {0};
		REQUIRE(ct_seg_peek(&seg, out, 4) == 4);
		REQUIRE(memcmp(out, data, 4) == 0);
		REQUIRE(ct_seg_pos(&seg) == 0);
	}

	SECTION("fill pattern") {
		REQUIRE(ct_seg_fill(&seg, 0xAA, 10) == 10);
		REQUIRE(ct_seg_pos(&seg) == 10);
		REQUIRE(ct_seg_count(&seg) == 10);
		for (int i = 0; i < 10; i++) { REQUIRE(buffer[i] == 0xAA); }
	}

	SECTION("fill capped") {
		ct_seg_init(&seg, buffer, 4);
		REQUIRE(ct_seg_fill(&seg, 0xBB, 100) == 4);
		REQUIRE(ct_seg_pos(&seg) == 4);
	}
}

TEST_CASE("seg View Operations", "[seg][view]") {
	uint8_t  buffer[64];
	ct_seg_t seg;
	ct_seg_t view;

	for (int i = 0; i < 64; i++) buffer[i] = (uint8_t)i;

	SECTION("since valid range") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_since(&seg, &view, 8, 24) == 0);
		REQUIRE(view.bytes == buffer + 8);
		REQUIRE(view.cap == sizeof(buffer) - 8);
		REQUIRE(view.len == 16);
		REQUIRE(view.pos == 0);
	}

	SECTION("since invalid range") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_since(&seg, &view, 24, 8) == -1);
		REQUIRE(ct_seg_since(&seg, &view, 0, 100) == -1);
	}

	SECTION("since empty range") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_since(&seg, &view, 10, 10) == 0);
		REQUIRE(view.bytes == buffer + 10);
		REQUIRE(view.len == 0);
		REQUIRE(view.pos == 0);
	}

	SECTION("since inherits config") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_set_hlswap(&seg, 1);

		ct_seg_since(&seg, &view, 0, 16);
		REQUIRE(view.endian == CT_ENDIAN_LITTLE);
		REQUIRE(view.hlswap == 1);
	}

	SECTION("readable_since") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		ct_seg_seek(&seg, 8);
		ct_seg_readable_since(&seg, &view);
		REQUIRE(view.bytes == buffer + 8);
		REQUIRE(view.len == 24);
	}

	SECTION("writable_since") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		ct_seg_seek(&seg, 8);
		ct_seg_writable_since(&seg, &view);
		REQUIRE(view.bytes == buffer + 8);
		REQUIRE(view.len == sizeof(buffer) - 8);
	}

	SECTION("compact") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 16);
		ct_seg_seek(&seg, 8);

		ct_seg_compact(&seg);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_count(&seg) == 8);
		REQUIRE(buffer[0] == 8);
		REQUIRE(buffer[7] == 15);
	}
}

TEST_CASE("seg Put/Take All Types", "[seg][types]") {
	uint8_t  buffer[128];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("u8 roundtrip") {
		ct_seg_put_u8(&seg, 0x00);
		ct_seg_put_u8(&seg, 0xFF);
		ct_seg_put_u8(&seg, 0x7F);
		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u8(&seg) == 0x00);
		REQUIRE(ct_seg_take_u8(&seg) == 0xFF);
		REQUIRE(ct_seg_take_u8(&seg) == 0x7F);
	}

	SECTION("u16 roundtrip") {
		ct_seg_put_u16(&seg, 0x0000);
		ct_seg_put_u16(&seg, 0xFFFF);
		ct_seg_put_u16(&seg, 0x1234);
		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u16(&seg) == 0x0000);
		REQUIRE(ct_seg_take_u16(&seg) == 0xFFFF);
		REQUIRE(ct_seg_take_u16(&seg) == 0x1234);
	}

	SECTION("u32 roundtrip") {
		ct_seg_put_u32(&seg, 0x00000000);
		ct_seg_put_u32(&seg, 0xFFFFFFFF);
		ct_seg_put_u32(&seg, 0x12345678);
		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0x00000000);
		REQUIRE(ct_seg_take_u32(&seg) == 0xFFFFFFFF);
		REQUIRE(ct_seg_take_u32(&seg) == 0x12345678);
	}

	SECTION("u64 roundtrip") {
		ct_seg_put_u64(&seg, 0x0000000000000000ULL);
		ct_seg_put_u64(&seg, 0xFFFFFFFFFFFFFFFFULL);
		ct_seg_put_u64(&seg, 0x123456789ABCDEF0ULL);
		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u64(&seg) == 0x0000000000000000ULL);
		REQUIRE(ct_seg_take_u64(&seg) == 0xFFFFFFFFFFFFFFFFULL);
		REQUIRE(ct_seg_take_u64(&seg) == 0x123456789ABCDEF0ULL);
	}

	SECTION("arr8 roundtrip") {
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
		ct_seg_put_arr8(&seg, data, 4);
		ct_seg_rewind(&seg);
		uint8_t out[4];
		REQUIRE(ct_seg_take_arr8(&seg, out, 4) == 0);
		REQUIRE(memcmp(out, data, 4) == 0);
	}

	SECTION("arr16 roundtrip") {
		uint16_t data[] = {0x1122, 0x3344, 0x5566};
		ct_seg_put_arr16(&seg, data, 3);
		ct_seg_rewind(&seg);
		uint16_t out[3];
		REQUIRE(ct_seg_take_arr16(&seg, out, 3) == 0);
		REQUIRE(out[0] == 0x1122);
		REQUIRE(out[1] == 0x3344);
		REQUIRE(out[2] == 0x5566);
	}

	SECTION("arr32 roundtrip") {
		uint32_t data[] = {0x11223344, 0x55667788};
		ct_seg_put_arr32(&seg, data, 2);
		ct_seg_rewind(&seg);
		uint32_t out[2];
		REQUIRE(ct_seg_take_arr32(&seg, out, 2) == 0);
		REQUIRE(out[0] == 0x11223344);
		REQUIRE(out[1] == 0x55667788);
	}

	SECTION("arr64 roundtrip") {
		uint64_t data[] = {0x1122334455667788ULL, 0x99AABBCCDDEEFF00ULL};
		ct_seg_put_arr64(&seg, data, 2);
		ct_seg_rewind(&seg);
		uint64_t out[2];
		REQUIRE(ct_seg_take_arr64(&seg, out, 2) == 0);
		REQUIRE(out[0] == 0x1122334455667788ULL);
		REQUIRE(out[1] == 0x99AABBCCDDEEFF00ULL);
	}

	SECTION("peek arr8/16/64") {
		uint8_t  d8[]  = {0xAA, 0xBB};
		uint16_t d16[] = {0x1234, 0x5678};
		uint64_t d64[] = {0xDEADBEEFCAFEBABEULL};

		ct_seg_put_arr8(&seg, d8, 2);
		ct_seg_put_arr16(&seg, d16, 2);
		ct_seg_put_arr64(&seg, d64, 1);
		ct_seg_rewind(&seg);

		uint8_t o8[2];
		REQUIRE(ct_seg_peek_arr8(&seg, 0, o8, 2) == 0);
		REQUIRE(o8[0] == 0xAA);
		REQUIRE(o8[1] == 0xBB);

		uint16_t o16[2];
		REQUIRE(ct_seg_peek_arr16(&seg, 2, o16, 2) == 0);
		REQUIRE(o16[0] == 0x1234);
		REQUIRE(o16[1] == 0x5678);

		uint64_t o64[1];
		REQUIRE(ct_seg_peek_arr64(&seg, 6, o64, 1) == 0);
		REQUIRE(o64[0] == 0xDEADBEEFCAFEBABEULL);
	}

	SECTION("overwrite arr8/16/64") {
		ct_seg_fill(&seg, 0, 32);

		uint8_t d8[] = {0xAA, 0xBB};
		REQUIRE(ct_seg_overwrite_arr8(&seg, 0, d8, 2) == 0);
		REQUIRE(buffer[0] == 0xAA);
		REQUIRE(buffer[1] == 0xBB);

		uint16_t d16[] = {0x1234};
		REQUIRE(ct_seg_overwrite_arr16(&seg, 4, d16, 1) == 0);

		uint64_t d64[] = {0xDEADBEEFCAFEBABEULL};
		REQUIRE(ct_seg_overwrite_arr64(&seg, 8, d64, 1) == 0);

		ct_seg_rewind(&seg);
		ct_seg_seek(&seg, 8);
		REQUIRE(ct_seg_take_u64(&seg) == 0xDEADBEEFCAFEBABEULL);
	}
}

TEST_CASE("seg Boundary Overflow", "[seg][boundary]") {
	uint8_t  buffer[8];
	ct_seg_t seg;

	SECTION("put beyond capacity") {
		ct_seg_init(&seg, buffer, sizeof(buffer));
		ct_seg_put_u32(&seg, 0x11111111);
		ct_seg_put_u32(&seg, 0x22222222);

		REQUIRE(ct_seg_pos(&seg) == 8);
		REQUIRE(ct_seg_is_full(&seg) == true);

		ct_seg_put_u8(&seg, 0xFF);
		REQUIRE(ct_seg_pos(&seg) == 8);
	}

	SECTION("take beyond length") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 4);
		buffer[0] = 0x11;
		buffer[1] = 0x22;
		buffer[2] = 0x33;
		buffer[3] = 0x44;

		REQUIRE(ct_seg_take_u32(&seg) == 0x11223344);
		REQUIRE(ct_seg_take_u32(&seg) == 0);
		REQUIRE(ct_seg_pos(&seg) == 4);
	}

	SECTION("array partial fit") {
		ct_seg_init(&seg, buffer, sizeof(buffer));
		uint32_t data[] = {0x11111111, 0x22222222, 0x33333333};
		ct_seg_put_arr32(&seg, data, 3);

		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_count(&seg) == 0);

		ct_seg_put_arr32(&seg, data, 2);
		REQUIRE(ct_seg_pos(&seg) == 8);
		REQUIRE(ct_seg_count(&seg) == 8);

		ct_seg_rewind(&seg);
		uint32_t out[2] = {0};
		REQUIRE(ct_seg_take_arr32(&seg, out, 2) == 0);
		REQUIRE(out[0] == 0x11111111);
		REQUIRE(out[1] == 0x22222222);
	}
}
