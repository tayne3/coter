/**
 * @file rbuf_test.cpp
 * @brief Ring buffer tests.
 */
#include "coter/bytes/rbuf.h"

#include <cstring>

#include "coter/testing/doctest.h"


#define TEST_BUF_CAPACITY 16

static uint8_t   s_test_buf[TEST_BUF_CAPACITY + 1];
static ct_rbuf_t s_rbuf;

TEST_CASE("init" * doctest::test_suite("rbuf")) {
    ct_rbuf_init(&s_rbuf, s_test_buf, TEST_BUF_CAPACITY);
    REQUIRE(ct_rbuf_capacity(&s_rbuf) == TEST_BUF_CAPACITY);
    REQUIRE(ct_rbuf_count(&s_rbuf) == 0);
    REQUIRE(ct_rbuf_remain(&s_rbuf) == TEST_BUF_CAPACITY);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == true);
    REQUIRE(ct_rbuf_is_full(&s_rbuf) == false);
    REQUIRE(memset(s_test_buf, 0xAA, sizeof(s_test_buf)));
}

TEST_CASE("static init" * doctest::test_suite("rbuf")) {
    ct_rbuf_t local_rbuf = CT_RBUF_INIT(s_test_buf, TEST_BUF_CAPACITY);
    REQUIRE(ct_rbuf_capacity(&local_rbuf) == TEST_BUF_CAPACITY);
    REQUIRE(ct_rbuf_count(&local_rbuf) == 0);
    REQUIRE(ct_rbuf_remain(&local_rbuf) == TEST_BUF_CAPACITY);
    REQUIRE(ct_rbuf_is_empty(&local_rbuf) == true);
    REQUIRE(ct_rbuf_is_full(&local_rbuf) == false);
    REQUIRE(local_rbuf.data_ == s_test_buf);
}

TEST_CASE("basic" * doctest::test_suite("rbuf")) {
    ct_rbuf_clear(&s_rbuf);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == true);

    const uint8_t input_data[]   = {1, 2, 3, 4, 5};
    uint8_t       output_data[5] = {0};

    REQUIRE(ct_rbuf_write(&s_rbuf, input_data, 5) == 5);
    REQUIRE(ct_rbuf_count(&s_rbuf) == 5);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == false);

    REQUIRE(ct_rbuf_peek(&s_rbuf, output_data, 5) == 5);
    REQUIRE(memcmp(input_data, output_data, 5) == 0);
    REQUIRE(ct_rbuf_count(&s_rbuf) == 5);  // Still there

    memset(output_data, 0, 5);
    REQUIRE(ct_rbuf_read(&s_rbuf, output_data, 5) == 5);
    REQUIRE(memcmp(input_data, output_data, 5) == 0);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == true);
}

TEST_CASE("wrap around" * doctest::test_suite("rbuf")) {
    ct_rbuf_clear(&s_rbuf);

    uint8_t fill_data[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t sink[16]      = {0};

    REQUIRE(ct_rbuf_write(&s_rbuf, fill_data, 12) == 12);
    REQUIRE(ct_rbuf_read(&s_rbuf, sink, 12) == 12);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == true);
    REQUIRE(s_rbuf.head_ == 12);
    REQUIRE(s_rbuf.tail_ == 12);

    uint8_t wrap_data[] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7};
    REQUIRE(ct_rbuf_write(&s_rbuf, wrap_data, 8) == 8);
    REQUIRE(ct_rbuf_count(&s_rbuf) == 8);
    REQUIRE(s_rbuf.tail_ == 4);

    uint8_t peek_sink[8] = {0};
    REQUIRE(ct_rbuf_peek(&s_rbuf, peek_sink, 8) == 8);
    REQUIRE(memcmp(wrap_data, peek_sink, 8) == 0);

    memset(sink, 0, 16);
    REQUIRE(ct_rbuf_read(&s_rbuf, sink, 8) == 8);
    REQUIRE(memcmp(wrap_data, sink, 8) == 0);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == true);
    REQUIRE(s_rbuf.head_ == 4);
}

TEST_CASE("full handling" * doctest::test_suite("rbuf")) {
    ct_rbuf_clear(&s_rbuf);

    uint8_t fill_data[16] = {0xFF};
    REQUIRE(ct_rbuf_write(&s_rbuf, fill_data, 16) == 16);
    REQUIRE(ct_rbuf_is_full(&s_rbuf) == true);
    REQUIRE(ct_rbuf_remain(&s_rbuf) == 0);

    REQUIRE(ct_rbuf_write(&s_rbuf, fill_data, 1) == 0);

    uint8_t sink[1];
    REQUIRE(ct_rbuf_read(&s_rbuf, sink, 1) == 1);
    REQUIRE(ct_rbuf_is_full(&s_rbuf) == false);
    REQUIRE(ct_rbuf_remain(&s_rbuf) == 1);

    uint8_t sink16[16];
    REQUIRE(ct_rbuf_read(&s_rbuf, sink16, 16) == 15);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == true);
}

TEST_CASE("skip" * doctest::test_suite("rbuf")) {
    ct_rbuf_clear(&s_rbuf);

    uint8_t data[] = {10, 20, 30, 40, 50};
    ct_rbuf_write(&s_rbuf, data, 5);

    REQUIRE(ct_rbuf_remove(&s_rbuf, 2) == 2);
    REQUIRE(ct_rbuf_count(&s_rbuf) == 3);

    uint8_t rem[3];
    REQUIRE(ct_rbuf_read(&s_rbuf, rem, 3) == 3);
    REQUIRE(rem[0] == 30);
    REQUIRE(rem[1] == 40);
    REQUIRE(rem[2] == 50);

    REQUIRE(ct_rbuf_remove(&s_rbuf, 5) == 0);
}

TEST_CASE("zero copy" * doctest::test_suite("rbuf")) {
    ct_rbuf_clear(&s_rbuf);

    size_t   chunk_len = 0;
    uint8_t* write_ptr = ct_rbuf_write_ptr(&s_rbuf, &chunk_len);
    REQUIRE(write_ptr != nullptr);
    REQUIRE(chunk_len == TEST_BUF_CAPACITY);

    for (int i = 0; i < 5; ++i) { write_ptr[i] = (uint8_t)(0xB0 + i); }
    REQUIRE(ct_rbuf_commit(&s_rbuf, 5) == 5);
    REQUIRE(ct_rbuf_count(&s_rbuf) == 5);

    const uint8_t* read_ptr = ct_rbuf_read_ptr(&s_rbuf, &chunk_len);
    REQUIRE(read_ptr != nullptr);
    REQUIRE(chunk_len == 5);
    for (int i = 0; i < 5; ++i) { REQUIRE(read_ptr[i] == (uint8_t)(0xB0 + i)); }

    REQUIRE(ct_rbuf_remove(&s_rbuf, 5) == 5);
    REQUIRE(ct_rbuf_is_empty(&s_rbuf) == true);

    write_ptr = ct_rbuf_write_ptr(&s_rbuf, &chunk_len);
    REQUIRE(chunk_len == TEST_BUF_CAPACITY - 5);
    REQUIRE(ct_rbuf_commit(&s_rbuf, chunk_len) == chunk_len);

    size_t   chunk2_len = 0;
    uint8_t* write_ptr2 = ct_rbuf_write_ptr(&s_rbuf, &chunk2_len);
    REQUIRE(write_ptr2 != nullptr);
    REQUIRE(chunk2_len == 5);
    REQUIRE(ct_rbuf_commit(&s_rbuf, chunk2_len) == chunk2_len);

    REQUIRE(ct_rbuf_is_full(&s_rbuf) == true);

    const uint8_t* wrap_read = ct_rbuf_read_ptr(&s_rbuf, &chunk_len);
    REQUIRE(wrap_read != nullptr);
    REQUIRE(chunk_len == TEST_BUF_CAPACITY - 5);

    REQUIRE(ct_rbuf_remove(&s_rbuf, chunk_len) == chunk_len);
    const uint8_t* wrap_read_2 = ct_rbuf_read_ptr(&s_rbuf, &chunk_len);
    REQUIRE(wrap_read_2 != nullptr);
    REQUIRE(chunk_len == 5);
}
