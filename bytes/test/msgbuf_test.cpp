/**
 * @file msgbuf_test.cpp
 * @brief Fixed-capacity message buffer tests.
 */
#include "coter/bytes/msgbuf.h"

#include <cstdint>
#include <cstring>

#include "coter/testing/doctest.h"

namespace {
struct MsgbufFixture {
    ct_msgbuf_t* small_bytes_;
    ct_msgbuf_t* large_bytes_;
    MsgbufFixture() : small_bytes_(nullptr), large_bytes_(nullptr) {
        small_bytes_ = ct_msgbuf_create(16);
        large_bytes_ = ct_msgbuf_create(1024);
        REQUIRE(small_bytes_ != nullptr);
        REQUIRE(large_bytes_ != nullptr);
    }
    ~MsgbufFixture() {
        REQUIRE(small_bytes_ != nullptr);
        REQUIRE(large_bytes_ != nullptr);
        ct_msgbuf_destroy(small_bytes_);
        ct_msgbuf_destroy(large_bytes_);
        small_bytes_ = nullptr;
        large_bytes_ = nullptr;
    }
};
}  // namespace

TEST_SUITE_BEGIN("msgbuf");

TEST_CASE("msgbuf lifecycle") {
    SUBCASE("Initialization") {
        ct_msgbuf_t* s = nullptr;
        ct_msgbuf_t* l = nullptr;
        REQUIRE(s == nullptr);
        REQUIRE(l == nullptr);
        s              = ct_msgbuf_create(16);
        l              = ct_msgbuf_create(1024);
        ct_msgbuf_t* d = ct_msgbuf_create(0);
        REQUIRE(s != nullptr);
        REQUIRE(l != nullptr);
        REQUIRE(d != nullptr);
        REQUIRE(ct_msgbuf_capacity(d) == 1024);
        REQUIRE(ct_msgbuf_available(d) == 1024);
        REQUIRE(ct_msgbuf_size(s) == 0);
        REQUIRE(ct_msgbuf_size(l) == 0);
        REQUIRE(ct_msgbuf_capacity(s) == 16);
        REQUIRE(ct_msgbuf_capacity(l) == 1024);
        REQUIRE(ct_msgbuf_available(s) == 16);
        REQUIRE(ct_msgbuf_available(l) == 1024);
        REQUIRE(ct_msgbuf_write(s, "test", 4) == 4);
        REQUIRE(ct_msgbuf_write(l, "test", 4) == 4);
        REQUIRE(ct_msgbuf_size(s) == 4);
        REQUIRE(ct_msgbuf_size(l) == 4);
        REQUIRE(ct_msgbuf_capacity(s) == 16);
        REQUIRE(ct_msgbuf_capacity(l) == 1024);
        REQUIRE(ct_msgbuf_available(s) == 12);
        REQUIRE(ct_msgbuf_available(l) == 1020);
        ct_msgbuf_destroy(s);
        ct_msgbuf_destroy(l);
        ct_msgbuf_destroy(d);
    }

    SUBCASE("Memory Management") {
        ct_msgbuf_t* s = ct_msgbuf_create(16);
        ct_msgbuf_t* l = ct_msgbuf_create(1024);
        REQUIRE(s != nullptr);
        REQUIRE(l != nullptr);
        ct_msgbuf_clear(s);
        ct_msgbuf_clear(l);
        ct_msgbuf_t* temp_bytes;
        for (int i = 0; i < 1000; ++i) {
            temp_bytes = ct_msgbuf_create(100);
            REQUIRE(temp_bytes != nullptr);
            ct_msgbuf_destroy(temp_bytes);
        }
        ct_msgbuf_destroy(s);
        ct_msgbuf_destroy(l);
    }
}

TEST_CASE_FIXTURE(MsgbufFixture, "msgbuf operations") {
    SUBCASE("Basic Read/Write") {
        REQUIRE(small_bytes_ != nullptr);
        ct_msgbuf_clear(small_bytes_);
        REQUIRE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        char*  byte_buffer = ct_msgbuf_buffer(small_bytes_);
        size_t write_len;
        write_len = ct_msgbuf_write(small_bytes_, "A", 1);
        REQUIRE(write_len == 1);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 1);
        REQUIRE_FALSE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        REQUIRE(byte_buffer[0] == 'A');
        write_len = ct_msgbuf_write(small_bytes_, "Hello", 5);
        REQUIRE(write_len == 5);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 6);
        REQUIRE_FALSE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        REQUIRE(std::memcmp(&byte_buffer[1], "Hello", 5) == 0);
        ct_msgbuf_clear(small_bytes_);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 0);
        REQUIRE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
    }

    SUBCASE("Boundary Conditions") {
        REQUIRE(small_bytes_ != nullptr);
        ct_msgbuf_clear(small_bytes_);
        REQUIRE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        size_t write_len;
        write_len = ct_msgbuf_write(small_bytes_, "", 0);
        REQUIRE(write_len == 0);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 0);
        REQUIRE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        char large_data[16 * 2];
        std::memset(large_data, 'A', sizeof(large_data));
        write_len = ct_msgbuf_write(small_bytes_, large_data, sizeof(large_data));
        REQUIRE(write_len == 16);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 16);
        REQUIRE_FALSE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE(ct_msgbuf_is_full(small_bytes_));
    }

    SUBCASE("Sequential Operations") {
        REQUIRE(small_bytes_ != nullptr);
        ct_msgbuf_clear(small_bytes_);
        REQUIRE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        char*  byte_buffer = ct_msgbuf_buffer(small_bytes_);
        size_t write_len;
        ct_msgbuf_clear(small_bytes_);
        write_len = ct_msgbuf_write(small_bytes_, "Hello", 5);
        write_len += ct_msgbuf_write(small_bytes_, " World", 6);
        REQUIRE(write_len == 11);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 11);
        REQUIRE_FALSE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        REQUIRE(std::memcmp(byte_buffer, "Hello World", 11) == 0);
        ct_msgbuf_clear(small_bytes_);
        write_len = ct_msgbuf_write(small_bytes_, "AB", 2);
        REQUIRE(write_len == 2);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 2);
        REQUIRE_FALSE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        REQUIRE(std::memcmp(byte_buffer, "AB", 2) == 0);
        write_len += ct_msgbuf_write(small_bytes_, "CD", 2);
        REQUIRE(write_len == 4);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 4);
        REQUIRE_FALSE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        REQUIRE(std::memcmp(byte_buffer, "ABCD", 4) == 0);
    }

    SUBCASE("Buffer Overflow Handling") {
        REQUIRE(small_bytes_ != nullptr);
        ct_msgbuf_clear(small_bytes_);
        REQUIRE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
        char*  byte_buffer = ct_msgbuf_buffer(small_bytes_);
        size_t write_len;
        ct_msgbuf_clear(small_bytes_);
        write_len = ct_msgbuf_write(small_bytes_, "AAAAAAAAAAAAAAAA", 16);
        REQUIRE(write_len == 16);
        write_len = ct_msgbuf_write(small_bytes_, "B", 1);
        REQUIRE(write_len == 0);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 16);
        REQUIRE_FALSE(ct_msgbuf_is_empty(small_bytes_));
        REQUIRE(ct_msgbuf_is_full(small_bytes_));
        REQUIRE(std::memcmp(byte_buffer, "AAAAAAAAAAAAAAAA", 16) == 0);
    }

    SUBCASE("Seg View Operations") {
        REQUIRE(small_bytes_ != nullptr);
        ct_msgbuf_clear(small_bytes_);
        ct_msgbuf_write(small_bytes_, "0123456789", 10);
        ct_bytes_t seg;
        seg.overflow_ = 1U;
        REQUIRE(ct_msgbuf_seg(small_bytes_, &seg, 2, 8) == 0);
        REQUIRE_FALSE(ct_bytes_has_error(&seg));
        REQUIRE(ct_bytes_capacity(&seg) == 14);
        REQUIRE(ct_bytes_count(&seg) == 6);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_msgbuf_seg(nullptr, &seg, 0, 1) == -1);
        REQUIRE(ct_msgbuf_seg(small_bytes_, nullptr, 0, 1) == -1);
#if SIZE_MAX > UINT32_MAX
        // Avoid allocating a >4 GiB buffer: a fake header is enough to
        // verify that the 32-bit view range is rejected before it is used.
        ct_msgbuf_t oversized = {};
        oversized.cap_        = (size_t)UINT32_MAX + 1U;
        REQUIRE(ct_msgbuf_seg(&oversized, &seg, 0, 1) == -1);
#endif
        uint8_t buf[10];
        ct_bytes_take_bytes(&seg, buf, 6);
        REQUIRE(std::memcmp((char*)buf, "234567", 6) == 0);
        ct_bytes_rewind(&seg);
        ct_bytes_set_u8(&seg, 0, 'A');
        char* raw_buffer = ct_msgbuf_buffer(small_bytes_);
        REQUIRE(raw_buffer[2] == 'A');
        REQUIRE(ct_msgbuf_seg(small_bytes_, &seg, 0, 16) == 0);
        REQUIRE(ct_bytes_capacity(&seg) == 16);
        REQUIRE(ct_msgbuf_seg(small_bytes_, &seg, 5, 5) == 0);
        REQUIRE(ct_bytes_count(&seg) == 0);
        REQUIRE(ct_msgbuf_seg(small_bytes_, &seg, 5, 4) == -1);
        REQUIRE(ct_msgbuf_seg(small_bytes_, &seg, 0, 17) == -1);
    }

    SUBCASE("Fmt Operations") {
        REQUIRE(small_bytes_ != nullptr);
        ct_msgbuf_clear(small_bytes_);
        ct_msgbuf_fmt(small_bytes_, "Hello %s %d", "World", 42);
        REQUIRE(ct_msgbuf_size(small_bytes_) == 14);
        char* byte_buffer = ct_msgbuf_buffer(small_bytes_);
        REQUIRE(std::strcmp(byte_buffer, "Hello World 42") == 0);

        // Test with NULL format, which triggers ct_snprintf_s returning -1.
        // It should NOT decrement the write pointer.
        size_t prev_size = ct_msgbuf_size(small_bytes_);
        ct_msgbuf_fmt(small_bytes_, nullptr);
        REQUIRE(ct_msgbuf_size(small_bytes_) == prev_size);

        ct_msgbuf_clear(small_bytes_);
        ct_msgbuf_fmt(small_bytes_, "%s", "this string is too long");
        REQUIRE(ct_msgbuf_size(small_bytes_) == 15);
        REQUIRE(byte_buffer[15] == '\0');
        REQUIRE_FALSE(ct_msgbuf_is_full(small_bytes_));
    }
}

TEST_SUITE_END();
