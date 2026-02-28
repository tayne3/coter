#include "coter/bytes/seg.h"

#include <catch.hpp>
#include <cstring>

TEST_CASE("seg Initialization", "[seg][init]") {
	uint8_t buffer[64];
	memset(buffer, 0xAA, sizeof(buffer));

	SECTION("ct_seg_init function") {
		ct_seg_t seg;
		ct_seg_init(&seg, buffer, sizeof(buffer));
		REQUIRE(seg.data == buffer);
		REQUIRE(seg.cap == sizeof(buffer));
		REQUIRE(seg.len == 0);
		REQUIRE(seg.pos == 0);
		REQUIRE(seg.endian == CT_ENDIAN_BIG);
		REQUIRE(seg.hlswap == 0);
	}

	SECTION("ct_seg_from function (clamped)") {
		ct_seg_t seg;
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(seg.data == buffer);
		REQUIRE(seg.cap == sizeof(buffer));
		REQUIRE(seg.len == 32);
		REQUIRE(seg.pos == 0);
		REQUIRE(seg.endian == CT_ENDIAN_BIG);
		REQUIRE(seg.hlswap == 0);
	}

	SECTION("ct_seg_init function") {
		ct_seg_t seg;
		ct_seg_init(&seg, buffer, sizeof(buffer));
		REQUIRE(seg.data == buffer);
		REQUIRE(seg.cap == sizeof(buffer));
		REQUIRE(seg.len == 0);
		REQUIRE(seg.pos == 0);
		REQUIRE(seg.endian == CT_ENDIAN_BIG);
		REQUIRE(seg.hlswap == 0);
	}

	SECTION("ct_seg_from function") {
		ct_seg_t seg;
		ct_seg_from(&seg, buffer, sizeof(buffer), 48);
		REQUIRE(seg.data == buffer);
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

	SECTION("Static Initialization Macros") {
		ct_seg_t seg_init = CT_SEG_INIT(buffer, sizeof(buffer));
		REQUIRE(seg_init.data == buffer);
		REQUIRE(seg_init.cap == sizeof(buffer));
		REQUIRE(seg_init.len == 0);
		REQUIRE(seg_init.pos == 0);
		REQUIRE(seg_init.endian == CT_ENDIAN_BIG);
		REQUIRE(seg_init.hlswap == 0);

		ct_seg_t seg_from = CT_SEG_FROM(buffer, sizeof(buffer), 32);
		REQUIRE(seg_from.data == buffer);
		REQUIRE(seg_from.cap == sizeof(buffer));
		REQUIRE(seg_from.len == 32);
		REQUIRE(seg_from.pos == 0);
		REQUIRE(seg_from.endian == CT_ENDIAN_BIG);
		REQUIRE(seg_from.hlswap == 0);
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
}

TEST_CASE("ct_seg Endianness Write and Read Identity", "[seg][endian]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;
	ct_seg_init(&seg, buffer, sizeof(buffer));

#define TEST_RW_IDENTITY(PutFn, TakeFn, PeekFn, OverwriteFn, Value) \
	do {                                                            \
		ct_seg_clear(&seg);                                         \
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);                  \
		PutFn(&seg, Value);                                         \
		ct_seg_rewind(&seg);                                        \
		REQUIRE(TakeFn(&seg) == Value);                             \
		ct_seg_clear(&seg);                                         \
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);                     \
		PutFn(&seg, Value);                                         \
		ct_seg_rewind(&seg);                                        \
		REQUIRE(TakeFn(&seg) == Value);                             \
		ct_seg_clear(&seg);                                         \
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);                  \
		PutFn(&seg, Value);                                         \
		ct_seg_rewind(&seg);                                        \
		REQUIRE(PeekFn(&seg, 0) == Value);                          \
		REQUIRE(OverwriteFn(&seg, 0, Value) == 0);                  \
		ct_seg_rewind(&seg);                                        \
		REQUIRE(TakeFn(&seg) == Value);                             \
	} while (0)

	TEST_RW_IDENTITY(ct_seg_put_u8, ct_seg_take_u8, ct_seg_peek_u8, ct_seg_set_u8, 0x12);
	TEST_RW_IDENTITY(ct_seg_put_u16, ct_seg_take_u16, ct_seg_peek_u16, ct_seg_set_u16, 0x1234);
	TEST_RW_IDENTITY(ct_seg_put_u32, ct_seg_take_u32, ct_seg_peek_u32, ct_seg_set_u32, 0x12345678);
	TEST_RW_IDENTITY(ct_seg_put_u64, ct_seg_take_u64, ct_seg_peek_u64, ct_seg_set_u64, 0x1122334455667788ULL);

#undef TEST_RW_IDENTITY

	SECTION("Default config values from ct_seg_init") {
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_set_hlswap(&seg, 1);
		REQUIRE(ct_seg_get_endian(&seg) == CT_ENDIAN_LITTLE);
		REQUIRE(ct_seg_get_hlswap(&seg) == 1);

		ct_seg_init(&seg, buffer, sizeof(buffer));
		REQUIRE(ct_seg_get_endian(&seg) == CT_ENDIAN_BIG);
		REQUIRE(ct_seg_get_hlswap(&seg) == 0);
	}
}

TEST_CASE("ct_seg Endianness Memory Layout Verification", "[seg][endian]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("16-bit") {
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u16(&seg, 0xABCD);
		REQUIRE(buffer[0] == 0xCD);
		REQUIRE(buffer[1] == 0xAB);
		ct_seg_clear(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u16(&seg, 0xABCD);
		REQUIRE(buffer[0] == 0xAB);
		REQUIRE(buffer[1] == 0xCD);
	}

	SECTION("32-bit") {
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u32(&seg, 0x12345678);
		REQUIRE(buffer[0] == 0x78);
		REQUIRE(buffer[1] == 0x56);
		REQUIRE(buffer[2] == 0x34);
		REQUIRE(buffer[3] == 0x12);
		ct_seg_clear(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0x12345678);
		REQUIRE(buffer[0] == 0x12);
		REQUIRE(buffer[1] == 0x34);
		REQUIRE(buffer[2] == 0x56);
		REQUIRE(buffer[3] == 0x78);
	}

	SECTION("64-bit") {
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u64(&seg, 0x1122334455667788ULL);
		REQUIRE(buffer[0] == 0x88);
		REQUIRE(buffer[1] == 0x77);
		REQUIRE(buffer[2] == 0x66);
		REQUIRE(buffer[3] == 0x55);
		REQUIRE(buffer[4] == 0x44);
		REQUIRE(buffer[5] == 0x33);
		REQUIRE(buffer[6] == 0x22);
		REQUIRE(buffer[7] == 0x11);

		ct_seg_clear(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u64(&seg, 0x1122334455667788ULL);
		REQUIRE(buffer[0] == 0x11);
		REQUIRE(buffer[1] == 0x22);
		REQUIRE(buffer[2] == 0x33);
		REQUIRE(buffer[3] == 0x44);
		REQUIRE(buffer[4] == 0x55);
		REQUIRE(buffer[5] == 0x66);
		REQUIRE(buffer[6] == 0x77);
		REQUIRE(buffer[7] == 0x88);
	}
}

TEST_CASE("ct_seg High-Low Word Swap Constraints", "[seg][hlswap]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("16-bit is unaffected by hlswap") {
		ct_seg_clear(&seg);
		ct_seg_set_hlswap(&seg, 1);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u16(&seg, 0xABCD);
		REQUIRE(buffer[0] == 0xAB);
		REQUIRE(buffer[1] == 0xCD);

		ct_seg_clear(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u16(&seg, 0xABCD);
		REQUIRE(buffer[0] == 0xCD);
		REQUIRE(buffer[1] == 0xAB);
	}

	SECTION("32-bit swap") {
		ct_seg_clear(&seg);
		ct_seg_set_hlswap(&seg, 1);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0xAABBCCDD);
		REQUIRE(buffer[0] == 0xBB);
		REQUIRE(buffer[1] == 0xAA);
		REQUIRE(buffer[2] == 0xDD);
		REQUIRE(buffer[3] == 0xCC);

		ct_seg_clear(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u32(&seg, 0xAABBCCDD);
		REQUIRE(buffer[0] == 0xCC);
		REQUIRE(buffer[1] == 0xDD);
		REQUIRE(buffer[2] == 0xAA);
		REQUIRE(buffer[3] == 0xBB);
	}

	SECTION("64-bit swap") {
		ct_seg_clear(&seg);
		ct_seg_set_hlswap(&seg, 1);
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u64(&seg, 0x1122334455667788ULL);
		REQUIRE(buffer[0] == 0x22);
		REQUIRE(buffer[1] == 0x11);
		REQUIRE(buffer[2] == 0x44);
		REQUIRE(buffer[3] == 0x33);
		REQUIRE(buffer[4] == 0x66);
		REQUIRE(buffer[5] == 0x55);
		REQUIRE(buffer[6] == 0x88);
		REQUIRE(buffer[7] == 0x77);

		ct_seg_clear(&seg);
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u64(&seg, 0x1122334455667788ULL);
		REQUIRE(buffer[0] == 0x77);
		REQUIRE(buffer[1] == 0x88);
		REQUIRE(buffer[2] == 0x55);
		REQUIRE(buffer[3] == 0x66);
		REQUIRE(buffer[4] == 0x33);
		REQUIRE(buffer[5] == 0x44);
		REQUIRE(buffer[6] == 0x11);
		REQUIRE(buffer[7] == 0x22);
	}
}

TEST_CASE("ct_seg View-Only Operation Endianness Validation", "[seg][peek][set]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;
	ct_seg_init(&seg, buffer, sizeof(buffer));

	buffer[0] = 0x11;
	buffer[1] = 0x22;
	buffer[2] = 0x33;
	buffer[3] = 0x44;

	SECTION("Peek respects endian") {
		ct_seg_commit(&seg, 4);
		ct_seg_rewind(&seg);

		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		REQUIRE(ct_seg_peek_u32(&seg, 0) == 0x11223344);

		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		REQUIRE(ct_seg_peek_u32(&seg, 0) == 0x44332211);
	}

	SECTION("Overwrite respects endian") {
		ct_seg_commit(&seg, 4);

		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		REQUIRE(ct_seg_set_u32(&seg, 0, 0xAABBCCDD) == 0);
		REQUIRE(buffer[0] == 0xAA);
		REQUIRE(buffer[1] == 0xBB);
		REQUIRE(buffer[2] == 0xCC);
		REQUIRE(buffer[3] == 0xDD);

		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		REQUIRE(ct_seg_set_u32(&seg, 0, 0xAABBCCDD) == 0);
		REQUIRE(buffer[0] == 0xDD);
		REQUIRE(buffer[1] == 0xCC);
		REQUIRE(buffer[2] == 0xBB);
		REQUIRE(buffer[3] == 0xAA);
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

	SECTION("Peek Bounds") {
		ct_seg_init(&seg, buffer, 10);
		ct_seg_put_u32(&seg, 0x12345678);
		ct_seg_rewind(&seg);

		REQUIRE(ct_seg_peek_u32(&seg, 0) == 0x12345678);

		REQUIRE(ct_seg_peek_u32(&seg, 10) == 0);
		REQUIRE(ct_seg_peek_u64(&seg, 0) == 0);
		REQUIRE(ct_seg_peek_u32(&seg, -1) == 0);
	}
}

TEST_CASE("seg Set Operations", "[seg][set]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Set Primitives") {
		ct_seg_put_u32(&seg, 0x11111111);
		ct_seg_put_u32(&seg, 0x22222222);
		ct_seg_put_u32(&seg, 0x33333333);

		size_t pos_before = ct_seg_pos(&seg);
		REQUIRE(ct_seg_set_u32(&seg, 0, 0xAABBCCDD) == 0);
		REQUIRE(ct_seg_pos(&seg) == pos_before);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0xAABBCCDD);
		REQUIRE(ct_seg_take_u32(&seg) == 0x22222222);
		REQUIRE(ct_seg_take_u32(&seg) == 0x33333333);
	}

	SECTION("Set Endianness") {
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0x12345678);

		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		REQUIRE(ct_seg_set_u32(&seg, 0, 0xAABBCCDD) == 0);

		uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
		REQUIRE(std::memcmp(buffer, expected, 4) == 0);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0xAABBCCDD);
	}

	SECTION("Set Bounds") {
		ct_seg_init(&seg, buffer, 10);
		ct_seg_put_u32(&seg, 0x12345678);
		REQUIRE(ct_seg_set_u8(&seg, 0, 0xAA) == 0);
		REQUIRE(ct_seg_set_u32(&seg, 10, 0xBBBBBBBB) == -1);
		REQUIRE(ct_seg_set_u64(&seg, 0, 0x1122334455667788ULL) == -1);
	}

	SECTION("Set All Types") {
		ct_seg_put_u64(&seg, 0);
		ct_seg_put_u64(&seg, 0);

		REQUIRE(ct_seg_set_u8(&seg, 0, 0xAB) == 0);
		REQUIRE(ct_seg_set_u16(&seg, 2, 0xCDEF) == 0);
		REQUIRE(ct_seg_set_u64(&seg, 8, 0x1122334455667788ULL) == 0);

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

	SECTION("write data") {
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
		REQUIRE(ct_seg_write(&seg, data, 5) == 5);
		REQUIRE(ct_seg_pos(&seg) == 5);
		REQUIRE(ct_seg_count(&seg) == 5);
		REQUIRE(std::memcmp(buffer, data, 5) == 0);
	}

	SECTION("write partial") {
		ct_seg_init(&seg, buffer, 4);
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		REQUIRE(ct_seg_write(&seg, data, 8) == 4);
		REQUIRE(ct_seg_pos(&seg) == 4);
		REQUIRE(ct_seg_count(&seg) == 4);
	}

	SECTION("read data") {
		uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
		ct_seg_write(&seg, data, 4);
		ct_seg_rewind(&seg);

		uint8_t out[4] = {0};
		REQUIRE(ct_seg_read(&seg, out, 4) == 4);
		REQUIRE(std::memcmp(out, data, 4) == 0);
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

	SECTION("peek data") {
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
		ct_seg_write(&seg, data, 4);
		ct_seg_rewind(&seg);

		uint8_t out[4] = {0};
		REQUIRE(ct_seg_peek(&seg, 0, out, 4) == 4);
		REQUIRE(std::memcmp(out, data, 4) == 0);
		REQUIRE(ct_seg_pos(&seg) == 0);

		uint8_t out2[2] = {0};
		REQUIRE(ct_seg_peek(&seg, 2, out2, 2) == 2);
		REQUIRE(out2[0] == 0x33);
		REQUIRE(out2[1] == 0x44);

		ct_seg_skip(&seg, 2);
		REQUIRE(ct_seg_peek(&seg, 0, out2, 2) == 2);
		REQUIRE(out2[0] == 0x33);
		REQUIRE(out2[1] == 0x44);
	}

	SECTION("fill pattern") {
		REQUIRE(ct_seg_fill(&seg, 0xAA, 10) == 10);
		REQUIRE(ct_seg_pos(&seg) == 10);
		REQUIRE(ct_seg_count(&seg) == 10);
		for (int i = 0; i < 10; ++i) { REQUIRE(buffer[i] == 0xAA); }
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

	for (int i = 0; i < 64; ++i) buffer[i] = (uint8_t)i;

	SECTION("since valid range") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_since(&seg, &view, 8, 24) == 0);
		REQUIRE(view.data == buffer + 8);
		REQUIRE(view.cap == sizeof(buffer) - 8);
		REQUIRE(view.len == 16);
		REQUIRE(view.pos == 0);
	}

	SECTION("since with default end (0)") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);

		// Case 1: start=0, end=0 -> [0, 32]
		REQUIRE(ct_seg_since(&seg, &view, 0, 0) == 0);
		REQUIRE(view.data == buffer);
		REQUIRE(view.len == 32);

		// Case 2: start=10, end=0 -> [10, 32]
		REQUIRE(ct_seg_since(&seg, &view, 10, 0) == 0);
		REQUIRE(view.data == buffer + 10);
		REQUIRE(view.len == 22);
	}

	SECTION("since invalid range") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_since(&seg, &view, 24, 8) == -1);
		REQUIRE(ct_seg_since(&seg, &view, 0, 100) == -1);
	}

	SECTION("since empty range") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		REQUIRE(ct_seg_since(&seg, &view, 10, 10) == 0);
		REQUIRE(view.data == buffer + 10);
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
		REQUIRE(view.data == buffer + 8);
		REQUIRE(view.len == 24);
	}

	SECTION("writable_since") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		ct_seg_seek(&seg, 8);
		ct_seg_writable_since(&seg, &view);
		REQUIRE(view.data == buffer + 8);
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
}

TEST_CASE("seg Get Operations", "[seg][get]") {
	uint8_t  buffer[4096];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Get Primitives") {
		ct_seg_put_u8(&seg, 0x12);
		ct_seg_put_u16(&seg, 0x3456);
		ct_seg_put_u32(&seg, 0x789ABCDE);
		ct_seg_put_u64(&seg, 0xFEDCBA9876543210ULL);

		REQUIRE(ct_seg_get_u8(&seg, 0) == 0x12);
		REQUIRE(ct_seg_get_u16(&seg, 1) == 0x3456);
		REQUIRE(ct_seg_get_u32(&seg, 3) == 0x789ABCDE);
		REQUIRE(ct_seg_get_u64(&seg, 7) == 0xFEDCBA9876543210ULL);

		REQUIRE(ct_seg_pos(&seg) == 15);
	}

	SECTION("Get does not change pos or len") {
		ct_seg_put_u32(&seg, 0x12345678);
		ct_seg_rewind(&seg);

		size_t pos_before = ct_seg_pos(&seg);
		size_t len_before = ct_seg_count(&seg);

		ct_seg_get_u32(&seg, 0);

		REQUIRE(ct_seg_pos(&seg) == pos_before);
		REQUIRE(ct_seg_count(&seg) == len_before);
	}

	SECTION("Get Bounds") {
		ct_seg_init(&seg, buffer, 10);
		ct_seg_put_u32(&seg, 0x12345678);

		REQUIRE(ct_seg_get_u32(&seg, 0) == 0x12345678);
		REQUIRE(ct_seg_get_u32(&seg, 10) == 0);
		REQUIRE(ct_seg_get_u64(&seg, 0) == 0);
		REQUIRE(ct_seg_get_u8(&seg, 100) == 0);
	}

	SECTION("Get Endianness Big") {
		ct_seg_set_endian(&seg, CT_ENDIAN_BIG);
		ct_seg_put_u32(&seg, 0x11223344);

		REQUIRE(ct_seg_get_u8(&seg, 0) == 0x11);
		REQUIRE(ct_seg_get_u8(&seg, 1) == 0x22);
		REQUIRE(ct_seg_get_u8(&seg, 2) == 0x33);
		REQUIRE(ct_seg_get_u8(&seg, 3) == 0x44);
	}

	SECTION("Get Endianness Little") {
		ct_seg_set_endian(&seg, CT_ENDIAN_LITTLE);
		ct_seg_put_u32(&seg, 0x11223344);

		REQUIRE(ct_seg_get_u8(&seg, 0) == 0x44);
		REQUIRE(ct_seg_get_u8(&seg, 1) == 0x33);
		REQUIRE(ct_seg_get_u8(&seg, 2) == 0x22);
		REQUIRE(ct_seg_get_u8(&seg, 3) == 0x11);
	}

	SECTION("Get All Types") {
		ct_seg_put_u8(&seg, 0xAB);
		ct_seg_put_u16(&seg, 0xCDEF);
		ct_seg_put_u32(&seg, 0x12345678);
		ct_seg_put_u64(&seg, 0xFEDCBA9876543210ULL);

		REQUIRE(ct_seg_get_u8(&seg, 0) == 0xAB);
		REQUIRE(ct_seg_get_u16(&seg, 1) == 0xCDEF);
		REQUIRE(ct_seg_get_u32(&seg, 3) == 0x12345678);
		REQUIRE(ct_seg_get_u64(&seg, 7) == 0xFEDCBA9876543210ULL);
	}
}

TEST_CASE("seg Truncate", "[seg][truncate]") {
	uint8_t  buffer[64];
	ct_seg_t seg;

	memset(buffer, 0, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Truncate normal") {
		ct_seg_put_u32(&seg, 0x11111111);
		ct_seg_put_u32(&seg, 0x22222222);
		ct_seg_put_u32(&seg, 0x33333333);

		REQUIRE(ct_seg_count(&seg) == 12);

		ct_seg_truncate(&seg, 8);
		REQUIRE(ct_seg_count(&seg) == 8);
		REQUIRE(ct_seg_pos(&seg) == 8);

		ct_seg_rewind(&seg);
		REQUIRE(ct_seg_take_u32(&seg) == 0x11111111);
		REQUIRE(ct_seg_take_u32(&seg) == 0x22222222);
		REQUIRE(ct_seg_readable(&seg) == 0);
	}

	SECTION("Truncate adjusts pos if needed") {
		ct_seg_put_u32(&seg, 0x11111111);
		ct_seg_put_u32(&seg, 0x22222222);
		ct_seg_put_u32(&seg, 0x33333333);

		ct_seg_seek(&seg, 10);
		REQUIRE(ct_seg_pos(&seg) == 10);

		ct_seg_truncate(&seg, 4);
		REQUIRE(ct_seg_count(&seg) == 4);
		REQUIRE(ct_seg_pos(&seg) == 4);
	}

	SECTION("Truncate no effect if new_len >= len") {
		ct_seg_put_u32(&seg, 0x12345678);
		REQUIRE(ct_seg_count(&seg) == 4);

		ct_seg_truncate(&seg, 10);
		REQUIRE(ct_seg_count(&seg) == 4);

		ct_seg_truncate(&seg, 4);
		REQUIRE(ct_seg_count(&seg) == 4);
	}

	SECTION("Truncate to zero") {
		ct_seg_put_u32(&seg, 0x12345678);
		ct_seg_seek(&seg, 2);

		ct_seg_truncate(&seg, 0);
		REQUIRE(ct_seg_count(&seg) == 0);
		REQUIRE(ct_seg_pos(&seg) == 0);
	}
}

TEST_CASE("seg Find", "[seg][find]") {
	uint8_t  buffer[64];
	ct_seg_t seg;

	for (int i = 0; i < 64; ++i) buffer[i] = (uint8_t)i;

	SECTION("Find byte exists") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);

		REQUIRE(ct_seg_find(&seg, 0, 0) == 0);
		REQUIRE(ct_seg_find(&seg, 10, 0) == 10);
		REQUIRE(ct_seg_find(&seg, 31, 0) == 31);
	}

	SECTION("Find byte not exists") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);

		REQUIRE(ct_seg_find(&seg, 100, 0) == -1);
		REQUIRE(ct_seg_find(&seg, 32, 0) == -1);
		REQUIRE(ct_seg_find(&seg, 255, 0) == -1);
	}

	SECTION("Find from pos") {
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);
		ct_seg_seek(&seg, 10);

		REQUIRE(ct_seg_find(&seg, 5, 0) == -1);
		REQUIRE(ct_seg_find(&seg, 10, 0) == 0);
		REQUIRE(ct_seg_find(&seg, 15, 0) == 5);
		REQUIRE(ct_seg_find(&seg, 31, 0) == 21);
	}

	SECTION("Find first match") {
		buffer[10] = 0xAA;
		buffer[20] = 0xAA;
		ct_seg_from(&seg, buffer, sizeof(buffer), 32);

		REQUIRE(ct_seg_find(&seg, 0xAA, 0) == 10);

		ct_seg_seek(&seg, 15);
		REQUIRE(ct_seg_find(&seg, 0xAA, 0) == 5);
	}

	SECTION("Find in empty buffer") {
		ct_seg_init(&seg, buffer, sizeof(buffer));

		REQUIRE(ct_seg_find(&seg, 0, 0) == -1);
		REQUIRE(ct_seg_find(&seg, 0xFF, 0) == -1);
	}
}

TEST_CASE("seg Overfill", "[seg][overfill]") {
	uint8_t  buffer[64];
	ct_seg_t seg;

	memset(buffer, 0xAA, sizeof(buffer));
	ct_seg_init(&seg, buffer, sizeof(buffer));

	SECTION("Overfill does not advance pos") {
		REQUIRE(ct_seg_pos(&seg) == 0);

		REQUIRE(ct_seg_overfill(&seg, 0x00, 10) == 10);
		REQUIRE(ct_seg_pos(&seg) == 0);
		REQUIRE(ct_seg_count(&seg) == 0);

		for (int i = 0; i < 10; ++i) { REQUIRE(buffer[i] == 0x00); }
	}

	SECTION("Overfill does not change len") {
		ct_seg_put_u32(&seg, 0x12345678);
		REQUIRE(ct_seg_count(&seg) == 4);

		REQUIRE(ct_seg_overfill(&seg, 0xFF, 8) == 8);
		REQUIRE(ct_seg_count(&seg) == 4);
	}

	SECTION("Overfill capped by capacity") {
		REQUIRE(ct_seg_overfill(&seg, 0xBB, 100) == 64);

		for (int i = 0; i < 64; ++i) { REQUIRE(buffer[i] == 0xBB); }
	}

	SECTION("Overfill from buffer start") {
		ct_seg_put_u32(&seg, 0x12345678);
		ct_seg_seek(&seg, 4);

		REQUIRE(ct_seg_overfill(&seg, 0xCC, 8) == 8);

		for (int i = 0; i < 8; ++i) { REQUIRE(buffer[i] == 0xCC); }
		REQUIRE(ct_seg_pos(&seg) == 4);
	}

	SECTION("Overfill zeroing") {
		memset(buffer, 0xFF, sizeof(buffer));
		ct_seg_init(&seg, buffer, sizeof(buffer));

		REQUIRE(ct_seg_overfill(&seg, 0, 32) == 32);

		for (int i = 0; i < 32; ++i) { REQUIRE(buffer[i] == 0); }
		for (int i = 32; i < 64; ++i) { REQUIRE(buffer[i] == 0xFF); }
	}
}
