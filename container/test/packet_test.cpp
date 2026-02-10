#include "coter/container/packet.h"

#include <catch.hpp>
#include <cfloat>
#include <cstring>

#include "coter/encoding/hex.h"

#define MAX_BUFFER_SIZE 100
static ct_packet_t packet[1];
static uint8_t     buffer[MAX_BUFFER_SIZE] = {0};

static inline std::string hex_of(const uint8_t* buf, size_t n) {
	char out[1024] = {0};
	ct_hex_encode(buf, (uint32_t)n, out, sizeof(out));
	return std::string(out);
}

TEST_CASE("packet_init", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_buffer(packet) == buffer);
	REQUIRE(ct_packet_total_size(packet) == 0);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE);
	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	REQUIRE(ct_packet_total_size(packet) == 50);
	REQUIRE(ct_packet_past(packet) == 10);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 50);
}

TEST_CASE("packet_static_init", "[packet]") {
	ct_packet_t local_packet = CT_PACKET_INIT(buffer, MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_buffer(&local_packet) == buffer);
	REQUIRE(ct_packet_total_size(&local_packet) == 0);
	REQUIRE(ct_packet_past(&local_packet) == 0);
	REQUIRE(ct_packet_max(&local_packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(&local_packet) == MAX_BUFFER_SIZE);
}

TEST_CASE("packet_reset", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	REQUIRE(ct_packet_total_size(packet) == 50);
	REQUIRE(ct_packet_past(packet) == 10);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 50);
	ct_packet_reset(packet);
	REQUIRE(ct_packet_buffer(packet) == buffer);
	REQUIRE(ct_packet_total_size(packet) == 0);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE);
}

TEST_CASE("packet_clean", "[packet]") {
	std::memset(buffer, 0xFF, MAX_BUFFER_SIZE);
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	REQUIRE(ct_packet_total_size(packet) == 50);
	REQUIRE(ct_packet_past(packet) == 10);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 50);
	ct_packet_clean(packet);
	REQUIRE(ct_packet_buffer(packet) == buffer);
	REQUIRE(ct_packet_total_size(packet) == 0);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE);
	for (uint16_t i = 0; i < 50; ++i) { REQUIRE(buffer[i] == 0); }
}

TEST_CASE("packet_u8", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint8_t expected[2] = {0};
	ct_packet_put_u8(packet, 0xAA);
	REQUIRE(ct_packet_size(packet) == 1);
	REQUIRE(ct_packet_total_size(packet) == 1);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 1);
	REQUIRE(ct_packet_get_u8(packet, 0) == 0xAA);
	expected[0] = 0xAA;
	REQUIRE(hex_of(expected, 1) == hex_of(ct_packet_buffer(packet), 1));
	ct_packet_put_u8(packet, 0xBB);
	REQUIRE(ct_packet_size(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 2);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 2);
	REQUIRE(ct_packet_get_u8(packet, 0) == 0xAA);
	REQUIRE(ct_packet_get_u8(packet, 1) == 0xBB);
	expected[1] = 0xBB;
	REQUIRE(hex_of(expected, 2) == hex_of(ct_packet_buffer(packet), 2));
	REQUIRE(ct_packet_take_u8(packet) == 0xAA);
	REQUIRE(ct_packet_size(packet) == 1);
	REQUIRE(ct_packet_total_size(packet) == 2);
	REQUIRE(ct_packet_past(packet) == 1);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 2);
	REQUIRE(hex_of(expected + 1, 1) == hex_of(ct_packet_buffer(packet), 1));
	REQUIRE(ct_packet_take_u8(packet) == 0xBB);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 2);
	REQUIRE(ct_packet_past(packet) == 2);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 2);
	ct_packet_put_u8(packet, 0xCC);
	expected[0] = 0xCC;
	REQUIRE(hex_of(expected, 1) == hex_of(ct_packet_buffer(packet), 1));
	ct_packet_set_u8(packet, 0, 0xDD);
	REQUIRE(ct_packet_size(packet) == 1);
	REQUIRE(ct_packet_total_size(packet) == 3);
	REQUIRE(ct_packet_past(packet) == 2);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 3);
	expected[0] = 0xDD;
	REQUIRE(hex_of(expected, 1) == hex_of(ct_packet_buffer(packet), 1));
	REQUIRE(ct_packet_take_u8(packet) == 0xDD);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 3);
	REQUIRE(ct_packet_past(packet) == 3);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 3);
}

TEST_CASE("packet_u16", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint8_t expected[4] = {0};
	ct_packet_put_u16(packet, 0xAABB, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 2);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 2);
	REQUIRE(ct_packet_get_u16(packet, 0, CT_ENDIAN_BIG) == 0xAABB);
	expected[0] = 0xAA;
	expected[1] = 0xBB;
	REQUIRE(hex_of(expected, 2) == hex_of(ct_packet_buffer(packet), 2));
	ct_packet_put_u16(packet, 0xCCDD, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 4);
	REQUIRE(ct_packet_get_u16(packet, 0, CT_ENDIAN_BIG) == 0xAABB);
	REQUIRE(ct_packet_get_u16(packet, 2, CT_ENDIAN_LITTLE) == 0xCCDD);
	expected[2] = 0xDD;
	expected[3] = 0xCC;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	REQUIRE(ct_packet_take_u16(packet, CT_ENDIAN_BIG) == 0xAABB);
	REQUIRE(ct_packet_size(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 2);
	REQUIRE(hex_of(expected + 2, 2) == hex_of(ct_packet_buffer(packet), 2));
	REQUIRE(ct_packet_take_u16(packet, CT_ENDIAN_LITTLE) == 0xCCDD);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 4);
	ct_packet_put_u8(packet, 0xAA);
	ct_packet_put_u8(packet, 0xBB);
	expected[0] = 0xAA;
	expected[1] = 0xBB;
	REQUIRE(hex_of(expected, 2) == hex_of(ct_packet_buffer(packet), 2));
	ct_packet_set_u16(packet, 0, 0xEEFF, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 6);
	REQUIRE(ct_packet_past(packet) == 4);
	expected[0] = 0xEE;
	expected[1] = 0xFF;
	REQUIRE(hex_of(expected, 2) == hex_of(ct_packet_buffer(packet), 2));
	REQUIRE(ct_packet_take_u16(packet, CT_ENDIAN_BIG) == 0xEEFF);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 6);
	REQUIRE(ct_packet_past(packet) == 6);
	ct_packet_reset(packet);
	ct_packet_put_u16(packet, 0xCCDD, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 2);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u16(packet, 0, CT_ENDIAN_LITTLE) == 0xCCDD);
	expected[0] = 0xDD;
	expected[1] = 0xCC;
	REQUIRE(hex_of(expected, 2) == hex_of(ct_packet_buffer(packet), 2));
	ct_packet_put_u16(packet, 0xAABB, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_get_u16(packet, 0, CT_ENDIAN_LITTLE) == 0xCCDD);
	REQUIRE(ct_packet_get_u16(packet, 2, CT_ENDIAN_BIG) == 0xAABB);
	expected[2] = 0xAA;
	expected[3] = 0xBB;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	REQUIRE(ct_packet_take_u16(packet, CT_ENDIAN_LITTLE) == 0xCCDD);
	REQUIRE(ct_packet_size(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 2);
	REQUIRE(hex_of(expected + 2, 2) == hex_of(ct_packet_buffer(packet), 2));
	REQUIRE(ct_packet_take_u16(packet, CT_ENDIAN_BIG) == 0xAABB);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 4);
	ct_packet_put_u8(packet, 0xAA);
	ct_packet_put_u8(packet, 0xBB);
	expected[0] = 0xAA;
	expected[1] = 0xBB;
	REQUIRE(hex_of(expected, 2) == hex_of(ct_packet_buffer(packet), 2));
	ct_packet_set_u16(packet, 0, 0xEEFF, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 6);
	REQUIRE(ct_packet_past(packet) == 4);
	expected[0] = 0xFF;
	expected[1] = 0xEE;
	REQUIRE(hex_of(expected, 2) == hex_of(ct_packet_buffer(packet), 2));
	REQUIRE(ct_packet_take_u16(packet, CT_ENDIAN_LITTLE) == 0xEEFF);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 6);
	REQUIRE(ct_packet_past(packet) == 6);
}

TEST_CASE("packet_u32", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint8_t expected[8] = {0};
	ct_packet_put_u32(packet, 0xAABBCCDD, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u32(packet, 0, CT_ENDIAN_BIG) == 0xAABBCCDD);
	expected[0] = 0xAA;
	expected[1] = 0xBB;
	expected[2] = 0xCC;
	expected[3] = 0xDD;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	ct_packet_put_u32(packet, 0xAABBCCDD, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u32(packet, 0, CT_ENDIAN_BIG) == 0xAABBCCDD);
	REQUIRE(ct_packet_get_u32(packet, 4, CT_ENDIAN_LITTLE) == 0xAABBCCDD);
	expected[4] = 0xDD;
	expected[5] = 0xCC;
	expected[6] = 0xBB;
	expected[7] = 0xAA;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	REQUIRE(ct_packet_take_u32(packet, CT_ENDIAN_BIG) == 0xAABBCCDD);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 4);
	REQUIRE(hex_of(expected + 4, 4) == hex_of(ct_packet_buffer(packet), 4));
	REQUIRE(ct_packet_take_u32(packet, CT_ENDIAN_LITTLE) == 0xAABBCCDD);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 8);
	ct_packet_put_u8(packet, 0xAA);
	ct_packet_put_u8(packet, 0xBB);
	ct_packet_put_u8(packet, 0xCC);
	ct_packet_put_u8(packet, 0xDD);
	expected[0] = 0xAA;
	expected[1] = 0xBB;
	expected[2] = 0xCC;
	expected[3] = 0xDD;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	ct_packet_set_u32(packet, 0, 0x11223344, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 8);
	expected[0] = 0x11;
	expected[1] = 0x22;
	expected[2] = 0x33;
	expected[3] = 0x44;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	REQUIRE(ct_packet_take_u32(packet, CT_ENDIAN_BIG) == 0x11223344);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 12);
	ct_packet_reset(packet);
	ct_packet_put_u32(packet, 0xAABBCCDD, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u32(packet, 0, CT_ENDIAN_LITTLE) == 0xAABBCCDD);
	expected[0] = 0xDD;
	expected[1] = 0xCC;
	expected[2] = 0xBB;
	expected[3] = 0xAA;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	ct_packet_put_u32(packet, 0xAABBCCDD, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u32(packet, 0, CT_ENDIAN_LITTLE) == 0xAABBCCDD);
	REQUIRE(ct_packet_get_u32(packet, 4, CT_ENDIAN_BIG) == 0xAABBCCDD);
	REQUIRE(ct_packet_size(packet) == 8);
	expected[4] = 0xAA;
	expected[5] = 0xBB;
	expected[6] = 0xCC;
	expected[7] = 0xDD;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	REQUIRE(ct_packet_take_u32(packet, CT_ENDIAN_LITTLE) == 0xAABBCCDD);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 4);
	REQUIRE(hex_of(expected + 4, 4) == hex_of(ct_packet_buffer(packet), 4));
	REQUIRE(ct_packet_take_u32(packet, CT_ENDIAN_BIG) == 0xAABBCCDD);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 8);
	ct_packet_put_u8(packet, 0xAA);
	ct_packet_put_u8(packet, 0xBB);
	ct_packet_put_u8(packet, 0xCC);
	ct_packet_put_u8(packet, 0xDD);
	expected[0] = 0xAA;
	expected[1] = 0xBB;
	expected[2] = 0xCC;
	expected[3] = 0xDD;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	ct_packet_set_u32(packet, 0, 0x11223344, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 8);
	expected[0] = 0x44;
	expected[1] = 0x33;
	expected[2] = 0x22;
	expected[3] = 0x11;
	REQUIRE(hex_of(expected, 4) == hex_of(ct_packet_buffer(packet), 4));
	REQUIRE(ct_packet_take_u32(packet, CT_ENDIAN_LITTLE) == 0x11223344);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 12);
}

TEST_CASE("packet_u64", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint8_t expected[16] = {0};
	ct_packet_put_u64(packet, 0x1122334455667788ULL, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u64(packet, 0, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	expected[0] = 0x11;
	expected[1] = 0x22;
	expected[2] = 0x33;
	expected[3] = 0x44;
	expected[4] = 0x55;
	expected[5] = 0x66;
	expected[6] = 0x77;
	expected[7] = 0x88;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	ct_packet_put_u64(packet, 0x1122334455667788ULL, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 16);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u64(packet, 0, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_get_u64(packet, 8, CT_ENDIAN_LITTLE) == 0x1122334455667788ULL);
	expected[8]  = 0x88;
	expected[9]  = 0x77;
	expected[10] = 0x66;
	expected[11] = 0x55;
	expected[12] = 0x44;
	expected[13] = 0x33;
	expected[14] = 0x22;
	expected[15] = 0x11;
	REQUIRE(hex_of(expected, 16) == hex_of(ct_packet_buffer(packet), 16));
	REQUIRE(ct_packet_take_u64(packet, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 8);
	REQUIRE(hex_of(expected + 8, 8) == hex_of(ct_packet_buffer(packet), 8));
	REQUIRE(ct_packet_take_u64(packet, CT_ENDIAN_LITTLE) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 16);
	ct_packet_put_u8(packet, 0x10);
	ct_packet_put_u8(packet, 0x20);
	ct_packet_put_u8(packet, 0x30);
	ct_packet_put_u8(packet, 0x40);
	ct_packet_put_u8(packet, 0x50);
	ct_packet_put_u8(packet, 0x60);
	ct_packet_put_u8(packet, 0x70);
	ct_packet_put_u8(packet, 0x80);
	expected[0] = 0x10;
	expected[1] = 0x20;
	expected[2] = 0x30;
	expected[3] = 0x40;
	expected[4] = 0x50;
	expected[5] = 0x60;
	expected[6] = 0x70;
	expected[7] = 0x80;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	ct_packet_set_u64(packet, 0, 0x1122334455667788ULL, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 16);
	expected[0] = 0x11;
	expected[1] = 0x22;
	expected[2] = 0x33;
	expected[3] = 0x44;
	expected[4] = 0x55;
	expected[5] = 0x66;
	expected[6] = 0x77;
	expected[7] = 0x88;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	REQUIRE(ct_packet_take_u64(packet, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 24);
	ct_packet_reset(packet);
	ct_packet_put_u64(packet, 0x1122334455667788ULL, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_get_u64(packet, 0, CT_ENDIAN_LITTLE) == 0x1122334455667788ULL);
	expected[0] = 0x88;
	expected[1] = 0x77;
	expected[2] = 0x66;
	expected[3] = 0x55;
	expected[4] = 0x44;
	expected[5] = 0x33;
	expected[6] = 0x22;
	expected[7] = 0x11;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	ct_packet_put_u64(packet, 0x1122334455667788ULL, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 16);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_get_u64(packet, 0, CT_ENDIAN_LITTLE) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_get_u64(packet, 8, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	expected[8]  = 0x11;
	expected[9]  = 0x22;
	expected[10] = 0x33;
	expected[11] = 0x44;
	expected[12] = 0x55;
	expected[13] = 0x66;
	expected[14] = 0x77;
	expected[15] = 0x88;
	REQUIRE(hex_of(expected, 16) == hex_of(ct_packet_buffer(packet), 16));
	REQUIRE(ct_packet_take_u64(packet, CT_ENDIAN_LITTLE) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 8);
	REQUIRE(hex_of(expected + 8, 8) == hex_of(ct_packet_buffer(packet), 8));
	REQUIRE(ct_packet_take_u64(packet, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 16);
	ct_packet_put_u8(packet, 0x10);
	ct_packet_put_u8(packet, 0x20);
	ct_packet_put_u8(packet, 0x30);
	ct_packet_put_u8(packet, 0x40);
	ct_packet_put_u8(packet, 0x50);
	ct_packet_put_u8(packet, 0x60);
	ct_packet_put_u8(packet, 0x70);
	ct_packet_put_u8(packet, 0x80);
	expected[0] = 0x10;
	expected[1] = 0x20;
	expected[2] = 0x30;
	expected[3] = 0x40;
	expected[4] = 0x50;
	expected[5] = 0x60;
	expected[6] = 0x70;
	expected[7] = 0x80;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	ct_packet_set_u64(packet, 0, 0x1122334455667788ULL, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 16);
	expected[0] = 0x88;
	expected[1] = 0x77;
	expected[2] = 0x66;
	expected[3] = 0x55;
	expected[4] = 0x44;
	expected[5] = 0x33;
	expected[6] = 0x22;
	expected[7] = 0x11;
	REQUIRE(hex_of(expected, 8) == hex_of(ct_packet_buffer(packet), 8));
	REQUIRE(ct_packet_take_u64(packet, CT_ENDIAN_LITTLE) == 0x1122334455667788ULL);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 24);
}

TEST_CASE("packet_float", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	float test_value = FLT_MAX;
	ct_packet_put_float(packet, test_value, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_float(packet, 0, CT_ENDIAN_BIG)));
	REQUIRE(ct_packet_get_float(packet, 0, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_get_float(packet, 0, CT_ENDIAN_LITTLE) != test_value);
	ct_packet_put_float(packet, test_value, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_float(packet, 0, CT_ENDIAN_BIG)));
	REQUIRE(!std::isnan(ct_packet_get_float(packet, 4, CT_ENDIAN_LITTLE)));
	REQUIRE(ct_packet_get_float(packet, 0, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_get_float(packet, 4, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_take_float(packet, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 4);
	REQUIRE(ct_packet_take_float(packet, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 8);
	ct_packet_put_u32(packet, 0xAABBCCDD, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_get_u32(packet, 0, CT_ENDIAN_BIG) == 0xAABBCCDD);
	ct_packet_set_float(packet, 0, test_value, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 8);
	REQUIRE(ct_packet_take_float(packet, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 12);
	ct_packet_reset(packet);
	ct_packet_put_float(packet, test_value, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_float(packet, 0, CT_ENDIAN_LITTLE)));
	REQUIRE(ct_packet_get_float(packet, 0, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_get_float(packet, 0, CT_ENDIAN_BIG) != test_value);
	ct_packet_put_float(packet, test_value, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_float(packet, 0, CT_ENDIAN_LITTLE)));
	REQUIRE(!std::isnan(ct_packet_get_float(packet, 4, CT_ENDIAN_BIG)));
	REQUIRE(ct_packet_get_float(packet, 0, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_get_float(packet, 4, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_take_float(packet, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 4);
	REQUIRE(ct_packet_take_float(packet, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 8);
	ct_packet_put_u32(packet, 0xAABBCCDD, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_get_u32(packet, 0, CT_ENDIAN_BIG) == 0xAABBCCDD);
	ct_packet_set_float(packet, 0, test_value, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 4);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 8);
	REQUIRE(ct_packet_take_float(packet, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 12);
	REQUIRE(ct_packet_past(packet) == 12);
}

TEST_CASE("packet_double", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	double test_value = DBL_MAX;
	ct_packet_put_double(packet, test_value, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_double(packet, 0, CT_ENDIAN_BIG)));
	REQUIRE(ct_packet_get_double(packet, 0, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_get_double(packet, 0, CT_ENDIAN_LITTLE) != test_value);
	ct_packet_put_double(packet, test_value, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 16);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_double(packet, 0, CT_ENDIAN_BIG)));
	REQUIRE(!std::isnan(ct_packet_get_double(packet, 8, CT_ENDIAN_LITTLE)));
	REQUIRE(ct_packet_get_double(packet, 0, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_get_double(packet, 8, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_take_double(packet, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 8);
	REQUIRE(ct_packet_take_double(packet, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 16);
	ct_packet_put_u64(packet, 0x1122334455667788ULL, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_get_u64(packet, 0, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	ct_packet_set_double(packet, 0, test_value, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 16);
	REQUIRE(ct_packet_take_double(packet, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 24);
	ct_packet_reset(packet);
	ct_packet_put_double(packet, test_value, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 8);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_double(packet, 0, CT_ENDIAN_LITTLE)));
	REQUIRE(ct_packet_get_double(packet, 0, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_get_double(packet, 0, CT_ENDIAN_BIG) != test_value);
	ct_packet_put_double(packet, test_value, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_size(packet) == 16);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(!std::isnan(ct_packet_get_double(packet, 0, CT_ENDIAN_LITTLE)));
	REQUIRE(!std::isnan(ct_packet_get_double(packet, 8, CT_ENDIAN_BIG)));
	REQUIRE(ct_packet_get_double(packet, 0, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_get_double(packet, 8, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_take_double(packet, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 8);
	REQUIRE(ct_packet_take_double(packet, CT_ENDIAN_BIG) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 16);
	REQUIRE(ct_packet_past(packet) == 16);
	ct_packet_put_u64(packet, 0x1122334455667788ULL, CT_ENDIAN_BIG);
	REQUIRE(ct_packet_get_u64(packet, 0, CT_ENDIAN_BIG) == 0x1122334455667788ULL);
	ct_packet_set_double(packet, 0, test_value, CT_ENDIAN_LITTLE);
	REQUIRE(ct_packet_size(packet) == 8);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 16);
	REQUIRE(ct_packet_take_double(packet, CT_ENDIAN_LITTLE) == test_value);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 24);
	REQUIRE(ct_packet_past(packet) == 24);
}

TEST_CASE("packet_u8s", "[packet]") {
	const uint8_t test_array[5]  = {0x11, 0x22, 0x33, 0x44, 0x55};
	uint8_t       read_array[10] = {0};
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint16_t write_result = ct_packet_put_u8s(packet, test_array, 5);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 5);
	REQUIRE(ct_packet_total_size(packet) == 5);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 5);
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t read_result = ct_packet_get_u8s(packet, 0, read_array, 10);
	REQUIRE(read_result == 5);
	REQUIRE(ct_packet_size(packet) == 5);
	REQUIRE(ct_packet_total_size(packet) == 5);
	REQUIRE(ct_packet_past(packet) == 0);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	write_result = ct_packet_put_u8s(packet, test_array, 5);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 10);
	REQUIRE(ct_packet_total_size(packet) == 10);
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_u8s(packet, 0, read_array, 10);
	REQUIRE(ct_packet_size(packet) == 10);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i + 5]); }
	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			std::memset(read_array, 0, sizeof(read_array));
			uint16_t read_length = end - start;
			read_result          = ct_packet_get_u8s(packet, start, read_array, read_length);
			REQUIRE(read_length == read_result);
			REQUIRE(ct_packet_size(packet) == 10);
			for (uint16_t i = 0; i < read_length; ++i) { REQUIRE(test_array[(start + i) % 5] == read_array[i]); }
		}
	}
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_take_u8s(packet, read_array, 10);
	REQUIRE(read_result == 10);
	REQUIRE(ct_packet_size(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 10);
	REQUIRE(ct_packet_past(packet) == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i + 5]); }
}

TEST_CASE("packet_u16s", "[packet]") {
	const uint16_t test_array[5]  = {0x11AA, 0x22BB, 0x33CC, 0x44DD, 0x55EE};
	uint16_t       read_array[10] = {0};
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint16_t write_result = ct_packet_put_u16s(packet, test_array, 5, CT_ENDIAN_BIG);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 10);
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t read_result = ct_packet_get_u16s(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	write_result = ct_packet_put_u16s(packet, test_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 20);
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_u16s(packet, 0, read_array, 5, CT_ENDIAN_BIG);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_u16s(packet, 0, read_array, 10, CT_ENDIAN_LITTLE);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i + 5]); }
	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_u16s(packet, start << 1, read_array, read_length, CT_ENDIAN_BIG);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length && start + i < 5; ++i) { REQUIRE(test_array[start + i] == read_array[i]); }
			}
			if (end >= 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_u16s(packet, start << 1, read_array, read_length, CT_ENDIAN_LITTLE);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length; ++i) {
					if (start + i < 5) { continue; }
					REQUIRE(test_array[(start + i) - 5] == read_array[i]);
				}
			}
		}
	}
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t taken = ct_packet_take_u16s(packet, read_array, 5, CT_ENDIAN_BIG);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	taken = ct_packet_take_u16s(packet, read_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 0);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
}

TEST_CASE("packet_u32s", "[packet]") {
	const uint32_t test_array[5]  = {0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
	uint32_t       read_array[10] = {0};
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint16_t write_result = ct_packet_put_u32s(packet, test_array, 5, CT_ENDIAN_BIG);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 20);
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t read_result = ct_packet_get_u32s(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	write_result = ct_packet_put_u32s(packet, test_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 40);
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_u32s(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_u32s(packet, 0, read_array, 10, CT_ENDIAN_LITTLE);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i + 5]); }
	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_u32s(packet, start << 2, read_array, read_length, CT_ENDIAN_BIG);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length && start + i < 5; ++i) { REQUIRE(test_array[start + i] == read_array[i]); }
			}
			if (end >= 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_u32s(packet, start << 2, read_array, read_length, CT_ENDIAN_LITTLE);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length; ++i) {
					if (start + i < 5) { continue; }
					REQUIRE(test_array[(start + i) - 5] == read_array[i]);
				}
			}
		}
	}
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t taken = ct_packet_take_u32s(packet, read_array, 5, CT_ENDIAN_BIG);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 20);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	taken = ct_packet_take_u32s(packet, read_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 0);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
}

TEST_CASE("packet_u64s", "[packet]") {
	const uint64_t test_array[5]  = {0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
	uint64_t       read_array[10] = {0};
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint16_t write_result = ct_packet_put_u64s(packet, test_array, 5, CT_ENDIAN_BIG);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 40);
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t read_result = ct_packet_get_u64s(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	write_result = ct_packet_put_u64s(packet, test_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 80);
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_u64s(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_u64s(packet, 40, read_array, 10, CT_ENDIAN_LITTLE);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == read_array[i]); }
	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_u64s(packet, start << 3, read_array, read_length, CT_ENDIAN_BIG);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length && start + i < 5; ++i) { REQUIRE(test_array[start + i] == read_array[i]); }
			}
			if (end >= 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_u64s(packet, start << 3, read_array, read_length, CT_ENDIAN_LITTLE);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length; ++i) {
					if (start + i < 5) { continue; }
					REQUIRE(test_array[(start + i) - 5] == read_array[i]);
				}
			}
		}
	}
	uint64_t taken_array[5] = {0};
	uint16_t taken          = ct_packet_take_u64s(packet, taken_array, 5, CT_ENDIAN_BIG);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 40);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == taken_array[i]); }
	std::memset(taken_array, 0, sizeof(taken_array));
	taken = ct_packet_take_u64s(packet, taken_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 0);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(test_array[i] == taken_array[i]); }
}

TEST_CASE("packet_floats", "[packet]") {
	const float test_array[5]  = {3.14f, 1.23f, 4.56f, 7.89f, FLT_MAX};
	float       read_array[10] = {0};
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint16_t write_result = ct_packet_put_floats(packet, test_array, 5, CT_ENDIAN_BIG);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 20);
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t read_result = ct_packet_get_floats(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i] == test_array[i]); }
	write_result = ct_packet_put_floats(packet, test_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 40);
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_floats(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i] == test_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_floats(packet, 0, read_array, 10, CT_ENDIAN_LITTLE);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i + 5] == test_array[i]); }
	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_floats(packet, start << 2, read_array, read_length, CT_ENDIAN_BIG);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length && start + i < 5; ++i) { REQUIRE(read_array[i] == test_array[start + i]); }
			}
			if (end >= 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_floats(packet, start << 2, read_array, read_length, CT_ENDIAN_LITTLE);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length; ++i) {
					if (start + i < 5) { continue; }
					REQUIRE(read_array[i] == test_array[(start + i) - 5]);
				}
			}
		}
	}
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t taken = ct_packet_take_floats(packet, read_array, 5, CT_ENDIAN_BIG);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 20);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i] == test_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	taken = ct_packet_take_floats(packet, read_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 0);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i] == test_array[i]); }
}

TEST_CASE("packet_doubles", "[packet]") {
	const double test_array[5]  = {3.14, 1.23, 4.56, 7.89, DBL_MAX};
	double       read_array[10] = {0};
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	uint16_t write_result = ct_packet_put_doubles(packet, test_array, 5, CT_ENDIAN_BIG);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 40);
	std::memset(read_array, 0, sizeof(read_array));
	uint16_t read_result = ct_packet_get_doubles(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i] == test_array[i]); }
	write_result = ct_packet_put_doubles(packet, test_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(write_result == 5);
	REQUIRE(ct_packet_size(packet) == 80);
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_doubles(packet, 0, read_array, 10, CT_ENDIAN_BIG);
	REQUIRE(read_result == 10);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i] == test_array[i]); }
	std::memset(read_array, 0, sizeof(read_array));
	read_result = ct_packet_get_doubles(packet, 40, read_array, 10, CT_ENDIAN_LITTLE);
	REQUIRE(read_result == 5);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(read_array[i] == test_array[i]); }
	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_doubles(packet, start << 3, read_array, read_length, CT_ENDIAN_BIG);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length && start + i < 5; ++i) { REQUIRE(read_array[i] == test_array[start + i]); }
			}
			if (end >= 5) {
				std::memset(read_array, 0, sizeof(read_array));
				uint16_t read_length = end - start;
				read_result          = ct_packet_get_doubles(packet, start << 3, read_array, read_length, CT_ENDIAN_LITTLE);
				REQUIRE(read_length == read_result);
				for (uint16_t i = 0; i < read_length; ++i) {
					if (start + i < 5) { continue; }
					REQUIRE(read_array[i] == test_array[(start + i) - 5]);
				}
			}
		}
	}
	double   taken_array[5] = {0};
	uint16_t taken          = ct_packet_take_doubles(packet, taken_array, 5, CT_ENDIAN_BIG);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 40);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(taken_array[i] == test_array[i]); }
	std::memset(taken_array, 0, sizeof(taken_array));
	taken = ct_packet_take_doubles(packet, taken_array, 5, CT_ENDIAN_LITTLE);
	REQUIRE(taken == 5);
	REQUIRE(ct_packet_size(packet) == 0);
	for (uint16_t i = 0; i < 5; ++i) { REQUIRE(taken_array[i] == test_array[i]); }
}

TEST_CASE("packet_boundary", "[packet]") {
	uint8_t read_array[MAX_BUFFER_SIZE + 10] = {0};
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	for (uint16_t i = 0; i < MAX_BUFFER_SIZE + 10; ++i) { ct_packet_put_u8(packet, (uint8_t)i); }
	REQUIRE(ct_packet_total_size(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == 0);
	uint16_t read_length = ct_packet_get_u8s(packet, 0, read_array, MAX_BUFFER_SIZE + 10);
	REQUIRE(read_length == MAX_BUFFER_SIZE);
	ct_packet_reset(packet);
	read_length = ct_packet_get_u8s(packet, 0, read_array, MAX_BUFFER_SIZE + 10);
	REQUIRE(read_length == 0);
}

TEST_CASE("packet_offset", "[packet]") {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ct_packet_put_u8(packet, 0xAA);
	ct_packet_put_u8(packet, 0xBB);
	REQUIRE(ct_packet_total_size(packet) == 2);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 2);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 2);
	REQUIRE(ct_packet_get_u8(packet, 0) == 0xAA);
	REQUIRE(ct_packet_get_u8(packet, 1) == 0xBB);
	ct_packet_add_past(packet, 1);
	ct_packet_put_u8(packet, 0xCC);
	REQUIRE(ct_packet_total_size(packet) == 3);
	REQUIRE(ct_packet_past(packet) == 1);
	REQUIRE(ct_packet_total_size(packet) == 3);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 3);
	REQUIRE(ct_packet_get_u8(packet, 0) == 0xBB);
	REQUIRE(ct_packet_get_u8(packet, 1) == 0xCC);
	ct_packet_sub_past(packet, 1);
	ct_packet_put_u8(packet, 0xDD);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_past(packet) == 0);
	REQUIRE(ct_packet_total_size(packet) == 4);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 4);
	REQUIRE(ct_packet_get_u8(packet, 0) == 0xAA);
	REQUIRE(ct_packet_get_u8(packet, 1) == 0xBB);
	REQUIRE(ct_packet_get_u8(packet, 2) == 0xCC);
	REQUIRE(ct_packet_get_u8(packet, 3) == 0xDD);
	ct_packet_add_past(packet, 2);
	ct_packet_put_u8(packet, 0xEE);
	REQUIRE(ct_packet_total_size(packet) == 5);
	REQUIRE(ct_packet_past(packet) == 2);
	REQUIRE(ct_packet_total_size(packet) == 5);
	REQUIRE(ct_packet_max(packet) == MAX_BUFFER_SIZE);
	REQUIRE(ct_packet_available(packet) == MAX_BUFFER_SIZE - 5);
	REQUIRE(ct_packet_get_u8(packet, 0) == 0xCC);
	REQUIRE(ct_packet_get_u8(packet, 1) == 0xDD);
	REQUIRE(ct_packet_get_u8(packet, 2) == 0xEE);
}
