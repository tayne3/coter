#include <catch.hpp>

#include "coter/bytes/seg.hpp"

TEST_CASE("Read/Write Primitives", "[seg][basic]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.put<uint8_t>(0x12);
	seg.put<uint16_t>(0x3456);
	seg.put<uint32_t>(0x789ABCDE);
	seg.put<uint64_t>(0xFEDCBA9876543210ULL);

	REQUIRE(seg.pos() == 1 + 2 + 4 + 8);

	seg.rewind();
	REQUIRE(seg.take<uint8_t>() == 0x12);
	REQUIRE(seg.take<uint16_t>() == 0x3456);
	REQUIRE(seg.take<uint32_t>() == 0x789ABCDE);
	REQUIRE(seg.take<uint64_t>() == 0xFEDCBA9876543210ULL);
}

TEST_CASE("Endianness", "[seg][config]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.set_endian(CT_ENDIAN_LITTLE);
	seg.put<uint32_t>(0x12345678);
	uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12};
	REQUIRE(std::memcmp(buffer.data(), expected_le, 4) == 0);

	seg.clear();
	seg.set_endian(CT_ENDIAN_BIG);
	seg.put<uint32_t>(0x12345678);
	uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78};
	REQUIRE(std::memcmp(buffer.data(), expected_be, 4) == 0);
}

TEST_CASE("High-Low Swap", "[seg][config]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.set_endian(CT_ENDIAN_BIG);
	seg.set_hlswap(1);
	seg.put<uint32_t>(0x11223344);
	uint8_t expected_32[] = {0x22, 0x11, 0x44, 0x33};
	REQUIRE(std::memcmp(buffer.data(), expected_32, 4) == 0);

	seg.clear();
	seg.set_endian(CT_ENDIAN_BIG);
	seg.set_hlswap(1);
	seg.put<uint64_t>(0x1122334455667788ULL);
	uint8_t expected_64[] = {0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77};
	REQUIRE(std::memcmp(buffer.data(), expected_64, 8) == 0);
}

TEST_CASE("Peek Primitives", "[seg][peek]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.put<uint8_t>(0x12);
	seg.put<uint16_t>(0x3456);
	seg.put<uint32_t>(0x789ABCDE);
	seg.put<uint64_t>(0xFEDCBA9876543210ULL);

	seg.rewind();
	REQUIRE(seg.peek<uint8_t>(0) == 0x12);
	REQUIRE(seg.peek<uint16_t>(1) == 0x3456);
	REQUIRE(seg.peek<uint32_t>(3) == 0x789ABCDE);
	REQUIRE(seg.peek<uint64_t>(7) == 0xFEDCBA9876543210ULL);
	REQUIRE(seg.pos() == 0);

	REQUIRE(seg.seek(5) == 0);
	REQUIRE(seg.peek<uint8_t>(-5) == 0x12);
	REQUIRE(seg.peek<uint16_t>(-4) == 0x3456);
}
TEST_CASE("Peek Bounds", "[seg][peek]") {
	std::array<uint8_t, 10> buffer{};
	coter::seg              seg(buffer.data(), buffer.size());

	seg.put<uint32_t>(0x12345678);
	seg.rewind();
	REQUIRE(seg.peek<uint32_t>(0) == 0x12345678);
	REQUIRE(seg.peek<uint32_t>(10) == 0);
	REQUIRE(seg.peek<uint64_t>(0) == 0);
	REQUIRE(seg.peek<uint32_t>(-1) == 0);
}

TEST_CASE("Overwrite Primitives", "[seg][overwrite]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.put<uint32_t>(0x11111111);
	seg.put<uint32_t>(0x22222222);
	seg.put<uint32_t>(0x33333333);
	auto pos_before = seg.pos();
	REQUIRE(seg.overwrite<uint32_t>(0, 0xAABBCCDD) == 0);
	REQUIRE(seg.pos() == pos_before);
	seg.rewind();
	REQUIRE(seg.take<uint32_t>() == 0xAABBCCDD);
	REQUIRE(seg.take<uint32_t>() == 0x22222222);
	REQUIRE(seg.take<uint32_t>() == 0x33333333);
}

TEST_CASE("Overwrite Endianness", "[seg][overwrite]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.set_endian(CT_ENDIAN_BIG);
	seg.put<uint32_t>(0x12345678);
	seg.set_endian(CT_ENDIAN_LITTLE);
	REQUIRE(seg.overwrite<uint32_t>(0, 0xAABBCCDD) == 0);
	uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
	REQUIRE(std::memcmp(buffer.data(), expected, 4) == 0);
	seg.rewind();
	REQUIRE(seg.take<uint32_t>() == 0xAABBCCDD);
}

TEST_CASE("Move Semantics", "[seg][move]") {
	std::array<uint8_t, 64> buffer{};
	coter::seg              seg(buffer.data(), buffer.size());
	seg.put<uint32_t>(0xDEADBEEF);
	coter::seg moved(std::move(seg));
	REQUIRE(seg.capacity() == 0);
	REQUIRE(seg.data() == nullptr);
	moved.rewind();
	REQUIRE(moved.take<uint32_t>() == 0xDEADBEEF);

	std::array<uint8_t, 64> buffer2{};
	coter::seg              seg2(buffer2.data(), buffer2.size());
	seg2 = std::move(moved);
	REQUIRE(moved.capacity() == 0);
	REQUIRE(moved.data() == nullptr);
	seg2.rewind();
	REQUIRE(seg2.take<uint32_t>() == 0xDEADBEEF);
}
