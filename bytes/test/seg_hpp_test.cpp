#include <array>
#include <catch.hpp>
#include <cmath>
#include <limits>

#include "coter/bytes/seg.hpp"

TEST_CASE("Constructor", "[seg][ctor]") {
	std::array<uint8_t, 64> buffer{};

	SECTION("Default len=0") {
		coter::seg seg(buffer.data(), buffer.size());
		REQUIRE(seg.capacity() == buffer.size());
		REQUIRE(seg.count() == 0);
		REQUIRE(seg.pos() == 0);
		REQUIRE(seg.data() == buffer.data());
	}

	SECTION("Explicit len") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		REQUIRE(seg.capacity() == buffer.size());
		REQUIRE(seg.count() == 32);
		REQUIRE(seg.pos() == 0);
	}

	SECTION("len > cap protection") {
		coter::seg seg(buffer.data(), buffer.size(), 1000);  // len > cap
		REQUIRE(seg.count() == buffer.size());               // Should be clamped to cap
	}
}

TEST_CASE("Copy Semantics", "[seg][copy]") {
	std::array<uint8_t, 64> buffer{};
	coter::seg              seg(buffer.data(), buffer.size());
	seg.put<uint32_t>(0xDEADBEEF);

	SECTION("Copy constructor") {
		coter::seg copied(seg);
		REQUIRE(copied.capacity() == seg.capacity());
		REQUIRE(copied.count() == seg.count());
		REQUIRE(copied.pos() == seg.pos());
		REQUIRE(copied.data() == seg.data());

		copied.rewind();
		REQUIRE(copied.pos() == 0);
		REQUIRE(seg.pos() == 4);
	}

	SECTION("Copy assignment") {
		std::array<uint8_t, 64> buffer2{};
		coter::seg              seg2(buffer2.data(), buffer2.size());
		seg2 = seg;
		REQUIRE(seg2.capacity() == seg.capacity());
		REQUIRE(seg2.count() == seg.count());
		REQUIRE(seg2.data() == seg.data());

		seg2.rewind();
		REQUIRE(seg2.pos() == 0);
		REQUIRE(seg.pos() == 4);
	}
}

TEST_CASE("State Queries", "[seg][state]") {
	std::array<uint8_t, 64> buffer{};
	coter::seg              seg(buffer.data(), buffer.size());

	SECTION("Empty buffer state") {
		REQUIRE(seg.is_empty() == true);
		REQUIRE(seg.is_full() == false);
		REQUIRE(seg.capacity() == buffer.size());
		REQUIRE(seg.count() == 0);
		REQUIRE(seg.pos() == 0);
		REQUIRE(seg.readable() == 0);
		REQUIRE(seg.writable() == buffer.size());
		REQUIRE(seg.appendable() == buffer.size());
	}

	SECTION("After writes") {
		seg.put<uint32_t>(0x12345678);
		REQUIRE(seg.is_empty() == false);
		REQUIRE(seg.count() == 4);
		REQUIRE(seg.pos() == 4);
		REQUIRE(seg.readable() == 0);
		REQUIRE(seg.writable() == buffer.size() - 4);
		REQUIRE(seg.appendable() == buffer.size() - 4);

		seg.rewind();
		REQUIRE(seg.readable() == 4);
		REQUIRE(seg.writable() == buffer.size());
	}

	SECTION("Full buffer state") {
		coter::seg full_seg(buffer.data(), buffer.size(), buffer.size());
		REQUIRE(full_seg.is_empty() == false);
		REQUIRE(full_seg.is_full() == true);
		REQUIRE(full_seg.appendable() == 0);
	}
}

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

	seg.put<uint16_t>(0x1234);
	seg.set_endian(CT_ENDIAN_LITTLE);
	seg.rewind();
	REQUIRE(seg[0] == 0x12);
	REQUIRE(seg[1] == 0x34);
	REQUIRE(seg.take<uint16_t>() == 0x3412);

	seg.clear();
	seg.put<uint16_t>(0x1234);
	seg.set_endian(CT_ENDIAN_BIG);
	seg.rewind();
	REQUIRE(seg[0] == 0x34);
	REQUIRE(seg[1] == 0x12);
	REQUIRE(seg.take<uint16_t>() == 0x3412);

	seg.clear();
	seg.set_endian(CT_ENDIAN_LITTLE);
	seg.put<uint32_t>(0x12345678);
	REQUIRE(seg[0] == 0x78);
	REQUIRE(seg[1] == 0x56);
	REQUIRE(seg[2] == 0x34);
	REQUIRE(seg[3] == 0x12);

	seg.clear();
	seg.set_endian(CT_ENDIAN_BIG);
	seg.put<uint32_t>(0x12345678);
	REQUIRE(seg[0] == 0x12);
	REQUIRE(seg[1] == 0x34);
	REQUIRE(seg[2] == 0x56);
	REQUIRE(seg[3] == 0x78);
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

TEST_CASE("Set Primitives", "[seg][set]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.put<uint32_t>(0x11111111);
	seg.put<uint32_t>(0x22222222);
	seg.put<uint32_t>(0x33333333);
	auto pos_before = seg.pos();
	REQUIRE(seg.set<uint32_t>(0, 0xAABBCCDD) == 0);
	REQUIRE(seg.pos() == pos_before);
	seg.rewind();
	REQUIRE(seg.take<uint32_t>() == 0xAABBCCDD);
	REQUIRE(seg.take<uint32_t>() == 0x22222222);
	REQUIRE(seg.take<uint32_t>() == 0x33333333);
}

TEST_CASE("Set Endianness", "[seg][set]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	seg.set_endian(CT_ENDIAN_BIG);
	seg.put<uint32_t>(0x12345678);
	seg.set_endian(CT_ENDIAN_LITTLE);
	REQUIRE(seg.set<uint32_t>(0, 0xAABBCCDD) == 0);
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

TEST_CASE("Position Control", "[seg][position]") {
	std::array<uint8_t, 64> buffer{};

	SECTION("seek/reseek") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		REQUIRE(seg.seek(16) == 0);
		REQUIRE(seg.pos() == 16);
		REQUIRE(seg.seek(33) == -1);

		REQUIRE(seg.reseek(0) == 0);
		REQUIRE(seg.pos() == 32);
		REQUIRE(seg.reseek(10) == 0);
		REQUIRE(seg.pos() == 22);
		REQUIRE(seg.reseek(33) == -1);
	}

	SECTION("skip/commit") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		REQUIRE(seg.skip(10) == 10);
		REQUIRE(seg.pos() == 10);
		REQUIRE(seg.skip(100) == 22);
		REQUIRE(seg.pos() == 32);

		coter::seg seg2(buffer.data(), buffer.size());
		REQUIRE(seg2.commit(10) == 10);
		REQUIRE(seg2.pos() == 10);
		REQUIRE(seg2.count() == 10);
	}

	SECTION("clear/rewind") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		seg.seek(16);

		seg.rewind();
		REQUIRE(seg.pos() == 0);
		REQUIRE(seg.count() == 32);

		seg.clear();
		REQUIRE(seg.pos() == 0);
		REQUIRE(seg.count() == 0);
	}
}

TEST_CASE("IO Operations", "[seg][io]") {
	std::array<uint8_t, 64> buffer{};
	coter::seg              seg(buffer.data(), buffer.size());

	SECTION("write/read bytes") {
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
		REQUIRE(seg.write(data, 5) == 5);
		REQUIRE(seg.pos() == 5);
		REQUIRE(seg.count() == 5);

		seg.rewind();
		uint8_t out[5] = {0};
		REQUIRE(seg.read(out, 5) == 5);
		REQUIRE(std::memcmp(out, data, 5) == 0);
	}

	SECTION("peek bytes") {
		uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
		seg.write(data, 4);
		seg.rewind();

		uint8_t out[4] = {0};
		REQUIRE(seg.peek(0, out, 4) == 4);
		REQUIRE(std::memcmp(out, data, 4) == 0);
		REQUIRE(seg.pos() == 0);

		uint8_t out2[2] = {0};
		REQUIRE(seg.peek(2, out2, 2) == 2);
		REQUIRE(out2[0] == 0xCC);
		REQUIRE(out2[1] == 0xDD);
	}

	SECTION("fill pattern") {
		REQUIRE(seg.fill(0xAA, 10) == 10);
		REQUIRE(seg.pos() == 10);
		REQUIRE(seg.count() == 10);
		for (int i = 0; i < 10; ++i) { REQUIRE(buffer[i] == 0xAA); }
	}

	SECTION("partial operations") {
		coter::seg small_seg(buffer.data(), 4);
		uint8_t    data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		REQUIRE(small_seg.write(data, 8) == 4);

		small_seg.rewind();
		uint8_t out[8] = {0};
		REQUIRE(small_seg.read(out, 8) == 4);
	}
}

TEST_CASE("Handle Access", "[seg][handle]") {
	std::array<uint8_t, 64> buffer{};
	coter::seg              seg(buffer.data(), buffer.size(), 32);

	SECTION("non-const handle") {
		ct_seg_t* h = seg.handle();
		REQUIRE(h != nullptr);
		REQUIRE(h->data == buffer.data());
		REQUIRE(h->cap == buffer.size());
		REQUIRE(h->len == 32);
	}

	SECTION("const handle") {
		const coter::seg& cseg = seg;
		const ct_seg_t*   h    = cseg.handle();
		REQUIRE(h != nullptr);
		REQUIRE(h->data == buffer.data());
	}
}

TEST_CASE("Getter Methods", "[seg][config]") {
	std::array<uint8_t, 64> buffer{};
	coter::seg              seg(buffer.data(), buffer.size());

	SECTION("default values") {
		REQUIRE(seg.get_endian() == CT_ENDIAN_BIG);
		REQUIRE(seg.get_hlswap() == 0);
	}

	SECTION("get after set") {
		seg.set_endian(CT_ENDIAN_LITTLE);
		REQUIRE(seg.get_endian() == CT_ENDIAN_LITTLE);

		seg.set_hlswap(1);
		REQUIRE(seg.get_hlswap() == 1);
	}
}

TEST_CASE("View Operations", "[seg][view]") {
	std::array<uint8_t, 64> buffer{};
	for (size_t i = 0; i < 64; ++i) buffer[i] = static_cast<uint8_t>(i);

	SECTION("since valid range") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		auto       result = seg.since(8, 24);

		REQUIRE(result.has_value());
		REQUIRE(result->data() == buffer.data() + 8);
		REQUIRE(result->capacity() == buffer.size() - 8);
		REQUIRE(result->count() == 16);
		REQUIRE(result->pos() == 0);
	}

	SECTION("since default arguments") {
		coter::seg seg(buffer.data(), buffer.size(), 32);

		// Case 1: since() -> [0, 32]
		auto res1 = seg.since();
		REQUIRE(res1.has_value());
		REQUIRE(res1->data() == buffer.data());
		REQUIRE(res1->count() == 32);

		// Case 2: since(10) -> [10, 32]
		auto res2 = seg.since(10);
		REQUIRE(res2.has_value());
		REQUIRE(res2->data() == buffer.data() + 10);
		REQUIRE(res2->count() == 22);

		// Case 3: since(10, 0) -> [10, 32] (explicit 0 for end)
		auto res3 = seg.since(10, 0);
		REQUIRE(res3.has_value());
		REQUIRE(res3->data() == buffer.data() + 10);
		REQUIRE(res3->count() == 22);
	}

	SECTION("since invalid range returns nullopt") {
		coter::seg seg(buffer.data(), buffer.size(), 32);

		REQUIRE_FALSE(seg.since(24, 8).has_value());   // end < start
		REQUIRE_FALSE(seg.since(0, 100).has_value());  // end > cap
	}

	SECTION("since empty range") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		auto       result = seg.since(10, 10);

		REQUIRE(result.has_value());
		REQUIRE(result->count() == 0);
		REQUIRE(result->data() == buffer.data() + 10);
	}

	SECTION("since inherits config") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		seg.set_endian(CT_ENDIAN_LITTLE);
		seg.set_hlswap(1);

		auto result = seg.since(0, 16);
		REQUIRE(result.has_value());
		REQUIRE(result->get_endian() == CT_ENDIAN_LITTLE);
		REQUIRE(result->get_hlswap() == 1);
	}

	SECTION("readable_since") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		seg.seek(8);

		auto result = seg.readable_since();
		REQUIRE(result.has_value());
		REQUIRE(result->data() == buffer.data() + 8);
		REQUIRE(result->count() == 24);  // [pos, len] = [8, 32]
	}

	SECTION("writable_since") {
		coter::seg seg(buffer.data(), buffer.size(), 32);
		seg.seek(8);

		auto result = seg.writable_since();
		REQUIRE(result.has_value());
		REQUIRE(result->data() == buffer.data() + 8);
		REQUIRE(result->count() == buffer.size() - 8);  // [pos, cap] = [8, 64]
	}

	SECTION("compact") {
		coter::seg seg(buffer.data(), buffer.size(), 16);
		seg.seek(8);

		seg.compact();
		REQUIRE(seg.pos() == 0);
		REQUIRE(seg.count() == 8);
		REQUIRE(buffer[0] == 8);
		REQUIRE(buffer[7] == 15);
	}
}

TEST_CASE("Signed Types", "[seg][types]") {
	std::array<uint8_t, 128> buffer{};
	coter::seg               seg(buffer.data(), buffer.size());

	SECTION("int8_t put/take") {
		seg.put<int8_t>(0);
		seg.put<int8_t>(127);
		seg.put<int8_t>(-128);
		seg.put<int8_t>(-1);

		seg.rewind();
		REQUIRE(seg.take<int8_t>() == 0);
		REQUIRE(seg.take<int8_t>() == 127);
		REQUIRE(seg.take<int8_t>() == -128);
		REQUIRE(seg.take<int8_t>() == -1);
	}

	SECTION("int16_t put/take") {
		seg.put<int16_t>(0);
		seg.put<int16_t>(std::numeric_limits<int16_t>::max());
		seg.put<int16_t>(std::numeric_limits<int16_t>::min());
		seg.put<int16_t>(-1);

		seg.rewind();
		REQUIRE(seg.take<int16_t>() == 0);
		REQUIRE(seg.take<int16_t>() == std::numeric_limits<int16_t>::max());
		REQUIRE(seg.take<int16_t>() == std::numeric_limits<int16_t>::min());
		REQUIRE(seg.take<int16_t>() == -1);
	}

	SECTION("int32_t put/take") {
		seg.put<int32_t>(0);
		seg.put<int32_t>(std::numeric_limits<int32_t>::max());
		seg.put<int32_t>(std::numeric_limits<int32_t>::min());
		seg.put<int32_t>(-1);

		seg.rewind();
		REQUIRE(seg.take<int32_t>() == 0);
		REQUIRE(seg.take<int32_t>() == std::numeric_limits<int32_t>::max());
		REQUIRE(seg.take<int32_t>() == std::numeric_limits<int32_t>::min());
		REQUIRE(seg.take<int32_t>() == -1);
	}

	SECTION("int64_t put/take") {
		seg.put<int64_t>(0);
		seg.put<int64_t>(std::numeric_limits<int64_t>::max());
		seg.put<int64_t>(std::numeric_limits<int64_t>::min());
		seg.put<int64_t>(-1);

		seg.rewind();
		REQUIRE(seg.take<int64_t>() == 0);
		REQUIRE(seg.take<int64_t>() == std::numeric_limits<int64_t>::max());
		REQUIRE(seg.take<int64_t>() == std::numeric_limits<int64_t>::min());
		REQUIRE(seg.take<int64_t>() == -1);
	}

	SECTION("signed peek") {
		seg.put<int8_t>(std::numeric_limits<int8_t>::min());
		seg.put<int16_t>(std::numeric_limits<int16_t>::min());
		seg.put<int32_t>(std::numeric_limits<int32_t>::min());
		seg.put<int64_t>(std::numeric_limits<int64_t>::min());

		seg.rewind();
		REQUIRE(seg.peek<int8_t>(0) == std::numeric_limits<int8_t>::min());
		REQUIRE(seg.peek<int16_t>(1) == std::numeric_limits<int16_t>::min());
		REQUIRE(seg.peek<int32_t>(3) == std::numeric_limits<int32_t>::min());
		REQUIRE(seg.peek<int64_t>(7) == std::numeric_limits<int64_t>::min());
	}

	SECTION("signed set") {
		seg.put<int32_t>(0);
		REQUIRE(seg.set<int32_t>(0, -12345) == 0);
		seg.rewind();
		REQUIRE(seg.take<int32_t>() == -12345);

		seg.clear();
		seg.put<int8_t>(0);
		REQUIRE(seg.set<int8_t>(0, -100) == 0);
		seg.rewind();
		REQUIRE(seg.take<int8_t>() == -100);
	}
}

TEST_CASE("Floating Point Types", "[seg][types]") {
	std::array<uint8_t, 128> buffer{};
	coter::seg               seg(buffer.data(), buffer.size());

	SECTION("float put/take/peek/set") {
		seg.put<float>(0.0f);
		seg.put<float>(3.14159f);
		seg.put<float>(-123.456f);

		seg.rewind();
		REQUIRE(seg.take<float>() == 0.0f);
		REQUIRE(seg.take<float>() == Catch::Detail::Approx(3.14159f));
		REQUIRE(seg.take<float>() == Catch::Detail::Approx(-123.456f));

		seg.rewind();
		REQUIRE(seg.peek<float>(0) == 0.0f);
		REQUIRE(seg.peek<float>(4) == Catch::Detail::Approx(3.14159f));

		REQUIRE(seg.set<float>(0, 99.9f) == 0);
		REQUIRE(seg.peek<float>(0) == Catch::Detail::Approx(99.9f));
	}

	SECTION("double put/take/peek/set") {
		seg.put<double>(0.0);
		seg.put<double>(3.141592653589793);
		seg.put<double>(-123456.789012);

		seg.rewind();
		REQUIRE(seg.take<double>() == 0.0);
		REQUIRE(seg.take<double>() == Catch::Detail::Approx(3.141592653589793));
		REQUIRE(seg.take<double>() == Catch::Detail::Approx(-123456.789012));

		seg.rewind();
		REQUIRE(seg.peek<double>(0) == 0.0);
		REQUIRE(seg.peek<double>(8) == Catch::Detail::Approx(3.141592653589793));

		REQUIRE(seg.set<double>(0, 99.999) == 0);
		REQUIRE(seg.peek<double>(0) == Catch::Detail::Approx(99.999));
	}

	SECTION("special values") {
		seg.put<float>(std::numeric_limits<float>::infinity());
		seg.put<float>(-std::numeric_limits<float>::infinity());
		seg.put<double>(std::numeric_limits<double>::infinity());
		seg.put<double>(-0.0);

		seg.rewind();
		REQUIRE(std::isinf(seg.take<float>()));
		REQUIRE(std::isinf(seg.take<float>()));
		REQUIRE(std::isinf(seg.take<double>()));

		double negative_zero = seg.take<double>();
		REQUIRE(negative_zero == 0.0);
		REQUIRE(std::signbit(negative_zero));
	}
}

TEST_CASE("Set All Types", "[seg][set]") {
	std::array<uint8_t, 128> buffer{};
	coter::seg               seg(buffer.data(), buffer.size());

	SECTION("u8/i8") {
		seg.fill(0, 16);
		REQUIRE(seg.set<uint8_t>(0, 0xAB) == 0);
		REQUIRE(seg.set<int8_t>(1, static_cast<int8_t>(-100)) == 0);

		seg.rewind();
		REQUIRE(seg.take<uint8_t>() == 0xAB);
		REQUIRE(seg.take<int8_t>() == static_cast<int8_t>(-100));
	}

	SECTION("u16/i16") {
		seg.fill(0, 16);
		REQUIRE(seg.set<uint16_t>(0, 0xABCD) == 0);
		REQUIRE(seg.set<int16_t>(2, static_cast<int16_t>(-12345)) == 0);

		seg.rewind();
		REQUIRE(seg.take<uint16_t>() == 0xABCD);
		REQUIRE(seg.take<int16_t>() == static_cast<int16_t>(-12345));
	}

	SECTION("u32/i32/float") {
		seg.fill(0, 16);
		REQUIRE(seg.set<uint32_t>(0, 0xDEADBEEF) == 0);
		REQUIRE(seg.set<int32_t>(4, static_cast<int32_t>(-123456789)) == 0);
		REQUIRE(seg.set<float>(8, 3.14f) == 0);

		seg.rewind();
		REQUIRE(seg.take<uint32_t>() == 0xDEADBEEF);
		REQUIRE(seg.take<int32_t>() == static_cast<int32_t>(-123456789));
		REQUIRE(seg.take<float>() == Catch::Detail::Approx(3.14f));
	}

	SECTION("u64/i64/double") {
		seg.fill(0, 32);
		REQUIRE(seg.set<uint64_t>(0, 0xDEADBEEFCAFEBABEULL) == 0);
		REQUIRE(seg.set<int64_t>(8, static_cast<int64_t>(-123456789012345LL)) == 0);
		REQUIRE(seg.set<double>(16, 3.141592653589793) == 0);

		seg.rewind();
		REQUIRE(seg.take<uint64_t>() == 0xDEADBEEFCAFEBABEULL);
		REQUIRE(seg.take<int64_t>() == static_cast<int64_t>(-123456789012345LL));
		REQUIRE(seg.take<double>() == Catch::Detail::Approx(3.141592653589793));
	}

	SECTION("boundary errors") {
		seg.fill(0, 4);
		REQUIRE(seg.set<uint32_t>(4, 0x12345678) == -1);
		REQUIRE(seg.set<uint64_t>(0, 0x1234567890ABCDEFULL) == -1);
	}
}

TEST_CASE("Get Operations", "[seg][get]") {
	std::array<uint8_t, 4096> buffer{};
	coter::seg                seg(buffer.data(), buffer.size());

	SECTION("Get Primitives") {
		seg.put<uint8_t>(0x12);
		seg.put<uint16_t>(0x3456);
		seg.put<uint32_t>(0x789ABCDE);
		seg.put<uint64_t>(0xFEDCBA9876543210ULL);

		REQUIRE(seg.get<uint8_t>(0) == 0x12);
		REQUIRE(seg.get<uint16_t>(1) == 0x3456);
		REQUIRE(seg.get<uint32_t>(3) == 0x789ABCDE);
		REQUIRE(seg.get<uint64_t>(7) == 0xFEDCBA9876543210ULL);

		REQUIRE(seg.pos() == 15);
	}

	SECTION("Get does not change pos or count") {
		seg.put<uint32_t>(0x12345678);
		seg.rewind();

		auto pos_before   = seg.pos();
		auto count_before = seg.count();

		seg.get<uint32_t>(0);

		REQUIRE(seg.pos() == pos_before);
		REQUIRE(seg.count() == count_before);
	}

	SECTION("Get Bounds") {
		coter::seg small_seg(buffer.data(), 10);
		small_seg.put<uint32_t>(0x12345678);

		REQUIRE(small_seg.get<uint32_t>(0) == 0x12345678);
		REQUIRE(small_seg.get<uint32_t>(10) == 0);
		REQUIRE(small_seg.get<uint64_t>(0) == 0);
		REQUIRE(small_seg.get<uint8_t>(100) == 0);
	}

	SECTION("Get Endianness Big") {
		seg.set_endian(CT_ENDIAN_BIG);
		seg.put<uint32_t>(0x11223344);

		REQUIRE(seg.get<uint8_t>(0) == 0x11);
		REQUIRE(seg.get<uint8_t>(1) == 0x22);
		REQUIRE(seg.get<uint8_t>(2) == 0x33);
		REQUIRE(seg.get<uint8_t>(3) == 0x44);
	}

	SECTION("Get Endianness Little") {
		seg.set_endian(CT_ENDIAN_LITTLE);
		seg.put<uint32_t>(0x11223344);

		REQUIRE(seg.get<uint8_t>(0) == 0x44);
		REQUIRE(seg.get<uint8_t>(1) == 0x33);
		REQUIRE(seg.get<uint8_t>(2) == 0x22);
		REQUIRE(seg.get<uint8_t>(3) == 0x11);
	}

	SECTION("Get All Types") {
		seg.put<uint8_t>(0xAB);
		seg.put<uint16_t>(0xCDEF);
		seg.put<uint32_t>(0x12345678);
		seg.put<uint64_t>(0xFEDCBA9876543210ULL);

		REQUIRE(seg.get<uint8_t>(0) == 0xAB);
		REQUIRE(seg.get<uint16_t>(1) == 0xCDEF);
		REQUIRE(seg.get<uint32_t>(3) == 0x12345678);
		REQUIRE(seg.get<uint64_t>(7) == 0xFEDCBA9876543210ULL);
	}

	SECTION("Get signed types") {
		seg.put<int8_t>(-100);
		seg.put<int16_t>(-12345);
		seg.put<int32_t>(-123456789);
		seg.put<int64_t>(-123456789012345LL);

		REQUIRE(seg.get<int8_t>(0) == -100);
		REQUIRE(seg.get<int16_t>(1) == -12345);
		REQUIRE(seg.get<int32_t>(3) == -123456789);
		REQUIRE(seg.get<int64_t>(7) == -123456789012345LL);
	}

	SECTION("Get float/double") {
		seg.put<float>(3.14159f);
		seg.put<double>(3.141592653589793);

		REQUIRE(seg.get<float>(0) == Catch::Detail::Approx(3.14159f));
		REQUIRE(seg.get<double>(4) == Catch::Detail::Approx(3.141592653589793));
	}
}
