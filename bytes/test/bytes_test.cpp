#include "coter/bytes/bytes.h"

#include <catch.hpp>
#include <cstring>

struct BytesFixture {
	ct_bytes_t* small_bytes;
	ct_bytes_t* large_bytes;
	BytesFixture() : small_bytes(nullptr), large_bytes(nullptr) {
		small_bytes = ct_bytes_create(16);
		large_bytes = ct_bytes_create(1024);
		REQUIRE(small_bytes != nullptr);
		REQUIRE(large_bytes != nullptr);
	}
	~BytesFixture() {
		REQUIRE(small_bytes != nullptr);
		REQUIRE(large_bytes != nullptr);
		ct_bytes_destroy(small_bytes);
		ct_bytes_destroy(large_bytes);
		small_bytes = nullptr;
		large_bytes = nullptr;
	}
};

TEST_CASE("Bytes Lifecycle", "[bytes][lifecycle]") {
	SECTION("Initialization") {
		ct_bytes_t* s = nullptr;
		ct_bytes_t* l = nullptr;
		REQUIRE(s == nullptr);
		REQUIRE(l == nullptr);
		s = ct_bytes_create(16);
		l = ct_bytes_create(1024);
		REQUIRE(s != nullptr);
		REQUIRE(l != nullptr);
		REQUIRE(ct_bytes_size(s) == 0);
		REQUIRE(ct_bytes_size(l) == 0);
		REQUIRE(ct_bytes_capacity(s) == 16);
		REQUIRE(ct_bytes_capacity(l) == 1024);
		REQUIRE(ct_bytes_available(s) == 16);
		REQUIRE(ct_bytes_available(l) == 1024);
		REQUIRE(ct_bytes_write(s, "test", 4) == 4);
		REQUIRE(ct_bytes_write(l, "test", 4) == 4);
		REQUIRE(ct_bytes_size(s) == 4);
		REQUIRE(ct_bytes_size(l) == 4);
		REQUIRE(ct_bytes_capacity(s) == 16);
		REQUIRE(ct_bytes_capacity(l) == 1024);
		REQUIRE(ct_bytes_available(s) == 12);
		REQUIRE(ct_bytes_available(l) == 1020);
		ct_bytes_destroy(s);
		ct_bytes_destroy(l);
	}

	SECTION("Memory Management") {
		ct_bytes_t* s = ct_bytes_create(16);
		ct_bytes_t* l = ct_bytes_create(1024);
		REQUIRE(s != nullptr);
		REQUIRE(l != nullptr);
		ct_bytes_clear(s);
		ct_bytes_clear(l);
		ct_bytes_t* temp_bytes;
		for (int i = 0; i < 1000; ++i) {
			temp_bytes = ct_bytes_create(100);
			REQUIRE(temp_bytes != nullptr);
			ct_bytes_destroy(temp_bytes);
		}
		ct_bytes_destroy(s);
		ct_bytes_destroy(l);
	}
}

TEST_CASE_METHOD(BytesFixture, "Bytes Operations", "[bytes][ops]") {
	SECTION("Basic Read/Write") {
		REQUIRE(small_bytes != nullptr);
		ct_bytes_clear(small_bytes);
		REQUIRE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		char*  byte_buffer = ct_bytes_buffer(small_bytes);
		size_t write_len;
		write_len = ct_bytes_write(small_bytes, "A", 1);
		REQUIRE(write_len == 1);
		REQUIRE(ct_bytes_size(small_bytes) == 1);
		REQUIRE_FALSE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		REQUIRE(byte_buffer[0] == 'A');
		write_len = ct_bytes_write(small_bytes, "Hello", 5);
		REQUIRE(write_len == 5);
		REQUIRE(ct_bytes_size(small_bytes) == 6);
		REQUIRE_FALSE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		REQUIRE(std::memcmp(&byte_buffer[1], "Hello", 5) == 0);
		ct_bytes_clear(small_bytes);
		REQUIRE(ct_bytes_size(small_bytes) == 0);
		REQUIRE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
	}

	SECTION("Boundary Conditions") {
		REQUIRE(small_bytes != nullptr);
		ct_bytes_clear(small_bytes);
		REQUIRE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		size_t write_len;
		write_len = ct_bytes_write(small_bytes, "", 0);
		REQUIRE(write_len == 0);
		REQUIRE(ct_bytes_size(small_bytes) == 0);
		REQUIRE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		char large_data[16 * 2];
		std::memset(large_data, 'A', sizeof(large_data));
		write_len = ct_bytes_write(small_bytes, large_data, sizeof(large_data));
		REQUIRE(write_len == 16);
		REQUIRE(ct_bytes_size(small_bytes) == 16);
		REQUIRE_FALSE(ct_bytes_isempty(small_bytes));
		REQUIRE(ct_bytes_isfull(small_bytes));
	}

	SECTION("Sequential Operations") {
		REQUIRE(small_bytes != nullptr);
		ct_bytes_clear(small_bytes);
		REQUIRE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		char*  byte_buffer = ct_bytes_buffer(small_bytes);
		size_t write_len;
		ct_bytes_clear(small_bytes);
		write_len = ct_bytes_write(small_bytes, "Hello", 5);
		write_len += ct_bytes_write(small_bytes, " World", 6);
		REQUIRE(write_len == 11);
		REQUIRE(ct_bytes_size(small_bytes) == 11);
		REQUIRE_FALSE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		REQUIRE(std::memcmp(byte_buffer, "Hello World", 11) == 0);
		ct_bytes_clear(small_bytes);
		write_len = ct_bytes_write(small_bytes, "AB", 2);
		REQUIRE(write_len == 2);
		REQUIRE(ct_bytes_size(small_bytes) == 2);
		REQUIRE_FALSE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		REQUIRE(std::memcmp(byte_buffer, "AB", 2) == 0);
		write_len += ct_bytes_write(small_bytes, "CD", 2);
		REQUIRE(write_len == 4);
		REQUIRE(ct_bytes_size(small_bytes) == 4);
		REQUIRE_FALSE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		REQUIRE(std::memcmp(byte_buffer, "ABCD", 4) == 0);
	}

	SECTION("Buffer Overflow Handling") {
		REQUIRE(small_bytes != nullptr);
		ct_bytes_clear(small_bytes);
		REQUIRE(ct_bytes_isempty(small_bytes));
		REQUIRE_FALSE(ct_bytes_isfull(small_bytes));
		char*  byte_buffer = ct_bytes_buffer(small_bytes);
		size_t write_len;
		ct_bytes_clear(small_bytes);
		write_len = ct_bytes_write(small_bytes, "AAAAAAAAAAAAAAAA", 16);
		REQUIRE(write_len == 16);
		write_len = ct_bytes_write(small_bytes, "B", 1);
		REQUIRE(write_len == 0);
		REQUIRE(ct_bytes_size(small_bytes) == 16);
		REQUIRE_FALSE(ct_bytes_isempty(small_bytes));
		REQUIRE(ct_bytes_isfull(small_bytes));
		REQUIRE(std::memcmp(byte_buffer, "AAAAAAAAAAAAAAAA", 16) == 0);
	}

	SECTION("Seg View Operations") {
		REQUIRE(small_bytes != nullptr);
		ct_bytes_clear(small_bytes);
		ct_bytes_write(small_bytes, "0123456789", 10);
		ct_seg_t seg;
		REQUIRE(ct_bytes_seg(small_bytes, &seg, 2, 8) == 0);
		REQUIRE(ct_seg_capacity(&seg) == 14);
		REQUIRE(ct_seg_count(&seg) == 6);
		REQUIRE(ct_seg_pos(&seg) == 0);
		uint8_t buf[10];
		ct_seg_read(&seg, buf, 6);
		REQUIRE(std::memcmp((char*)buf, "234567", 6) == 0);
		ct_seg_rewind(&seg);
		ct_seg_set_u8(&seg, 0, 'A');
		char* raw_buffer = ct_bytes_buffer(small_bytes);
		REQUIRE(raw_buffer[2] == 'A');
		REQUIRE(ct_bytes_seg(small_bytes, &seg, 0, 16) == 0);
		REQUIRE(ct_seg_capacity(&seg) == 16);
		REQUIRE(ct_bytes_seg(small_bytes, &seg, 5, 5) == 0);
		REQUIRE(ct_seg_count(&seg) == 0);
		REQUIRE(ct_bytes_seg(small_bytes, &seg, 5, 4) == -1);
		REQUIRE(ct_bytes_seg(small_bytes, &seg, 0, 17) == -1);
	}
}
