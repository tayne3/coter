#include "coter/bytes/segment.hpp"

#include <catch.hpp>

using namespace coter;

TEST_CASE("segment construction", "[segment]") {
	SECTION("default construction") {
		segment s(256);
		REQUIRE(s.capacity() == 256);
		REQUIRE(s.count() == 0);
		REQUIRE(s.is_empty());
	}

	SECTION("from seg") {
		uint8_t buf[10] = {1, 2, 3, 4, 5};

		seg view(buf, 10);
		view.commit(5);
		view.seek(2);

		segment s(view);
		REQUIRE(s.count() == 5);
		REQUIRE(s.capacity() == 5);
		REQUIRE(s.pos() == 2);
		REQUIRE(s[0] == 1);
		REQUIRE(s[4] == 5);

		const seg& ref = s;
		s              = ref;
		REQUIRE(s.capacity() == 5);
		REQUIRE(s.count() == 5);
		REQUIRE(s.pos() == 2);
		REQUIRE(s[0] == 1);
		REQUIRE(s[4] == 5);
	}
}

TEST_CASE("segment move semantics", "[segment]") {
	segment s1(256);
	s1.put<uint16_t>(100);
	s1.put<uint16_t>(200);

	segment s2(std::move(s1));
	REQUIRE(s2.count() == 4);
	REQUIRE(s2.pos() == 4);
	REQUIRE(s2.capacity() == 256);
	s2.rewind();
	REQUIRE(s2.take<uint16_t>() == 100);
	REQUIRE(s2.take<uint16_t>() == 200);

	segment s3(16);
	segment temp(128);
	temp.put<uint32_t>(0xAABBCCDD);
	s3 = std::move(temp);
	REQUIRE(s3.capacity() == 128);
	REQUIRE(s3.count() == 4);
}

TEST_CASE("segment copy semantics", "[segment]") {
	segment s1(256);
	s1.put<uint16_t>(100);
	s1.put<uint16_t>(200);

	segment s2(s1);
	REQUIRE(s2.count() == s1.count());
	REQUIRE(s2.capacity() == s1.capacity());
	REQUIRE(s2.pos() == s1.pos());
	s2.rewind();
	REQUIRE(s2.take<uint16_t>() == 100);

	segment s3(16);
	s3 = s1;
	REQUIRE(s3.count() == s1.count());
	REQUIRE(s3.capacity() == 256);

	segment& self = s3;
	s3            = self;
	REQUIRE(s3.count() == s1.count());
	REQUIRE(s3.capacity() == 256);
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
	s.put<uint16_t>(0x1234);
	s.set_endian(CT_ENDIAN_LITTLE);
	s.rewind();
	REQUIRE(s.take<uint16_t>() == 0x3412);
}

TEST_CASE("segment clone method", "[segment]") {
	uint8_t src_data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

	SECTION("clone with exact capacity and length") {
		segment s;
		s.clone(src_data, 5, 5);
		REQUIRE(s.capacity() == 5);
		REQUIRE(s.count() == 5);
		REQUIRE(s[0] == 0xAA);
		REQUIRE(s[4] == 0xEE);
	}

	SECTION("clone with larger capacity than length") {
		segment s;
		s.clone(src_data, 20, 5);
		REQUIRE(s.capacity() == 20);
		REQUIRE(s.count() == 5);
		REQUIRE(s[1] == 0xBB);
	}

	SECTION("clone with zero length") {
		segment s(100);
		s.put<uint8_t>(0xFF);

		s.clone(nullptr, 50, 0);
		REQUIRE(s.capacity() == 50);
		REQUIRE(s.count() == 0);
	}

	SECTION("self clone protection via clone method") {
		segment s;
		s.clone(src_data, 5, 5);

		s.clone(s.data(), 10, 5);
		REQUIRE(s.capacity() == 10);
		REQUIRE(s.count() == 5);
		REQUIRE(s[0] == 0xAA);
		REQUIRE(s[4] == 0xEE);
	}
}
