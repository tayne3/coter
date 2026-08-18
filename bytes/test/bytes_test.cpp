#include "coter/bytes/bytes.h"

#include <climits>
#include <cstring>

#include "coter/testing/doctest.h"

TEST_CASE("seg Initialization" * doctest::test_suite("seg") * doctest::test_suite("init")) {
    uint8_t buffer[64];
    memset(buffer, 0xAA, sizeof(buffer));

    SUBCASE("ct_bytes_init function") {
        ct_bytes_t seg;
        ct_bytes_init(&seg, buffer, sizeof(buffer));
        REQUIRE(seg.data == buffer);
        REQUIRE(seg.cap == sizeof(buffer));
        REQUIRE(seg.len == 0);
        REQUIRE(seg.pos == 0);
        REQUIRE(+seg.endian == CT_ENDIAN_BIG);
        REQUIRE(+seg.hlswap == 0);
    }

    SUBCASE("ct_bytes_from function (clamped)") {
        ct_bytes_t seg;
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(seg.data == buffer);
        REQUIRE(seg.cap == sizeof(buffer));
        REQUIRE(seg.len == 32);
        REQUIRE(seg.pos == 0);
        REQUIRE(+seg.endian == CT_ENDIAN_BIG);
        REQUIRE(+seg.hlswap == 0);
    }

    SUBCASE("ct_bytes_init function") {
        ct_bytes_t seg;
        ct_bytes_init(&seg, buffer, sizeof(buffer));
        REQUIRE(seg.data == buffer);
        REQUIRE(seg.cap == sizeof(buffer));
        REQUIRE(seg.len == 0);
        REQUIRE(seg.pos == 0);
        REQUIRE(+seg.endian == CT_ENDIAN_BIG);
        REQUIRE(+seg.hlswap == 0);
    }

    SUBCASE("ct_bytes_from function") {
        ct_bytes_t seg;
        ct_bytes_from(&seg, buffer, sizeof(buffer), 48);
        REQUIRE(seg.data == buffer);
        REQUIRE(seg.cap == sizeof(buffer));
        REQUIRE(seg.len == 48);
        REQUIRE(seg.pos == 0);
        REQUIRE(+seg.endian == CT_ENDIAN_BIG);
        REQUIRE(+seg.hlswap == 0);
    }

    SUBCASE("ct_bytes_from len > cap protection") {
        ct_bytes_t seg;
        ct_bytes_from(&seg, buffer, sizeof(buffer), 1000);
        REQUIRE(seg.len == sizeof(buffer));
    }

    SUBCASE("Static Initialization Macros") {
        ct_bytes_t seg_init = ct_bytes_INIT(buffer, sizeof(buffer));
        REQUIRE(seg_init.data == buffer);
        REQUIRE(seg_init.cap == sizeof(buffer));
        REQUIRE(seg_init.len == 0);
        REQUIRE(seg_init.pos == 0);
        REQUIRE(+seg_init.endian == CT_ENDIAN_BIG);
        REQUIRE(+seg_init.hlswap == 0);

        ct_bytes_t seg_from = ct_bytes_FROM(buffer, sizeof(buffer), 32);
        REQUIRE(seg_from.data == buffer);
        REQUIRE(seg_from.cap == sizeof(buffer));
        REQUIRE(seg_from.len == 32);
        REQUIRE(seg_from.pos == 0);
        REQUIRE(+seg_from.endian == CT_ENDIAN_BIG);
        REQUIRE(+seg_from.hlswap == 0);
    }
}

TEST_CASE("seg State Queries" * doctest::test_suite("seg") * doctest::test_suite("state")) {
    uint8_t    buffer[64];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("Empty buffer state") {
        REQUIRE(ct_bytes_is_empty(&seg) == true);
        REQUIRE(ct_bytes_is_full(&seg) == false);
        REQUIRE(ct_bytes_capacity(&seg) == sizeof(buffer));
        REQUIRE(ct_bytes_count(&seg) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_bytes_readable(&seg) == 0);
        REQUIRE(ct_bytes_writable(&seg) == sizeof(buffer));
        REQUIRE(ct_bytes_appendable(&seg) == sizeof(buffer));
    }

    SUBCASE("Partial fill state") {
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(ct_bytes_is_empty(&seg) == false);
        REQUIRE(ct_bytes_is_full(&seg) == false);
        REQUIRE(ct_bytes_count(&seg) == 4);
        REQUIRE(ct_bytes_pos(&seg) == 4);
        REQUIRE(ct_bytes_readable(&seg) == 0);
        REQUIRE(ct_bytes_writable(&seg) == sizeof(buffer) - 4);
        REQUIRE(ct_bytes_appendable(&seg) == sizeof(buffer) - 4);

        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_bytes_readable(&seg) == 4);
        REQUIRE(ct_bytes_writable(&seg) == sizeof(buffer));
    }

    SUBCASE("Full buffer state") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), sizeof(buffer));
        REQUIRE(ct_bytes_is_empty(&seg) == false);
        REQUIRE(ct_bytes_is_full(&seg) == true);
        REQUIRE(ct_bytes_count(&seg) == sizeof(buffer));
        REQUIRE(ct_bytes_appendable(&seg) == 0);
    }
}

TEST_CASE("seg Basic Operations" * doctest::test_suite("seg") * doctest::test_suite("basic")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("Read/Write Primitives") {
        ct_bytes_put_u8(&seg, 0x12);
        ct_bytes_put_u16(&seg, 0x3456);
        ct_bytes_put_u32(&seg, 0x789ABCDE);
        ct_bytes_put_u64(&seg, 0xFEDCBA9876543210ULL);

        REQUIRE(ct_bytes_pos(&seg) == 1 + 2 + 4 + 8);

        ct_bytes_rewind(&seg);

        REQUIRE(ct_bytes_take_u8(&seg) == 0x12);
        REQUIRE(ct_bytes_take_u16(&seg) == 0x3456);
        REQUIRE(ct_bytes_take_u32(&seg) == 0x789ABCDE);
        REQUIRE(ct_bytes_take_u64(&seg) == 0xFEDCBA9876543210ULL);
    }
}

TEST_CASE("ct_seg Endianness Write and Read Identity" * doctest::test_suite("seg") * doctest::test_suite("endian")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;
    ct_bytes_init(&seg, buffer, sizeof(buffer));

#define TEST_RW_IDENTITY(PutFn, TakeFn, PeekFn, OverwriteFn, Value) \
    do {                                                            \
        ct_bytes_clear(&seg);                                       \
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);                \
        PutFn(&seg, Value);                                         \
        ct_bytes_rewind(&seg);                                      \
        REQUIRE(TakeFn(&seg) == Value);                             \
        ct_bytes_clear(&seg);                                       \
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);                   \
        PutFn(&seg, Value);                                         \
        ct_bytes_rewind(&seg);                                      \
        REQUIRE(TakeFn(&seg) == Value);                             \
        ct_bytes_clear(&seg);                                       \
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);                \
        PutFn(&seg, Value);                                         \
        ct_bytes_rewind(&seg);                                      \
        REQUIRE(PeekFn(&seg, 0) == Value);                          \
        REQUIRE(OverwriteFn(&seg, 0, Value) == 0);                  \
        ct_bytes_rewind(&seg);                                      \
        REQUIRE(TakeFn(&seg) == Value);                             \
    } while (0)

    TEST_RW_IDENTITY(ct_bytes_put_u8, ct_bytes_take_u8, ct_bytes_peek_u8, ct_bytes_set_u8, 0x12);
    TEST_RW_IDENTITY(ct_bytes_put_u16, ct_bytes_take_u16, ct_bytes_peek_u16, ct_bytes_set_u16, 0x1234);
    TEST_RW_IDENTITY(ct_bytes_put_u32, ct_bytes_take_u32, ct_bytes_peek_u32, ct_bytes_set_u32, 0x12345678);
    TEST_RW_IDENTITY(ct_bytes_put_u64, ct_bytes_take_u64, ct_bytes_peek_u64, ct_bytes_set_u64, 0x1122334455667788ULL);

#define TEST_RW_IDENTITY_BYTES(PutFn, TakeFn, PeekFn, OverwriteFn, ValueType, ValBytesPtr, Len) \
    do {                                                                                        \
        ct_bytes_clear(&seg);                                                                   \
        const uint8_t* in_buf       = (ValBytesPtr);                                            \
        uint8_t        out_buf[Len] = {0};                                                      \
        PutFn(&seg, in_buf, Len);                                                               \
        ct_bytes_rewind(&seg);                                                                  \
        REQUIRE(TakeFn(&seg, out_buf, Len) == Len);                                             \
        REQUIRE(memcmp(in_buf, out_buf, Len) == 0);                                             \
        ct_bytes_clear(&seg);                                                                   \
        PutFn(&seg, in_buf, Len);                                                               \
        ct_bytes_rewind(&seg);                                                                  \
        memset(out_buf, 0, Len);                                                                \
        REQUIRE(PeekFn(&seg, 0, out_buf, Len) == Len);                                          \
        REQUIRE(memcmp(in_buf, out_buf, Len) == 0);                                             \
        REQUIRE(OverwriteFn(&seg, 0, in_buf, Len) == 0);                                        \
        ct_bytes_rewind(&seg);                                                                  \
        memset(out_buf, 0, Len);                                                                \
        REQUIRE(TakeFn(&seg, out_buf, Len) == Len);                                             \
        REQUIRE(memcmp(in_buf, out_buf, Len) == 0);                                             \
    } while (0)

    const uint8_t identity_bytes[] = {0x11, 0x22, 0x33, 0x44};
    TEST_RW_IDENTITY_BYTES(ct_bytes_put_bytes, ct_bytes_take_bytes, ct_bytes_peek_bytes, ct_bytes_set_bytes, uint8_t,
                           identity_bytes, 4);

#undef TEST_RW_IDENTITY
#undef TEST_RW_IDENTITY_BYTES

    SUBCASE("Default config values from ct_bytes_init") {
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_set_hlswap(&seg, 1);
        REQUIRE(ct_bytes_get_endian(&seg) == CT_ENDIAN_LITTLE);
        REQUIRE(ct_bytes_get_hlswap(&seg) == 1);

        ct_bytes_init(&seg, buffer, sizeof(buffer));
        REQUIRE(ct_bytes_get_endian(&seg) == CT_ENDIAN_BIG);
        REQUIRE(ct_bytes_get_hlswap(&seg) == 0);
    }
}

TEST_CASE("ct_seg Endianness Memory Layout Verification" * doctest::test_suite("seg") * doctest::test_suite("endian")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("16-bit") {
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_put_u16(&seg, 0xABCD);
        REQUIRE(buffer[0] == 0xCD);
        REQUIRE(buffer[1] == 0xAB);
        ct_bytes_clear(&seg);
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u16(&seg, 0xABCD);
        REQUIRE(buffer[0] == 0xAB);
        REQUIRE(buffer[1] == 0xCD);
    }

    SUBCASE("32-bit") {
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(buffer[0] == 0x78);
        REQUIRE(buffer[1] == 0x56);
        REQUIRE(buffer[2] == 0x34);
        REQUIRE(buffer[3] == 0x12);
        ct_bytes_clear(&seg);
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(buffer[0] == 0x12);
        REQUIRE(buffer[1] == 0x34);
        REQUIRE(buffer[2] == 0x56);
        REQUIRE(buffer[3] == 0x78);
    }

    SUBCASE("64-bit") {
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_put_u64(&seg, 0x1122334455667788ULL);
        REQUIRE(buffer[0] == 0x88);
        REQUIRE(buffer[1] == 0x77);
        REQUIRE(buffer[2] == 0x66);
        REQUIRE(buffer[3] == 0x55);
        REQUIRE(buffer[4] == 0x44);
        REQUIRE(buffer[5] == 0x33);
        REQUIRE(buffer[6] == 0x22);
        REQUIRE(buffer[7] == 0x11);

        ct_bytes_clear(&seg);
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u64(&seg, 0x1122334455667788ULL);
        REQUIRE(buffer[0] == 0x11);
        REQUIRE(buffer[1] == 0x22);
        REQUIRE(buffer[2] == 0x33);
        REQUIRE(buffer[3] == 0x44);
        REQUIRE(buffer[4] == 0x55);
        REQUIRE(buffer[5] == 0x66);
        REQUIRE(buffer[6] == 0x77);
        REQUIRE(buffer[7] == 0x88);
    }
}

TEST_CASE("ct_seg High-Low Word Swap Constraints" * doctest::test_suite("seg") * doctest::test_suite("hlswap")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("16-bit is unaffected by hlswap") {
        ct_bytes_clear(&seg);
        ct_bytes_set_hlswap(&seg, 1);
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u16(&seg, 0xABCD);
        REQUIRE(buffer[0] == 0xAB);
        REQUIRE(buffer[1] == 0xCD);

        ct_bytes_clear(&seg);
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_put_u16(&seg, 0xABCD);
        REQUIRE(buffer[0] == 0xCD);
        REQUIRE(buffer[1] == 0xAB);
    }

    SUBCASE("32-bit swap") {
        ct_bytes_clear(&seg);
        ct_bytes_set_hlswap(&seg, 1);
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u32(&seg, 0xAABBCCDD);
        REQUIRE(buffer[0] == 0xBB);
        REQUIRE(buffer[1] == 0xAA);
        REQUIRE(buffer[2] == 0xDD);
        REQUIRE(buffer[3] == 0xCC);

        ct_bytes_clear(&seg);
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_put_u32(&seg, 0xAABBCCDD);
        REQUIRE(buffer[0] == 0xCC);
        REQUIRE(buffer[1] == 0xDD);
        REQUIRE(buffer[2] == 0xAA);
        REQUIRE(buffer[3] == 0xBB);
    }

    SUBCASE("64-bit swap") {
        ct_bytes_clear(&seg);
        ct_bytes_set_hlswap(&seg, 1);
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u64(&seg, 0x1122334455667788ULL);
        REQUIRE(buffer[0] == 0x22);
        REQUIRE(buffer[1] == 0x11);
        REQUIRE(buffer[2] == 0x44);
        REQUIRE(buffer[3] == 0x33);
        REQUIRE(buffer[4] == 0x66);
        REQUIRE(buffer[5] == 0x55);
        REQUIRE(buffer[6] == 0x88);
        REQUIRE(buffer[7] == 0x77);

        ct_bytes_clear(&seg);
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_put_u64(&seg, 0x1122334455667788ULL);
        REQUIRE(buffer[0] == 0x77);
        REQUIRE(buffer[1] == 0x88);
        REQUIRE(buffer[2] == 0x55);
        REQUIRE(buffer[3] == 0x66);
        REQUIRE(buffer[4] == 0x33);
        REQUIRE(buffer[5] == 0x44);
        REQUIRE(buffer[6] == 0x11);
        REQUIRE(buffer[7] == 0x22);
    }
}

TEST_CASE("ct_seg View-Only Operation Endianness Validation" * doctest::test_suite("seg") *
          doctest::test_suite("peek") * doctest::test_suite("set")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    buffer[0] = 0x11;
    buffer[1] = 0x22;
    buffer[2] = 0x33;
    buffer[3] = 0x44;

    SUBCASE("Peek respects endian") {
        ct_bytes_commit(&seg, 4);
        ct_bytes_rewind(&seg);

        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        REQUIRE(ct_bytes_peek_u32(&seg, 0) == 0x11223344);

        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        REQUIRE(ct_bytes_peek_u32(&seg, 0) == 0x44332211);
    }

    SUBCASE("Overwrite respects endian") {
        ct_bytes_commit(&seg, 4);

        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        REQUIRE(ct_bytes_set_u32(&seg, 0, 0xAABBCCDD) == 0);
        REQUIRE(buffer[0] == 0xAA);
        REQUIRE(buffer[1] == 0xBB);
        REQUIRE(buffer[2] == 0xCC);
        REQUIRE(buffer[3] == 0xDD);

        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        REQUIRE(ct_bytes_set_u32(&seg, 0, 0xAABBCCDD) == 0);
        REQUIRE(buffer[0] == 0xDD);
        REQUIRE(buffer[1] == 0xCC);
        REQUIRE(buffer[2] == 0xBB);
        REQUIRE(buffer[3] == 0xAA);
    }
}

TEST_CASE("seg Peek Operations" * doctest::test_suite("seg") * doctest::test_suite("peek")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("Peek Primitives") {
        ct_bytes_put_u8(&seg, 0x12);
        ct_bytes_put_u16(&seg, 0x3456);
        ct_bytes_put_u32(&seg, 0x789ABCDE);
        ct_bytes_put_u64(&seg, 0xFEDCBA9876543210ULL);

        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_peek_u8(&seg, 0) == 0x12);
        REQUIRE(ct_bytes_peek_u16(&seg, 1) == 0x3456);
        REQUIRE(ct_bytes_peek_u32(&seg, 3) == 0x789ABCDE);
        REQUIRE(ct_bytes_peek_u64(&seg, 7) == 0xFEDCBA9876543210ULL);
        REQUIRE(ct_bytes_pos(&seg) == 0);

        ct_bytes_seek(&seg, 5);
        REQUIRE(ct_bytes_peek_u8(&seg, -5) == 0x12);
        REQUIRE(ct_bytes_peek_u16(&seg, -4) == 0x3456);
    }

    SUBCASE("Peek Bounds") {
        ct_bytes_init(&seg, buffer, 10);
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_rewind(&seg);

        REQUIRE(ct_bytes_peek_u32(&seg, 0) == 0x12345678);

        REQUIRE(ct_bytes_peek_u32(&seg, 10) == 0);
        REQUIRE(ct_bytes_peek_u64(&seg, 0) == 0);
        REQUIRE(ct_bytes_peek_u32(&seg, -1) == 0);
    }
}

TEST_CASE("seg Set Operations" * doctest::test_suite("seg") * doctest::test_suite("set")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("Set Primitives") {
        ct_bytes_put_u32(&seg, 0x11111111);
        ct_bytes_put_u32(&seg, 0x22222222);
        ct_bytes_put_u32(&seg, 0x33333333);

        size_t pos_before = ct_bytes_pos(&seg);
        REQUIRE(ct_bytes_set_u32(&seg, 0, 0xAABBCCDD) == 0);
        REQUIRE(ct_bytes_pos(&seg) == pos_before);

        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u32(&seg) == 0xAABBCCDD);
        REQUIRE(ct_bytes_take_u32(&seg) == 0x22222222);
        REQUIRE(ct_bytes_take_u32(&seg) == 0x33333333);
    }

    SUBCASE("Set Endianness") {
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u32(&seg, 0x12345678);

        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        REQUIRE(ct_bytes_set_u32(&seg, 0, 0xAABBCCDD) == 0);

        uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
        REQUIRE(std::memcmp(buffer, expected, 4) == 0);

        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u32(&seg) == 0xAABBCCDD);
    }

    SUBCASE("Set Bounds") {
        ct_bytes_init(&seg, buffer, 10);
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(ct_bytes_set_u8(&seg, 0, 0xAA) == 0);
        REQUIRE(ct_bytes_set_u32(&seg, 10, 0xBBBBBBBB) == -1);
        REQUIRE(ct_bytes_set_u64(&seg, 0, 0x1122334455667788ULL) == -1);
    }

    SUBCASE("Set All Types") {
        ct_bytes_put_u64(&seg, 0);
        ct_bytes_put_u64(&seg, 0);

        REQUIRE(ct_bytes_set_u8(&seg, 0, 0xAB) == 0);
        REQUIRE(ct_bytes_set_u16(&seg, 2, 0xCDEF) == 0);
        REQUIRE(ct_bytes_set_u64(&seg, 8, 0x1122334455667788ULL) == 0);

        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u8(&seg) == 0xAB);
        ct_bytes_seek(&seg, 2);
        REQUIRE(ct_bytes_take_u16(&seg) == 0xCDEF);
        ct_bytes_seek(&seg, 8);
        REQUIRE(ct_bytes_take_u64(&seg) == 0x1122334455667788ULL);
    }
}

TEST_CASE("seg Position Control" * doctest::test_suite("seg") * doctest::test_suite("position")) {
    uint8_t    buffer[64];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));

    SUBCASE("seek valid") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_seek(&seg, 16) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 16);
        REQUIRE(ct_bytes_seek(&seg, 0) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_bytes_seek(&seg, 32) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 32);
    }

    SUBCASE("seek out of bounds") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_seek(&seg, 33) == -1);
        REQUIRE(ct_bytes_pos(&seg) == 0);
    }

    SUBCASE("reseek from end") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_reseek(&seg, 0) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 32);
        REQUIRE(ct_bytes_reseek(&seg, 10) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 22);
        REQUIRE(ct_bytes_reseek(&seg, 32) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 0);
    }

    SUBCASE("reseek out of bounds") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_reseek(&seg, 33) == -1);
        REQUIRE(ct_bytes_pos(&seg) == 0);
    }

    SUBCASE("skip forward") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_skip(&seg, 10) == 10);
        REQUIRE(ct_bytes_pos(&seg) == 10);
        REQUIRE(ct_bytes_skip(&seg, 5) == 5);
        REQUIRE(ct_bytes_pos(&seg) == 15);
    }

    SUBCASE("skip partial") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 10);
        REQUIRE(ct_bytes_skip(&seg, 100) == 10);
        REQUIRE(ct_bytes_pos(&seg) == 10);
    }

    SUBCASE("commit extends len") {
        ct_bytes_init(&seg, buffer, sizeof(buffer));
        REQUIRE(ct_bytes_count(&seg) == 0);

        REQUIRE(ct_bytes_commit(&seg, 10) == 10);
        REQUIRE(ct_bytes_pos(&seg) == 10);
        REQUIRE(ct_bytes_count(&seg) == 10);

        REQUIRE(ct_bytes_commit(&seg, 5) == 5);
        REQUIRE(ct_bytes_pos(&seg) == 15);
        REQUIRE(ct_bytes_count(&seg) == 15);
    }

    SUBCASE("commit capped by capacity") {
        ct_bytes_init(&seg, buffer, sizeof(buffer));
        REQUIRE(ct_bytes_commit(&seg, 1000) == (int)sizeof(buffer));
        REQUIRE(ct_bytes_pos(&seg) == sizeof(buffer));
        REQUIRE(ct_bytes_count(&seg) == sizeof(buffer));
    }

    SUBCASE("clear resets pos and len") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        ct_bytes_seek(&seg, 16);
        REQUIRE(ct_bytes_pos(&seg) == 16);
        REQUIRE(ct_bytes_count(&seg) == 32);

        ct_bytes_clear(&seg);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_bytes_count(&seg) == 0);
    }

    SUBCASE("rewind only resets pos") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        ct_bytes_seek(&seg, 16);

        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_bytes_count(&seg) == 32);
    }
}

TEST_CASE("seg IO Operations" * doctest::test_suite("seg") * doctest::test_suite("io")) {
    uint8_t    buffer[64];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("put_bytes data") {
        uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
        REQUIRE(ct_bytes_put_bytes(&seg, data, 5) == 5);
        REQUIRE(ct_bytes_pos(&seg) == 5);
        REQUIRE(ct_bytes_count(&seg) == 5);
        REQUIRE(std::memcmp(buffer, data, 5) == 0);
    }

    SUBCASE("put_bytes partial") {
        ct_bytes_init(&seg, buffer, 4);
        uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        REQUIRE(ct_bytes_put_bytes(&seg, data, 8) == 4);
        REQUIRE(ct_bytes_pos(&seg) == 4);
        REQUIRE(ct_bytes_count(&seg) == 4);
    }

    SUBCASE("take_bytes data") {
        uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
        ct_bytes_put_bytes(&seg, data, 4);
        ct_bytes_rewind(&seg);

        uint8_t out[4] = {0};
        REQUIRE(ct_bytes_take_bytes(&seg, out, 4) == 4);
        REQUIRE(std::memcmp(out, data, 4) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 4);
    }

    SUBCASE("take_bytes partial") {
        uint8_t data[] = {0xAA, 0xBB};
        ct_bytes_put_bytes(&seg, data, 2);
        ct_bytes_rewind(&seg);

        uint8_t out[8] = {0};
        REQUIRE(ct_bytes_take_bytes(&seg, out, 8) == 2);
        REQUIRE(ct_bytes_pos(&seg) == 2);
    }

    SUBCASE("peek_bytes and poke_bytes data") {
        uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
        ct_bytes_put_bytes(&seg, data, 4);
        ct_bytes_rewind(&seg);

        uint8_t out[4] = {0};
        REQUIRE(ct_bytes_peek_bytes(&seg, 0, out, 4) == 4);
        REQUIRE(std::memcmp(out, data, 4) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 0);

        uint8_t out2[2] = {0};
        REQUIRE(ct_bytes_peek_bytes(&seg, 2, out2, 2) == 2);
        REQUIRE(out2[0] == 0x33);
        REQUIRE(out2[1] == 0x44);

        ct_bytes_skip(&seg, 2);
        REQUIRE(ct_bytes_peek_bytes(&seg, 0, out2, 2) == 2);
        REQUIRE(out2[0] == 0x33);
        REQUIRE(out2[1] == 0x44);

        uint8_t inject[] = {0xAA, 0xBB};
        REQUIRE(ct_bytes_poke_bytes(&seg, -1, inject, 2) == 0);
        REQUIRE(ct_bytes_peek_bytes(&seg, -1, out2, 2) == 2);
        REQUIRE(out2[0] == 0xAA);
        REQUIRE(out2[1] == 0xBB);

        REQUIRE(ct_bytes_poke_bytes(&seg, -3, inject, 2) == -1);  // out of bounds negative
        REQUIRE(ct_bytes_poke_bytes(&seg, 2, inject, 2) == -1);   // out of bounds positive
    }

    SUBCASE("fill pattern") {
        REQUIRE(ct_bytes_fill(&seg, 0xAA, 10) == 10);
        REQUIRE(ct_bytes_pos(&seg) == 10);
        REQUIRE(ct_bytes_count(&seg) == 10);
        for (int i = 0; i < 10; ++i) { REQUIRE(buffer[i] == 0xAA); }
    }

    SUBCASE("fill capped") {
        ct_bytes_init(&seg, buffer, 4);
        REQUIRE(ct_bytes_fill(&seg, 0xBB, 100) == 4);
        REQUIRE(ct_bytes_pos(&seg) == 4);
    }
}

TEST_CASE("seg View Operations" * doctest::test_suite("seg") * doctest::test_suite("view")) {
    uint8_t    buffer[64];
    ct_bytes_t seg;
    ct_bytes_t view;

    for (int i = 0; i < 64; ++i) buffer[i] = (uint8_t)i;

    SUBCASE("since valid range") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_since(&seg, &view, 8, 24) == 0);
        REQUIRE(view.data == buffer + 8);
        REQUIRE(view.cap == sizeof(buffer) - 8);
        REQUIRE(view.len == 16);
        REQUIRE(view.pos == 0);
    }

    SUBCASE("since with default end (0)") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);

        // Case 1: start=0, end=0 -> [0, 32]
        REQUIRE(ct_bytes_since(&seg, &view, 0, 0) == 0);
        REQUIRE(view.data == buffer);
        REQUIRE(view.len == 32);

        // Case 2: start=10, end=0 -> [10, 32]
        REQUIRE(ct_bytes_since(&seg, &view, 10, 0) == 0);
        REQUIRE(view.data == buffer + 10);
        REQUIRE(view.len == 22);
    }

    SUBCASE("since invalid range") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_since(&seg, &view, 24, 8) == -1);
        REQUIRE(ct_bytes_since(&seg, &view, 0, 100) == -1);
    }

    SUBCASE("since empty range") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        REQUIRE(ct_bytes_since(&seg, &view, 10, 10) == 0);
        REQUIRE(view.data == buffer + 10);
        REQUIRE(view.len == 0);
        REQUIRE(view.pos == 0);
    }

    SUBCASE("since inherits config") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_set_hlswap(&seg, 1);

        ct_bytes_since(&seg, &view, 0, 16);
        REQUIRE(+view.endian == CT_ENDIAN_LITTLE);
        REQUIRE(+view.hlswap == 1);
    }

    SUBCASE("readable_since") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        ct_bytes_seek(&seg, 8);
        ct_bytes_readable_since(&seg, &view);
        REQUIRE(view.data == buffer + 8);
        REQUIRE(view.len == 24);
    }

    SUBCASE("writable_since") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        ct_bytes_seek(&seg, 8);
        ct_bytes_writable_since(&seg, &view);
        REQUIRE(view.data == buffer + 8);
        REQUIRE(view.len == sizeof(buffer) - 8);
    }

    SUBCASE("compact") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 16);
        ct_bytes_seek(&seg, 8);

        ct_bytes_compact(&seg);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_bytes_count(&seg) == 8);
        REQUIRE(buffer[0] == 8);
        REQUIRE(buffer[7] == 15);
    }
}

TEST_CASE("seg Put/Take All Types" * doctest::test_suite("seg") * doctest::test_suite("types")) {
    uint8_t    buffer[128];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("u8 roundtrip") {
        ct_bytes_put_u8(&seg, 0x00);
        ct_bytes_put_u8(&seg, 0xFF);
        ct_bytes_put_u8(&seg, 0x7F);
        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u8(&seg) == 0x00);
        REQUIRE(ct_bytes_take_u8(&seg) == 0xFF);
        REQUIRE(ct_bytes_take_u8(&seg) == 0x7F);
    }

    SUBCASE("u16 roundtrip") {
        ct_bytes_put_u16(&seg, 0x0000);
        ct_bytes_put_u16(&seg, 0xFFFF);
        ct_bytes_put_u16(&seg, 0x1234);
        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u16(&seg) == 0x0000);
        REQUIRE(ct_bytes_take_u16(&seg) == 0xFFFF);
        REQUIRE(ct_bytes_take_u16(&seg) == 0x1234);
    }

    SUBCASE("u32 roundtrip") {
        ct_bytes_put_u32(&seg, 0x00000000);
        ct_bytes_put_u32(&seg, 0xFFFFFFFF);
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u32(&seg) == 0x00000000);
        REQUIRE(ct_bytes_take_u32(&seg) == 0xFFFFFFFF);
        REQUIRE(ct_bytes_take_u32(&seg) == 0x12345678);
    }

    SUBCASE("u64 roundtrip") {
        ct_bytes_put_u64(&seg, 0x0000000000000000ULL);
        ct_bytes_put_u64(&seg, 0xFFFFFFFFFFFFFFFFULL);
        ct_bytes_put_u64(&seg, 0x123456789ABCDEF0ULL);
        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u64(&seg) == 0x0000000000000000ULL);
        REQUIRE(ct_bytes_take_u64(&seg) == 0xFFFFFFFFFFFFFFFFULL);
        REQUIRE(ct_bytes_take_u64(&seg) == 0x123456789ABCDEF0ULL);
    }
}

TEST_CASE("seg Boundary Overflow" * doctest::test_suite("seg") * doctest::test_suite("boundary")) {
    uint8_t    buffer[8];
    ct_bytes_t seg;

    SUBCASE("put beyond capacity") {
        ct_bytes_init(&seg, buffer, sizeof(buffer));
        ct_bytes_put_u32(&seg, 0x11111111);
        ct_bytes_put_u32(&seg, 0x22222222);

        REQUIRE(ct_bytes_pos(&seg) == 8);
        REQUIRE(ct_bytes_is_full(&seg) == true);

        ct_bytes_put_u8(&seg, 0xFF);
        REQUIRE(ct_bytes_pos(&seg) == 8);
    }

    SUBCASE("take beyond length") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 4);
        buffer[0] = 0x11;
        buffer[1] = 0x22;
        buffer[2] = 0x33;
        buffer[3] = 0x44;

        REQUIRE(ct_bytes_take_u32(&seg) == 0x11223344);
        REQUIRE(ct_bytes_take_u32(&seg) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 4);
    }
}

TEST_CASE("seg Get Operations" * doctest::test_suite("seg") * doctest::test_suite("get")) {
    uint8_t    buffer[4096];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("Get Primitives") {
        ct_bytes_put_u8(&seg, 0x12);
        ct_bytes_put_u16(&seg, 0x3456);
        ct_bytes_put_u32(&seg, 0x789ABCDE);
        ct_bytes_put_u64(&seg, 0xFEDCBA9876543210ULL);

        REQUIRE(ct_bytes_get_u8(&seg, 0) == 0x12);
        REQUIRE(ct_bytes_get_u16(&seg, 1) == 0x3456);
        REQUIRE(ct_bytes_get_u32(&seg, 3) == 0x789ABCDE);
        REQUIRE(ct_bytes_get_u64(&seg, 7) == 0xFEDCBA9876543210ULL);

        REQUIRE(ct_bytes_pos(&seg) == 15);
    }

    SUBCASE("Get does not change pos or len") {
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_rewind(&seg);

        size_t pos_before = ct_bytes_pos(&seg);
        size_t len_before = ct_bytes_count(&seg);

        ct_bytes_get_u32(&seg, 0);

        REQUIRE(ct_bytes_pos(&seg) == pos_before);
        REQUIRE(ct_bytes_count(&seg) == len_before);
    }

    SUBCASE("Get Bounds") {
        ct_bytes_init(&seg, buffer, 10);
        ct_bytes_put_u32(&seg, 0x12345678);

        REQUIRE(ct_bytes_get_u32(&seg, 0) == 0x12345678);
        REQUIRE(ct_bytes_get_u32(&seg, 10) == 0);
        REQUIRE(ct_bytes_get_u64(&seg, 0) == 0);
        REQUIRE(ct_bytes_get_u8(&seg, 100) == 0);
    }

    SUBCASE("Get Endianness Big") {
        ct_bytes_set_endian(&seg, CT_ENDIAN_BIG);
        ct_bytes_put_u32(&seg, 0x11223344);

        REQUIRE(ct_bytes_get_u8(&seg, 0) == 0x11);
        REQUIRE(ct_bytes_get_u8(&seg, 1) == 0x22);
        REQUIRE(ct_bytes_get_u8(&seg, 2) == 0x33);
        REQUIRE(ct_bytes_get_u8(&seg, 3) == 0x44);
    }

    SUBCASE("Get Endianness Little") {
        ct_bytes_set_endian(&seg, CT_ENDIAN_LITTLE);
        ct_bytes_put_u32(&seg, 0x11223344);

        REQUIRE(ct_bytes_get_u8(&seg, 0) == 0x44);
        REQUIRE(ct_bytes_get_u8(&seg, 1) == 0x33);
        REQUIRE(ct_bytes_get_u8(&seg, 2) == 0x22);
        REQUIRE(ct_bytes_get_u8(&seg, 3) == 0x11);
    }

    SUBCASE("Get All Types") {
        ct_bytes_put_u8(&seg, 0xAB);
        ct_bytes_put_u16(&seg, 0xCDEF);
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_put_u64(&seg, 0xFEDCBA9876543210ULL);

        REQUIRE(ct_bytes_get_u8(&seg, 0) == 0xAB);
        REQUIRE(ct_bytes_get_u16(&seg, 1) == 0xCDEF);
        REQUIRE(ct_bytes_get_u32(&seg, 3) == 0x12345678);
        REQUIRE(ct_bytes_get_u64(&seg, 7) == 0xFEDCBA9876543210ULL);
    }
}

TEST_CASE("seg Truncate" * doctest::test_suite("seg") * doctest::test_suite("truncate")) {
    uint8_t    buffer[64];
    ct_bytes_t seg;

    memset(buffer, 0, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("Truncate normal") {
        ct_bytes_put_u32(&seg, 0x11111111);
        ct_bytes_put_u32(&seg, 0x22222222);
        ct_bytes_put_u32(&seg, 0x33333333);

        REQUIRE(ct_bytes_count(&seg) == 12);

        ct_bytes_truncate(&seg, 8);
        REQUIRE(ct_bytes_count(&seg) == 8);
        REQUIRE(ct_bytes_pos(&seg) == 8);

        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_take_u32(&seg) == 0x11111111);
        REQUIRE(ct_bytes_take_u32(&seg) == 0x22222222);
        REQUIRE(ct_bytes_readable(&seg) == 0);
    }

    SUBCASE("Truncate adjusts pos if needed") {
        ct_bytes_put_u32(&seg, 0x11111111);
        ct_bytes_put_u32(&seg, 0x22222222);
        ct_bytes_put_u32(&seg, 0x33333333);

        ct_bytes_seek(&seg, 10);
        REQUIRE(ct_bytes_pos(&seg) == 10);

        ct_bytes_truncate(&seg, 4);
        REQUIRE(ct_bytes_count(&seg) == 4);
        REQUIRE(ct_bytes_pos(&seg) == 4);
    }

    SUBCASE("Truncate no effect if new_len >= len") {
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(ct_bytes_count(&seg) == 4);

        ct_bytes_truncate(&seg, 10);
        REQUIRE(ct_bytes_count(&seg) == 4);

        ct_bytes_truncate(&seg, 4);
        REQUIRE(ct_bytes_count(&seg) == 4);
    }

    SUBCASE("Truncate to zero") {
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_seek(&seg, 2);

        ct_bytes_truncate(&seg, 0);
        REQUIRE(ct_bytes_count(&seg) == 0);
        REQUIRE(ct_bytes_pos(&seg) == 0);
    }
}

TEST_CASE("seg Find" * doctest::test_suite("seg") * doctest::test_suite("find")) {
    uint8_t    buffer[64];
    ct_bytes_t seg;

    for (int i = 0; i < 64; ++i) buffer[i] = (uint8_t)i;

    SUBCASE("Find byte exists") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);

        REQUIRE(ct_bytes_find(&seg, 0, 0) == 0);
        REQUIRE(ct_bytes_find(&seg, 10, 0) == 10);
        REQUIRE(ct_bytes_find(&seg, 31, 0) == 31);
    }

    SUBCASE("Find byte not exists") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);

        REQUIRE(ct_bytes_find(&seg, 100, 0) == -1);
        REQUIRE(ct_bytes_find(&seg, 32, 0) == -1);
        REQUIRE(ct_bytes_find(&seg, 255, 0) == -1);
    }

    SUBCASE("Find from pos") {
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);
        ct_bytes_seek(&seg, 10);

        REQUIRE(ct_bytes_find(&seg, 5, 0) == -1);
        REQUIRE(ct_bytes_find(&seg, 10, 0) == 0);
        REQUIRE(ct_bytes_find(&seg, 15, 0) == 5);
        REQUIRE(ct_bytes_find(&seg, 31, 0) == 21);
    }

    SUBCASE("Find first match") {
        buffer[10] = 0xAA;
        buffer[20] = 0xAA;
        ct_bytes_from(&seg, buffer, sizeof(buffer), 32);

        REQUIRE(ct_bytes_find(&seg, 0xAA, 0) == 10);

        ct_bytes_seek(&seg, 15);
        REQUIRE(ct_bytes_find(&seg, 0xAA, 0) == 5);
    }

    SUBCASE("Find in empty buffer") {
        ct_bytes_init(&seg, buffer, sizeof(buffer));

        REQUIRE(ct_bytes_find(&seg, 0, 0) == -1);
        REQUIRE(ct_bytes_find(&seg, 0xFF, 0) == -1);
    }
}

TEST_CASE("seg Overfill" * doctest::test_suite("seg") * doctest::test_suite("overfill")) {
    uint8_t    buffer[64];
    ct_bytes_t seg;

    memset(buffer, 0xAA, sizeof(buffer));
    ct_bytes_init(&seg, buffer, sizeof(buffer));

    SUBCASE("Overfill does not advance pos") {
        REQUIRE(ct_bytes_pos(&seg) == 0);

        REQUIRE(ct_bytes_overfill(&seg, 0x00, 10) == 10);
        REQUIRE(ct_bytes_pos(&seg) == 0);
        REQUIRE(ct_bytes_count(&seg) == 0);

        for (int i = 0; i < 10; ++i) { REQUIRE(buffer[i] == 0x00); }
    }

    SUBCASE("Overfill does not change len") {
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(ct_bytes_count(&seg) == 4);

        REQUIRE(ct_bytes_overfill(&seg, 0xFF, 8) == 8);
        REQUIRE(ct_bytes_count(&seg) == 4);
    }

    SUBCASE("Overfill capped by capacity") {
        REQUIRE(ct_bytes_overfill(&seg, 0xBB, 100) == 64);

        for (int i = 0; i < 64; ++i) { REQUIRE(buffer[i] == 0xBB); }
    }

    SUBCASE("Overfill from buffer start") {
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_seek(&seg, 4);

        REQUIRE(ct_bytes_overfill(&seg, 0xCC, 8) == 8);

        for (int i = 0; i < 8; ++i) { REQUIRE(buffer[i] == 0xCC); }
        REQUIRE(ct_bytes_pos(&seg) == 4);
    }

    SUBCASE("Overfill zeroing") {
        memset(buffer, 0xFF, sizeof(buffer));
        ct_bytes_init(&seg, buffer, sizeof(buffer));

        REQUIRE(ct_bytes_overfill(&seg, 0, 32) == 32);

        for (int i = 0; i < 32; ++i) { REQUIRE(buffer[i] == 0); }
        for (int i = 32; i < 64; ++i) { REQUIRE(buffer[i] == 0xFF); }
    }
}

TEST_CASE("seg error marking" * doctest::test_suite("seg") * doctest::test_suite("error")) {
    uint8_t    buffer[16];
    ct_bytes_t seg;

    SUBCASE("Tier 1: Feature Coverage - Out-of-bounds sets overflow, valid does not") {
        // Valid operations do not set overflow
        ct_bytes_init(&seg, buffer, 8);
        REQUIRE(ct_bytes_has_error(&seg) == 0);
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // Out-of-bounds write (put)
        ct_bytes_put_u64(&seg, 0x1122334455667788ULL);  // Capacity is 8, already has 4 bytes. 8 > 4 remaining space.
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Reset and test write bytes
        ct_bytes_init(&seg, buffer, 8);
        uint8_t write_data[10] = {0};
        ct_bytes_put_bytes(&seg, write_data, 10);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Out-of-bounds read (take)
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_rewind(&seg);
        REQUIRE(ct_bytes_has_error(&seg) == 0);
        ct_bytes_take_u64(&seg);  // Readable is 4, trying to read 8
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Reset and test take bytes
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_rewind(&seg);
        uint8_t read_data[10];
        ct_bytes_take_bytes(&seg, read_data, 10);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Out-of-bounds absolute get
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);  // len = 4
        ct_bytes_get_u32(&seg,
                         2);  // gets 4 bytes starting at offset 2 (offsets 2,3,4,5). Offset 4 and 5 are out of len.
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // Out-of-bounds absolute set
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);  // len = 4
        ct_bytes_set_u32(&seg, 2, 0x11111111);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Out-of-bounds peek/poke
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);  // len = 4, pos = 4
        ct_bytes_peek_u8(&seg, 0);           // pos + 0 = 4 >= len
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);            // len = 4, pos = 4
        ct_bytes_poke_bytes(&seg, -2, write_data, 4);  // pos - 2 = 2. 2 + 4 = 6 > len
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Out-of-bounds control operations
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);  // len = 4
        REQUIRE(ct_bytes_seek(&seg, 5) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Out-of-bounds control operations - reseek
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);  // len = 4
        REQUIRE(ct_bytes_reseek(&seg, 5) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Out-of-bounds control operations - commit
        ct_bytes_init(&seg, buffer, 8);
        REQUIRE(ct_bytes_commit(&seg, 10) == 8);  // commit capped at writable
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Out-of-bounds control operations - since
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_t view;
        REQUIRE(ct_bytes_since(&seg, &view, 0, 10) == -1);  // end > cap
        REQUIRE(ct_bytes_has_error(&seg) == 0);
    }

    SUBCASE("Tier 2: Boundary & Corner Cases") {
        // Empty segment
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_take_u8(&seg);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Zero capacity segment
        ct_bytes_init(&seg, nullptr, 0);
        ct_bytes_put_u8(&seg, 0xAA);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Exact boundary operations (len/cap)
        ct_bytes_init(&seg, buffer, 4);
        ct_bytes_put_u32(&seg, 0x12345678);
        REQUIRE(ct_bytes_has_error(&seg) == 0);  // Exact boundary write, should not error
        ct_bytes_put_u8(&seg, 0xAA);
        REQUIRE(ct_bytes_has_error(&seg) == 1);  // 1 byte past boundary, should error

        ct_bytes_init(&seg, buffer, 4);
        ct_bytes_put_u32(&seg, 0x12345678);
        ct_bytes_rewind(&seg);
        ct_bytes_take_u32(&seg);
        REQUIRE(ct_bytes_has_error(&seg) == 0);  // Exact boundary read, should not error
        ct_bytes_take_u8(&seg);
        REQUIRE(ct_bytes_has_error(&seg) == 1);  // 1 byte past boundary, should error
    }

    SUBCASE("Tier 3: Cross-Feature Combination - clearing & copying/moving") {
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u64(&seg, 0x1122334455667788ULL);
        ct_bytes_put_u8(&seg, 0xAA);  // overflow set
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // clear_error() resets
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // clear() resets
        ct_bytes_put_u8(&seg, 0xAA);  // overflow set again
        REQUIRE(ct_bytes_has_error(&seg) == 1);
        ct_bytes_clear(&seg);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // Structure copy preserves flag
        ct_bytes_init(&seg, buffer, 8);
        ct_bytes_put_u8(&seg, 0xAA);
        ct_bytes_take_u16(&seg);  // overflow set
        REQUIRE(ct_bytes_has_error(&seg) == 1);
        ct_bytes_t seg_copy = seg;
        REQUIRE(ct_bytes_has_error(&seg_copy) == 1);
    }

    SUBCASE("Tier 4: Real-world Workloads - Multi-step parsing scenario") {
        // Protocol: [length: 2 bytes] [type: 1 byte] [payload: N bytes]
        // Buffer: 0x00, 0x05, 0x01, 0xAA, 0xBB (Truncated payload, payload says 5 bytes but only 2 present)
        uint8_t packet[] = {0x00, 0x05, 0x01, 0xAA, 0xBB};
        ct_bytes_from(&seg, packet, sizeof(packet), sizeof(packet));

        REQUIRE(ct_bytes_has_error(&seg) == 0);

        uint16_t length = ct_bytes_take_u16(&seg);
        REQUIRE(length == 5);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        uint8_t type = ct_bytes_take_u8(&seg);
        REQUIRE(type == 1);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        uint8_t payload[5];
        int     read_bytes = ct_bytes_take_bytes(&seg, payload, length);
        REQUIRE(read_bytes < length);            // Should only read 2 bytes
        REQUIRE(ct_bytes_has_error(&seg) == 1);  // Triggers overflow
    }
}

TEST_CASE("seg Extreme Boundary Values" * doctest::test_suite("seg") * doctest::test_suite("extreme")) {
    uint8_t    buffer[16] = {0};
    ct_bytes_t seg;
    ct_bytes_init(&seg, buffer, 16);
    ct_bytes_commit(&seg, 8);  // len = 8

    SUBCASE("Absolute getters with extreme offsets") {
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_get_u8(&seg, SIZE_MAX) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_get_u16(&seg, SIZE_MAX - 1) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_get_u32(&seg, SIZE_MAX - 3) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_get_u64(&seg, SIZE_MAX - 7) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);
    }

    SUBCASE("Absolute setters with extreme offsets") {
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_u8(&seg, SIZE_MAX, 0xFF) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_u16(&seg, SIZE_MAX - 1, 0xFFFF) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_u32(&seg, SIZE_MAX - 3, 0xFFFFFFFF) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_u64(&seg, SIZE_MAX - 7, 0xFFFFFFFFFFFFFFFFULL) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("Absolute bytes with extreme offsets") {
        uint8_t out[8] = {0};
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_get_bytes(&seg, SIZE_MAX, out, 4) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_bytes(&seg, SIZE_MAX, out, 4) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("Seek/Reseek with extreme offsets") {
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_seek(&seg, SIZE_MAX) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_reseek(&seg, SIZE_MAX) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("Peek/Poke relative offset wrapping") {
        ct_bytes_clear_error(&seg);
        // pos is 8. If offset is INT_MAX, abs_pos = 8 + INT_MAX = 2147483655, which is > len (8)
        REQUIRE(ct_bytes_peek_u8(&seg, INT_MAX) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u16(&seg, INT_MAX) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u32(&seg, INT_MAX) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u64(&seg, INT_MAX) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        uint8_t out[4] = {0};
        REQUIRE(ct_bytes_peek_bytes(&seg, INT_MAX, out, 4) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_poke_bytes(&seg, INT_MAX, out, 4) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }
}

TEST_CASE("seg Adversarial Boundary and Overflow Tests" * doctest::test_suite("seg") *
          doctest::test_suite("adversarial")) {
    uint8_t    buffer[64] = {0};
    ct_bytes_t seg;

    SUBCASE("ct_bytes_find offset wrap around bug") {
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);  // len = 10
        ct_bytes_seek(&seg, 2);     // pos = 2
        buffer[4] = 0xBB;

        // If offset is SIZE_MAX - 1 (i.e. (size_t)-2), then pos + offset is 2 + SIZE_MAX - 1 = SIZE_MAX + 1 = 0
        // (wrapped). Since 0 < len (10), it bypasses the bounds check. It then searches starting from buffer[0] (pos +
        // offset = 0) for length 10. It should return -1 because offset is way out of bounds.
        int result = ct_bytes_find(&seg, 0xBB, (size_t)-2);
        REQUIRE(result == -1);

        result = ct_bytes_find(&seg, 0xBB, SIZE_MAX);
        REQUIRE(result == -1);
    }

    SUBCASE("ct_bytes_FROM macro lacks len <= cap protection") {
        ct_bytes_t macro_seg = ct_bytes_FROM(buffer, 10, 20);
        // The macro does not clamp len, so len remains 20.
        // The function ct_bytes_from clamps it to 10.
        // Correct behavior should clamp or fail, but the macro allows len > cap.
        REQUIRE(macro_seg.len <= macro_seg.cap);
    }

    SUBCASE("Peek/Poke with INT_MIN offset") {
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);
        ct_bytes_seek(&seg, 2);  // pos = 2

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u8(&seg, INT_MIN) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        uint8_t out[4] = {0};
        REQUIRE(ct_bytes_peek_bytes(&seg, INT_MIN, out, 2) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_poke_bytes(&seg, INT_MIN, out, 2) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("ct_bytes_since with extreme start/end") {
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);
        ct_bytes_t view;

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_since(&seg, &view, (size_t)-1, 10) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_since(&seg, &view, 0, (size_t)-1) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 0);
    }
}

TEST_CASE("seg Challenger 2 Adversarial Boundary Tests" * doctest::test_suite("seg") *
          doctest::test_suite("challenger2")) {
    uint8_t buffer[64] = {0};

    SUBCASE("Seek/Reseek negative offsets cast as size_t") {
        ct_bytes_t seg;
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);

        // Negative offsets cast to size_t should be treated as extremely large numbers and fail
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_seek(&seg, (size_t)-1) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_seek(&seg, (size_t)-500) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_reseek(&seg, (size_t)-1) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_reseek(&seg, (size_t)-500) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("Skip and Commit with size_t overflow values") {
        ct_bytes_t seg;
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);
        ct_bytes_seek(&seg, 5);  // readable = 5, writable = 11

        // Skip with SIZE_MAX should clamp to readable and trigger overflow
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_skip(&seg, SIZE_MAX) == 5);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
        REQUIRE(ct_bytes_pos(&seg) == 10);

        // Commit with SIZE_MAX should clamp to writable (which is 16 - pos = 6) and trigger overflow
        ct_bytes_seek(&seg, 10);
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_commit(&seg, SIZE_MAX) == 6);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
        REQUIRE(ct_bytes_pos(&seg) == 16);
        REQUIRE(ct_bytes_count(&seg) == 16);
    }

    SUBCASE("Get/Set absolute bytes with SIZE_MAX and offsets near size_t max") {
        ct_bytes_t seg;
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);

        uint8_t out[8] = {0};
        ct_bytes_clear_error(&seg);
        // length = 2, offset = SIZE_MAX - 1 (offset + length wraps to 0)
        REQUIRE(ct_bytes_get_bytes(&seg, SIZE_MAX - 1, out, 2) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_bytes(&seg, SIZE_MAX - 1, out, 2) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("Peek/Poke negative and positive boundary offsets") {
        ct_bytes_t seg;
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);
        ct_bytes_seek(&seg, 5);

        // Peek with negative offset wrapping past 0
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u8(&seg, -6) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // Poke with negative offset wrapping past 0
        ct_bytes_clear_error(&seg);
        uint8_t val = 0xAA;
        REQUIRE(ct_bytes_poke_bytes(&seg, -6, &val, 1) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Peek with offset that makes abs_pos >= len
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u8(&seg, 5) == 0);  // pos + 5 = 10 == len (out of bounds)
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // Poke with offset that makes abs_pos > len
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_poke_bytes(&seg, 6, &val, 1) == -1);  // pos + 6 = 11 > len (out of bounds)
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("Uninitialized bitfields copy safety") {
        // Initialize memory with garbage to force potential sanitizer/uninitialized read issues
        alignas(ct_bytes_t) uint8_t raw_memory[sizeof(ct_bytes_t)];
        memset(raw_memory, 0x5A, sizeof(raw_memory));

        ct_bytes_t* seg_ptr = reinterpret_cast<ct_bytes_t*>(raw_memory);
        // Call ct_bytes_init, which initializes bitfields
        ct_bytes_init(seg_ptr, buffer, 16);

        // Verify bitfields are correctly set to expected initial values
        REQUIRE(+seg_ptr->endian == CT_ENDIAN_BIG);
        REQUIRE(+seg_ptr->hlswap == 0U);
        REQUIRE(+seg_ptr->overflow == 0U);

        // Perform a copy of the struct containing initialized bitfields (and potentially uninitialized padding/other
        // bits)
        ct_bytes_t copied_seg = *seg_ptr;
        REQUIRE(copied_seg.data == buffer);
        REQUIRE(copied_seg.cap == 16);
        REQUIRE(+copied_seg.endian == CT_ENDIAN_BIG);
        REQUIRE(+copied_seg.hlswap == 0U);
        REQUIRE(+copied_seg.overflow == 0U);
    }
}

TEST_CASE("seg Challenger 4 Adversarial Boundary Tests" * doctest::test_suite("seg") *
          doctest::test_suite("challenger4")) {
    uint8_t buffer[64] = {0};

    SUBCASE("take_bytes, put_bytes, fill, overfill with SIZE_MAX length") {
        ct_bytes_t seg;
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 8);  // len = 8, pos = 8

        // take_bytes with SIZE_MAX
        uint8_t out[16] = {0};
        ct_bytes_clear_error(&seg);
        int read_len = ct_bytes_take_bytes(&seg, out, SIZE_MAX);
        REQUIRE(read_len == 0);  // pos is 8, len is 8, so len - pos = 0
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Reset and seek to 4 (readable = 4)
        ct_bytes_seek(&seg, 4);
        ct_bytes_clear_error(&seg);
        read_len = ct_bytes_take_bytes(&seg, out, SIZE_MAX);
        REQUIRE(read_len == 4);  // should clamp to readable (4)
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // put_bytes with SIZE_MAX
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_clear_error(&seg);
        int write_len = ct_bytes_put_bytes(&seg, out, SIZE_MAX);
        REQUIRE(write_len == 16);  // should clamp to writable (16)
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // fill with SIZE_MAX
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_clear_error(&seg);
        int filled_len = ct_bytes_fill(&seg, 0xAA, SIZE_MAX);
        REQUIRE(filled_len == 16);  // should clamp to writable (16)
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // overfill with SIZE_MAX
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_clear_error(&seg);
        int overfilled_len = ct_bytes_overfill(&seg, 0xBB, SIZE_MAX);
        REQUIRE(overfilled_len == 16);  // should clamp to cap (16)
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("ct_bytes_init and ct_bytes_from with nullptr and zero/non-zero capacity") {
        ct_bytes_t seg;

        // nullptr with 0 capacity
        ct_bytes_init(&seg, nullptr, 0);
        REQUIRE(seg.data == nullptr);
        REQUIRE(seg.cap == 0);
        REQUIRE(seg.len == 0);
        REQUIRE(seg.pos == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // nullptr with non-zero capacity
        ct_bytes_init(&seg, nullptr, 10);
        REQUIRE(seg.data == nullptr);
        REQUIRE(seg.cap == 10);
        REQUIRE(seg.len == 0);
        REQUIRE(seg.pos == 0);

        // Since length is 0, reading should fail and set overflow but not crash
        uint8_t val = 0;
        ct_bytes_clear_error(&seg);
        int read_len = ct_bytes_take_bytes(&seg, &val, 1);
        REQUIRE(read_len == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        REQUIRE(ct_bytes_capacity(&seg) == 10);
        REQUIRE(ct_bytes_writable(&seg) == 10);
    }
}

TEST_CASE("seg Challenger 3 Adversarial Safety Tests" * doctest::test_suite("seg") *
          doctest::test_suite("challenger3")) {
    uint8_t    buffer[64] = {0};
    ct_bytes_t seg;

    SUBCASE("Reseek and Seek with negative / extreme offsets") {
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);

        // negative values cast as size_t should be rejected
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_seek(&seg, (size_t)-1) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_seek(&seg, (size_t)INT_MIN) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_reseek(&seg, (size_t)-1) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_reseek(&seg, (size_t)INT_MIN) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }

    SUBCASE("Peek and Poke relative offset bounds and negative offsets") {
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);  // len = 10
        ct_bytes_seek(&seg, 4);     // pos = 4

        // Negative offset peek wrapping past 0
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u8(&seg, -5) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u16(&seg, -5) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u32(&seg, -5) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u64(&seg, -5) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        // Negative offset poke wrapping past 0
        ct_bytes_clear_error(&seg);
        uint8_t poke_val = 0xAA;
        REQUIRE(ct_bytes_poke_bytes(&seg, -5, &poke_val, 1) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // Positive overflow peek
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_peek_u8(&seg, 6) == 0);  // pos + 6 = 10 == len (out of bounds)
        REQUIRE(ct_bytes_has_error(&seg) == 0);
    }

    SUBCASE("Uninitialized View Initialization and since safety") {
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);

        // Create a view with garbage values to ensure all fields are initialized
        ct_bytes_t view;
        memset(&view, 0x5A, sizeof(view));

        // Call since with valid range
        int res = ct_bytes_since(&seg, &view, 2, 8);
        REQUIRE(res == 0);
        REQUIRE(view.data == buffer + 2);
        REQUIRE(view.cap == 14);  // 16 - 2
        REQUIRE(view.len == 6);   // 8 - 2
        REQUIRE(view.pos == 0);
        REQUIRE(+view.endian == +seg.endian);
        REQUIRE(+view.hlswap == +seg.hlswap);
        REQUIRE(+view.overflow == 0);

        // Call since with invalid range, view should not be modified on failure
        memset(&view, 0x5A, sizeof(view));
        ct_bytes_clear_error(&seg);
        res = ct_bytes_since(&seg, &view, 5, 2);
        REQUIRE(res == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 0);
        // Verify view is untouched
        uint8_t expected_garbage[sizeof(view)];
        memset(expected_garbage, 0x5A, sizeof(expected_garbage));
        REQUIRE(memcmp(&view, expected_garbage, sizeof(view)) == 0);
    }

    SUBCASE("Absolute operations with extreme values") {
        ct_bytes_init(&seg, buffer, 16);
        ct_bytes_commit(&seg, 10);

        // SIZE_MAX offset
        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_get_u8(&seg, SIZE_MAX) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_u8(&seg, SIZE_MAX, 0xFF) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);

        // SIZE_MAX - offset wrap around check in get/set
        ct_bytes_clear_error(&seg);
        uint8_t out_bytes[4];
        REQUIRE(ct_bytes_get_bytes(&seg, SIZE_MAX - 2, out_bytes, 4) == 0);
        REQUIRE(ct_bytes_has_error(&seg) == 0);

        ct_bytes_clear_error(&seg);
        REQUIRE(ct_bytes_set_bytes(&seg, SIZE_MAX - 2, out_bytes, 4) == -1);
        REQUIRE(ct_bytes_has_error(&seg) == 1);
    }
}
