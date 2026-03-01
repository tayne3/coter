#include "coter/bytes/rbuf.h"

#include <catch.hpp>

#define TEST_BUF_CAPACITY 16

static uint8_t   test_buf[TEST_BUF_CAPACITY + 1];
static ct_rbuf_t rbuf;

TEST_CASE("init", "[rbuf]") {
	ct_rbuf_init(&rbuf, test_buf, TEST_BUF_CAPACITY);
	REQUIRE(ct_rbuf_capacity(&rbuf) == TEST_BUF_CAPACITY);
	REQUIRE(ct_rbuf_count(&rbuf) == 0);
	REQUIRE(ct_rbuf_remain(&rbuf) == TEST_BUF_CAPACITY);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == true);
	REQUIRE(ct_rbuf_is_full(&rbuf) == false);
	REQUIRE(memset(test_buf, 0xAA, sizeof(test_buf)));
}

TEST_CASE("static init", "[rbuf]") {
	ct_rbuf_t local_rbuf = CT_RBUF_INIT(test_buf, TEST_BUF_CAPACITY);
	REQUIRE(ct_rbuf_capacity(&local_rbuf) == TEST_BUF_CAPACITY);
	REQUIRE(ct_rbuf_count(&local_rbuf) == 0);
	REQUIRE(ct_rbuf_remain(&local_rbuf) == TEST_BUF_CAPACITY);
	REQUIRE(ct_rbuf_is_empty(&local_rbuf) == true);
	REQUIRE(ct_rbuf_is_full(&local_rbuf) == false);
	REQUIRE(local_rbuf.data == test_buf);
}

TEST_CASE("basic", "[rbuf]") {
	ct_rbuf_clear(&rbuf);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == true);

	const uint8_t input_data[]   = {1, 2, 3, 4, 5};
	uint8_t       output_data[5] = {0};

	REQUIRE(ct_rbuf_write(&rbuf, input_data, 5) == 5);
	REQUIRE(ct_rbuf_count(&rbuf) == 5);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == false);

	REQUIRE(ct_rbuf_peek(&rbuf, output_data, 5) == 5);
	REQUIRE(memcmp(input_data, output_data, 5) == 0);
	REQUIRE(ct_rbuf_count(&rbuf) == 5);  // Still there

	memset(output_data, 0, 5);
	REQUIRE(ct_rbuf_read(&rbuf, output_data, 5) == 5);
	REQUIRE(memcmp(input_data, output_data, 5) == 0);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == true);
}

TEST_CASE("wrap around", "[rbuf]") {
	ct_rbuf_clear(&rbuf);

	uint8_t fill_data[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	uint8_t sink[16]      = {0};

	REQUIRE(ct_rbuf_write(&rbuf, fill_data, 12) == 12);
	REQUIRE(ct_rbuf_read(&rbuf, sink, 12) == 12);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == true);
	REQUIRE(rbuf.head == 12);
	REQUIRE(rbuf.tail == 12);

	uint8_t wrap_data[] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7};
	REQUIRE(ct_rbuf_write(&rbuf, wrap_data, 8) == 8);
	REQUIRE(ct_rbuf_count(&rbuf) == 8);
	REQUIRE(rbuf.tail == 4);

	uint8_t peek_sink[8] = {0};
	REQUIRE(ct_rbuf_peek(&rbuf, peek_sink, 8) == 8);
	REQUIRE(memcmp(wrap_data, peek_sink, 8) == 0);

	memset(sink, 0, 16);
	REQUIRE(ct_rbuf_read(&rbuf, sink, 8) == 8);
	REQUIRE(memcmp(wrap_data, sink, 8) == 0);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == true);
	REQUIRE(rbuf.head == 4);
}

TEST_CASE("full handling", "[rbuf]") {
	ct_rbuf_clear(&rbuf);

	uint8_t fill_data[16] = {0xFF};
	REQUIRE(ct_rbuf_write(&rbuf, fill_data, 16) == 16);
	REQUIRE(ct_rbuf_is_full(&rbuf) == true);
	REQUIRE(ct_rbuf_remain(&rbuf) == 0);

	REQUIRE(ct_rbuf_write(&rbuf, fill_data, 1) == 0);

	uint8_t sink[1];
	REQUIRE(ct_rbuf_read(&rbuf, sink, 1) == 1);
	REQUIRE(ct_rbuf_is_full(&rbuf) == false);
	REQUIRE(ct_rbuf_remain(&rbuf) == 1);

	uint8_t sink16[16];
	REQUIRE(ct_rbuf_read(&rbuf, sink16, 16) == 15);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == true);
}

TEST_CASE("skip", "[rbuf]") {
	ct_rbuf_clear(&rbuf);

	uint8_t data[] = {10, 20, 30, 40, 50};
	ct_rbuf_write(&rbuf, data, 5);

	REQUIRE(ct_rbuf_remove(&rbuf, 2) == 2);
	REQUIRE(ct_rbuf_count(&rbuf) == 3);

	uint8_t rem[3];
	REQUIRE(ct_rbuf_read(&rbuf, rem, 3) == 3);
	REQUIRE(rem[0] == 30);
	REQUIRE(rem[1] == 40);
	REQUIRE(rem[2] == 50);

	REQUIRE(ct_rbuf_remove(&rbuf, 5) == 0);
}

TEST_CASE("zero copy", "[rbuf]") {
	ct_rbuf_clear(&rbuf);

	size_t   chunk_len = 0;
	uint8_t* write_ptr = ct_rbuf_write_ptr(&rbuf, &chunk_len);
	REQUIRE(write_ptr != nullptr);
	REQUIRE(chunk_len == TEST_BUF_CAPACITY);

	for (int i = 0; i < 5; ++i) { write_ptr[i] = (uint8_t)(0xB0 + i); }
	REQUIRE(ct_rbuf_commit(&rbuf, 5) == 5);
	REQUIRE(ct_rbuf_count(&rbuf) == 5);

	const uint8_t* read_ptr = ct_rbuf_read_ptr(&rbuf, &chunk_len);
	REQUIRE(read_ptr != nullptr);
	REQUIRE(chunk_len == 5);
	for (int i = 0; i < 5; ++i) { REQUIRE(read_ptr[i] == (uint8_t)(0xB0 + i)); }

	REQUIRE(ct_rbuf_remove(&rbuf, 5) == 5);
	REQUIRE(ct_rbuf_is_empty(&rbuf) == true);

	write_ptr = ct_rbuf_write_ptr(&rbuf, &chunk_len);
	REQUIRE(chunk_len == TEST_BUF_CAPACITY - 5);
	REQUIRE(ct_rbuf_commit(&rbuf, chunk_len) == chunk_len);

	size_t   chunk2_len = 0;
	uint8_t* write_ptr2 = ct_rbuf_write_ptr(&rbuf, &chunk2_len);
	REQUIRE(write_ptr2 != nullptr);
	REQUIRE(chunk2_len == 5);
	REQUIRE(ct_rbuf_commit(&rbuf, chunk2_len) == chunk2_len);

	REQUIRE(ct_rbuf_is_full(&rbuf) == true);

	const uint8_t* wrap_read = ct_rbuf_read_ptr(&rbuf, &chunk_len);
	REQUIRE(wrap_read != nullptr);
	REQUIRE(chunk_len == TEST_BUF_CAPACITY - 5);

	REQUIRE(ct_rbuf_remove(&rbuf, chunk_len) == chunk_len);
	const uint8_t* wrap_read_2 = ct_rbuf_read_ptr(&rbuf, &chunk_len);
	REQUIRE(wrap_read_2 != nullptr);
	REQUIRE(chunk_len == 5);
}
