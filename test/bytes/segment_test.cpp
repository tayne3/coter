#include "coter/bytes/segment.hpp"

#include <catch.hpp>

using namespace coter;

TEST_CASE("segment construction", "[segment]") {
	SECTION("default construction") {
		segment s(256);
		REQUIRE(s.capacity() == 256);
		REQUIRE(s.count() == 0);
		REQUIRE(s.isEmpty());
	}

	SECTION("from seg") {
		uint8_t buf[10] = {1, 2, 3, 4, 5};
		seg     view(buf, 10);
		view.commit(5);

		segment s(view);
		REQUIRE(s.count() == 5);
		REQUIRE(s.data()[0] == 1);
		REQUIRE(s.data()[4] == 5);
	}
}

TEST_CASE("segment move semantics", "[segment]") {
	segment s1(256);
	s1.put<uint16_t>(100);
	s1.put<uint16_t>(200);

	segment s2(std::move(s1));
	REQUIRE(s2.count() == 4);
	s2.rewind();
	REQUIRE(s2.take<uint16_t>() == 100);
	REQUIRE(s2.take<uint16_t>() == 200);
}

TEST_CASE("segment copy semantics", "[segment]") {
	segment s1(256);
	s1.put<uint16_t>(100);
	s1.put<uint16_t>(200);

	segment s2(s1);
	REQUIRE(s2.count() == s1.count());
	s2.rewind();
	REQUIRE(s2.take<uint16_t>() == 100);
}

TEST_CASE("segment reserve", "[segment]") {
	segment s(10);
	s.put<uint32_t>(0x12345678);

	s.reserve(100);
	REQUIRE(s.capacity() >= 100);
	REQUIRE(s.count() == 4);

	s.rewind();
	REQUIRE(s.take<uint32_t>() == 0x12345678);
}

TEST_CASE("segment put/take", "[segment]") {
	segment s(256);

	s.put<uint8_t>(0xAB);
	s.put<uint16_t>(0x1234);
	s.put<uint32_t>(0x56789ABC);
	s.put<float>(3.14f);

	s.rewind();
	REQUIRE(s.take<uint8_t>() == 0xAB);
	REQUIRE(s.take<uint16_t>() == 0x1234);
	REQUIRE(s.take<uint32_t>() == 0x56789ABC);
	REQUIRE(s.take<float>() == Approx(3.14f));
}

TEST_CASE("segment shrink_to_fit", "[segment]") {
	segment s(256);
	s.put<uint32_t>(100);
	s.put<uint32_t>(200);

	s.shrink_to_fit();
	REQUIRE(s.capacity() == 8);
	REQUIRE(s.count() == 8);
}

TEST_CASE("segment endianness", "[segment]") {
	segment s(256);
	s.setEndian(CT_ENDIAN_BIG);

	s.put<uint16_t>(0x1234);

	REQUIRE(s.data()[0] == 0x12);
	REQUIRE(s.data()[1] == 0x34);
}
