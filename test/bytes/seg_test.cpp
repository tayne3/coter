#include <array>
#include <cstdint>
#include <cstring>

#include "catch.hpp"
#include "coter/bytes/seg.h"

TEST_CASE("Read/Write Primitives", "[seg][basic]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	seg.putU8(0x12);
	seg.putU16(0x3456);
	seg.putU32(0x789ABCDE);
	seg.putU64(0xFEDCBA9876543210ULL);

	REQUIRE(seg.pos() == 1 + 2 + 4 + 8);

	seg.rewind();
	REQUIRE(seg.takeU8() == 0x12);
	REQUIRE(seg.takeU16() == 0x3456);
	REQUIRE(seg.takeU32() == 0x789ABCDE);
	REQUIRE(seg.takeU64() == 0xFEDCBA9876543210ULL);
}

TEST_CASE("Read/Write Arrays", "[seg][basic]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	uint32_t data[] = {0x11223344, 0x55667788};
	seg.putArr32(data, 2);

	seg.rewind();
	uint32_t out[2]{};
	REQUIRE(seg.takeArr32(out, 2) == 0);
	REQUIRE(out[0] == 0x11223344);
	REQUIRE(out[1] == 0x55667788);
}

TEST_CASE("Endianness", "[seg][config]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	seg.setEndian(CT_ENDIAN_LITTLE);
	seg.putU32(0x12345678);
	uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12};
	REQUIRE(std::memcmp(buffer.data(), expected_le, 4) == 0);

	seg.clear();
	seg.setEndian(CT_ENDIAN_BIG);
	seg.putU32(0x12345678);
	uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78};
	REQUIRE(std::memcmp(buffer.data(), expected_be, 4) == 0);
}

TEST_CASE("High-Low Swap", "[seg][config]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	seg.setEndian(CT_ENDIAN_BIG);
	seg.setHlswap(1);
	seg.putU32(0x11223344);
	uint8_t expected_32[] = {0x22, 0x11, 0x44, 0x33};
	REQUIRE(std::memcmp(buffer.data(), expected_32, 4) == 0);

	seg.clear();
	seg.setEndian(CT_ENDIAN_BIG);
	seg.setHlswap(1);
	seg.putU64(0x1122334455667788ULL);
	uint8_t expected_64[] = {0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77};
	REQUIRE(std::memcmp(buffer.data(), expected_64, 8) == 0);
}

TEST_CASE("Array Swap", "[seg][config]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	seg.setEndian(CT_ENDIAN_LITTLE);
	seg.setHlswap(1);
	uint32_t data[] = {0x11223344};
	seg.putArr32(data, 1);
	uint8_t expected[] = {0x33, 0x44, 0x11, 0x22};
	REQUIRE(std::memcmp(buffer.data(), expected, 4) == 0);

	seg.rewind();
	uint32_t out[1]{};
	REQUIRE(seg.takeArr32(out, 1) == 0);
	REQUIRE(out[0] == 0x11223344);
}

TEST_CASE("Peek Primitives", "[seg][peek]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	seg.putU8(0x12);
	seg.putU16(0x3456);
	seg.putU32(0x789ABCDE);
	seg.putU64(0xFEDCBA9876543210ULL);

	seg.rewind();
	REQUIRE(seg.peekU8(0) == 0x12);
	REQUIRE(seg.peekU16(1) == 0x3456);
	REQUIRE(seg.peekU32(3) == 0x789ABCDE);
	REQUIRE(seg.peekU64(7) == 0xFEDCBA9876543210ULL);
	REQUIRE(seg.pos() == 0);

	REQUIRE(seg.seek(5) == 0);
	REQUIRE(seg.peekU8(-5) == 0x12);
	REQUIRE(seg.peekU16(-4) == 0x3456);
}

TEST_CASE("Peek Arrays", "[seg][peek]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	uint32_t data[] = {0x11223344, 0x55667788, 0x99AABBCC};
	seg.putArr32(data, 3);

	seg.rewind();
	uint32_t out[2]{};
	REQUIRE(seg.peekArr32(0, out, 2) == 0);
	REQUIRE(out[0] == 0x11223344);
	REQUIRE(out[1] == 0x55667788);
	REQUIRE(seg.pos() == 0);
	REQUIRE(seg.peekArr32(4, out, 2) == 0);
	REQUIRE(out[0] == 0x55667788);
	REQUIRE(out[1] == 0x99AABBCC);
}

TEST_CASE("Peek Bounds", "[seg][peek]") {
	std::array<uint8_t, 10> buffer{};
	coter::Seg              seg(buffer.data(), buffer.size());

	seg.putU32(0x12345678);
	seg.rewind();
	REQUIRE(seg.peekU32(0) == 0x12345678);
	REQUIRE(seg.peekU32(10) == 0);
	REQUIRE(seg.peekU64(0) == 0);
	REQUIRE(seg.peekU32(-1) == 0);
	uint32_t out[2]{};
	REQUIRE(seg.peekArr32(0, out, 2) == -1);
}

TEST_CASE("Overwrite Primitives", "[seg][overwrite]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	seg.putU32(0x11111111);
	seg.putU32(0x22222222);
	seg.putU32(0x33333333);
	auto pos_before = seg.pos();
	REQUIRE(seg.overwriteU32(0, 0xAABBCCDD) == 0);
	REQUIRE(seg.pos() == pos_before);
	seg.rewind();
	REQUIRE(seg.takeU32() == 0xAABBCCDD);
	REQUIRE(seg.takeU32() == 0x22222222);
	REQUIRE(seg.takeU32() == 0x33333333);
}

TEST_CASE("Overwrite Arrays", "[seg][overwrite]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	uint32_t data[] = {0x11111111, 0x22222222, 0x33333333};
	seg.putArr32(data, 3);
	uint32_t new_data[] = {0xAAAAAAAA, 0xBBBBBBBB};
	REQUIRE(seg.overwriteArr32(4, new_data, 2) == 0);
	seg.rewind();
	uint32_t out[3]{};
	REQUIRE(seg.takeArr32(out, 3) == 0);
	REQUIRE(out[0] == 0x11111111);
	REQUIRE(out[1] == 0xAAAAAAAA);
	REQUIRE(out[2] == 0xBBBBBBBB);
}

TEST_CASE("Overwrite Endianness", "[seg][overwrite]") {
	std::array<uint8_t, 4096> buffer{};
	coter::Seg                seg(buffer.data(), buffer.size());

	seg.setEndian(CT_ENDIAN_BIG);
	seg.putU32(0x12345678);
	seg.setEndian(CT_ENDIAN_LITTLE);
	REQUIRE(seg.overwriteU32(0, 0xAABBCCDD) == 0);
	uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
	REQUIRE(std::memcmp(buffer.data(), expected, 4) == 0);
	seg.rewind();
	REQUIRE(seg.takeU32() == 0xAABBCCDD);
}

TEST_CASE("Overwrite Bounds", "[seg][overwrite]") {
	std::array<uint8_t, 10> buffer{};
	coter::Seg              seg(buffer.data(), buffer.size());

	seg.putU32(0x12345678);
	REQUIRE(seg.overwriteU8(0, 0xAA) == 0);
	REQUIRE(seg.overwriteU32(10, 0xBBBBBBBB) == -1);
	REQUIRE(seg.overwriteU64(0, 0x1122334455667788ULL) == -1);
	uint32_t data[] = {0x11111111, 0x22222222};
	REQUIRE(seg.overwriteArr32(0, data, 2) == -1);
}

TEST_CASE("Move Semantics", "[seg][move]") {
	std::array<uint8_t, 64> buffer{};
	coter::Seg              seg(buffer.data(), buffer.size());
	seg.putU32(0xDEADBEEF);
	coter::Seg moved(std::move(seg));
	REQUIRE(seg.capacity() == 0);
	REQUIRE(seg.data() == nullptr);
	moved.rewind();
	REQUIRE(moved.takeU32() == 0xDEADBEEF);

	std::array<uint8_t, 64> buffer2{};
	coter::Seg              seg2(buffer2.data(), buffer2.size());
	seg2 = std::move(moved);
	REQUIRE(moved.capacity() == 0);
	REQUIRE(moved.data() == nullptr);
	seg2.rewind();
	REQUIRE(seg2.takeU32() == 0xDEADBEEF);
}
