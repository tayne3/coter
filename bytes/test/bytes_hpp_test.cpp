#include <array>
#include <cmath>
#include <limits>

#include "coter/bytes/bytes.hpp"
#include "coter/testing/doctest.h"

TEST_CASE("Constructor" * doctest::test_suite("seg") * doctest::test_suite("ctor")) {
    std::array<uint8_t, 64> buffer{};

    SUBCASE("Default len=0") {
        coter::bytes seg(buffer.data(), buffer.size());
        REQUIRE(seg.capacity() == buffer.size());
        REQUIRE(seg.count() == 0);
        REQUIRE(seg.pos() == 0);
        REQUIRE(seg.data() == buffer.data());
    }

    SUBCASE("Explicit len") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        REQUIRE(seg.capacity() == buffer.size());
        REQUIRE(seg.count() == 32);
        REQUIRE(seg.pos() == 0);
    }

    SUBCASE("len > cap protection") {
        coter::bytes seg(buffer.data(), buffer.size(), 1000);  // len > cap
        REQUIRE(seg.count() == buffer.size());                 // Should be clamped to cap
    }
}

TEST_CASE("Copy Semantics" * doctest::test_suite("seg") * doctest::test_suite("copy")) {
    std::array<uint8_t, 64> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size());
    seg.put<uint32_t>(0xDEADBEEF);

    SUBCASE("Copy constructor") {
        coter::bytes copied(seg);
        REQUIRE(copied.capacity() == seg.capacity());
        REQUIRE(copied.count() == seg.count());
        REQUIRE(copied.pos() == seg.pos());
        REQUIRE(copied.data() == seg.data());

        copied.rewind();
        REQUIRE(copied.pos() == 0);
        REQUIRE(seg.pos() == 4);
    }

    SUBCASE("Copy assignment") {
        std::array<uint8_t, 64> buffer2{};
        coter::bytes            seg2(buffer2.data(), buffer2.size());
        seg2 = seg;
        REQUIRE(seg2.capacity() == seg.capacity());
        REQUIRE(seg2.count() == seg.count());
        REQUIRE(seg2.data() == seg.data());

        seg2.rewind();
        REQUIRE(seg2.pos() == 0);
        REQUIRE(seg.pos() == 4);
    }
}

TEST_CASE("State Queries" * doctest::test_suite("seg") * doctest::test_suite("state")) {
    std::array<uint8_t, 64> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size());

    SUBCASE("Empty buffer state") {
        REQUIRE(seg.is_empty() == true);
        REQUIRE(seg.is_full() == false);
        REQUIRE(seg.capacity() == buffer.size());
        REQUIRE(seg.count() == 0);
        REQUIRE(seg.pos() == 0);
        REQUIRE(seg.readable() == 0);
        REQUIRE(seg.writable() == buffer.size());
        REQUIRE(seg.appendable() == buffer.size());
    }

    SUBCASE("After writes") {
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

    SUBCASE("Full buffer state") {
        coter::bytes full_seg(buffer.data(), buffer.size(), buffer.size());
        REQUIRE(full_seg.is_empty() == false);
        REQUIRE(full_seg.is_full() == true);
        REQUIRE(full_seg.appendable() == 0);
    }
}

TEST_CASE("Read/Write Primitives" * doctest::test_suite("seg") * doctest::test_suite("basic")) {
    std::array<uint8_t, 4096> buffer{};
    coter::bytes              seg(buffer.data(), buffer.size());

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

TEST_CASE("Endianness" * doctest::test_suite("seg") * doctest::test_suite("config")) {
    std::array<uint8_t, 4096> buffer{};
    coter::bytes              seg(buffer.data(), buffer.size());

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

TEST_CASE("High-Low Swap" * doctest::test_suite("seg") * doctest::test_suite("config")) {
    std::array<uint8_t, 4096> buffer{};
    coter::bytes              seg(buffer.data(), buffer.size());

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

TEST_CASE("Peek Primitives" * doctest::test_suite("seg") * doctest::test_suite("peek")) {
    std::array<uint8_t, 4096> buffer{};
    coter::bytes              seg(buffer.data(), buffer.size());

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
TEST_CASE("Peek Bounds" * doctest::test_suite("seg") * doctest::test_suite("peek")) {
    std::array<uint8_t, 10> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size());

    seg.put<uint32_t>(0x12345678);
    seg.rewind();
    REQUIRE(seg.peek<uint32_t>(0) == 0x12345678);
    REQUIRE(seg.peek<uint32_t>(10) == 0);
    REQUIRE(seg.peek<uint64_t>(0) == 0);
    REQUIRE(seg.peek<uint32_t>(-1) == 0);
}

TEST_CASE("Set Primitives" * doctest::test_suite("seg") * doctest::test_suite("set")) {
    std::array<uint8_t, 4096> buffer{};
    coter::bytes              seg(buffer.data(), buffer.size());

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

TEST_CASE("Set Endianness" * doctest::test_suite("seg") * doctest::test_suite("set")) {
    std::array<uint8_t, 4096> buffer{};
    coter::bytes              seg(buffer.data(), buffer.size());

    seg.set_endian(CT_ENDIAN_BIG);
    seg.put<uint32_t>(0x12345678);
    seg.set_endian(CT_ENDIAN_LITTLE);
    REQUIRE(seg.set<uint32_t>(0, 0xAABBCCDD) == 0);
    uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
    REQUIRE(std::memcmp(buffer.data(), expected, 4) == 0);
    seg.rewind();
    REQUIRE(seg.take<uint32_t>() == 0xAABBCCDD);
}

TEST_CASE("Move Semantics" * doctest::test_suite("seg") * doctest::test_suite("move")) {
    std::array<uint8_t, 64> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size());

    seg.put<uint32_t>(0xDEADBEEF);
    coter::bytes moved(std::move(seg));
    REQUIRE(seg.capacity() == 0);
    REQUIRE(seg.data() == nullptr);
    moved.rewind();
    REQUIRE(moved.take<uint32_t>() == 0xDEADBEEF);

    std::array<uint8_t, 64> buffer2{};
    coter::bytes            seg2(buffer2.data(), buffer2.size());
    seg2 = std::move(moved);
    REQUIRE(moved.capacity() == 0);
    REQUIRE(moved.data() == nullptr);
    seg2.rewind();
    REQUIRE(seg2.take<uint32_t>() == 0xDEADBEEF);
}

TEST_CASE("Position Control" * doctest::test_suite("seg") * doctest::test_suite("position")) {
    std::array<uint8_t, 64> buffer{};

    SUBCASE("seek/reseek") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        REQUIRE(seg.seek(16) == 0);
        REQUIRE(seg.pos() == 16);
        REQUIRE(seg.seek(33) == -1);

        REQUIRE(seg.reseek(0) == 0);
        REQUIRE(seg.pos() == 32);
        REQUIRE(seg.reseek(10) == 0);
        REQUIRE(seg.pos() == 22);
        REQUIRE(seg.reseek(33) == -1);
    }

    SUBCASE("skip/commit") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        REQUIRE(seg.skip(10) == 10);
        REQUIRE(seg.pos() == 10);
        REQUIRE(seg.skip(100) == 22);
        REQUIRE(seg.pos() == 32);

        coter::bytes seg2(buffer.data(), buffer.size());
        REQUIRE(seg2.commit(10) == 10);
        REQUIRE(seg2.pos() == 10);
        REQUIRE(seg2.count() == 10);
    }

    SUBCASE("clear/rewind") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        seg.seek(16);

        seg.rewind();
        REQUIRE(seg.pos() == 0);
        REQUIRE(seg.count() == 32);

        seg.clear();
        REQUIRE(seg.pos() == 0);
        REQUIRE(seg.count() == 0);
    }
}

TEST_CASE("IO Operations" * doctest::test_suite("seg") * doctest::test_suite("io")) {
    std::array<uint8_t, 64> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size());

    SUBCASE("write/read bytes") {
        uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
        REQUIRE(seg.put_bytes(data, 5) == 5);
        REQUIRE(seg.pos() == 5);
        REQUIRE(seg.count() == 5);

        seg.rewind();
        uint8_t out[5] = {0};
        REQUIRE(seg.take_bytes(out, 5) == 5);
        REQUIRE(std::memcmp(out, data, 5) == 0);
    }

    SUBCASE("peek_bytes and poke_bytes") {
        uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
        seg.put_bytes(data, 4);
        seg.rewind();

        uint8_t out[4] = {0};
        REQUIRE(seg.peek_bytes(0, out, 4) == 4);
        REQUIRE(std::memcmp(out, data, 4) == 0);
        REQUIRE(seg.pos() == 0);

        uint8_t out2[2] = {0};
        REQUIRE(seg.peek_bytes(2, out2, 2) == 2);
        REQUIRE(out2[0] == 0xCC);
        REQUIRE(out2[1] == 0xDD);

        uint8_t inject[] = {0xEE, 0xFF};
        REQUIRE(seg.poke_bytes(1, inject, 2) == 0);
        REQUIRE(seg.peek_bytes(1, out2, 2) == 2);
        REQUIRE(out2[0] == 0xEE);
        REQUIRE(out2[1] == 0xFF);

        // Boundaries
        REQUIRE(seg.poke_bytes(-1, inject, 2) == -1);
        REQUIRE(seg.poke_bytes(3, inject, 2) == -1);
    }

    SUBCASE("fill pattern") {
        REQUIRE(seg.fill(0xAA, 10) == 10);
        REQUIRE(seg.pos() == 10);
        REQUIRE(seg.count() == 10);
        for (int i = 0; i < 10; ++i) { REQUIRE(buffer[i] == 0xAA); }
    }

    SUBCASE("partial operations") {
        coter::bytes small_seg(buffer.data(), 4);
        uint8_t      data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        REQUIRE(small_seg.put_bytes(data, 8) == 4);

        small_seg.rewind();
        uint8_t out[8] = {0};
        REQUIRE(small_seg.take_bytes(out, 8) == 4);
    }

    SUBCASE("get_bytes and set_bytes") {
        coter::bytes seg(buffer.data(), buffer.size());
        uint8_t      data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
        seg.put_bytes(data, 5);
        seg.rewind();

        uint8_t chunk[3];
        REQUIRE(seg.get_bytes(1, chunk, 3) == 3);
        REQUIRE(chunk[0] == 0x22);
        REQUIRE(chunk[1] == 0x33);
        REQUIRE(chunk[2] == 0x44);

        uint8_t overwrite[] = {0xAA, 0xBB};
        REQUIRE(seg.set_bytes(2, overwrite, 2) == 0);
        REQUIRE(seg.get_bytes(2, chunk, 2) == 2);
        REQUIRE(chunk[0] == 0xAA);
        REQUIRE(chunk[1] == 0xBB);

        // Boundaries
        REQUIRE(seg.set_bytes(4, overwrite, 2) == -1);
        REQUIRE(seg.get_bytes(4, chunk, 3) == 1);
        REQUIRE(chunk[0] == 0x55);
    }
}

TEST_CASE("Handle Access" * doctest::test_suite("seg") * doctest::test_suite("handle")) {
    std::array<uint8_t, 64> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size(), 32);

    SUBCASE("non-const handle") {
        ct_bytes_t* h = seg.handle();
        REQUIRE(h != nullptr);
        REQUIRE(h->data == buffer.data());
        REQUIRE(h->cap == buffer.size());
        REQUIRE(h->len == 32);
    }

    SUBCASE("const handle") {
        const coter::bytes& cseg = seg;
        const ct_bytes_t*   h    = cseg.handle();
        REQUIRE(h != nullptr);
        REQUIRE(h->data == buffer.data());
    }
}

TEST_CASE("Getter Methods" * doctest::test_suite("seg") * doctest::test_suite("config")) {
    std::array<uint8_t, 64> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size());

    SUBCASE("default values") {
        REQUIRE(seg.get_endian() == CT_ENDIAN_BIG);
        REQUIRE(seg.get_hlswap() == 0);
    }

    SUBCASE("get after set") {
        seg.set_endian(CT_ENDIAN_LITTLE);
        REQUIRE(seg.get_endian() == CT_ENDIAN_LITTLE);

        seg.set_hlswap(1);
        REQUIRE(seg.get_hlswap() == 1);
    }
}

TEST_CASE("View Operations" * doctest::test_suite("seg") * doctest::test_suite("view")) {
    std::array<uint8_t, 64> buffer{};
    for (size_t i = 0; i < 64; ++i) buffer[i] = static_cast<uint8_t>(i);

    SUBCASE("since valid range") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        auto         result = seg.since(8, 24);

        REQUIRE(result.has_value());
        REQUIRE(result->data() == buffer.data() + 8);
        REQUIRE(result->capacity() == buffer.size() - 8);
        REQUIRE(result->count() == 16);
        REQUIRE(result->pos() == 0);
    }

    SUBCASE("since default arguments") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);

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

    SUBCASE("since invalid range returns nullopt") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);

        REQUIRE_FALSE(seg.since(24, 8).has_value());   // end < start
        REQUIRE_FALSE(seg.since(0, 100).has_value());  // end > cap
    }

    SUBCASE("since empty range") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        auto         result = seg.since(10, 10);

        REQUIRE(result.has_value());
        REQUIRE(result->count() == 0);
        REQUIRE(result->data() == buffer.data() + 10);
    }

    SUBCASE("since inherits config") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        seg.set_endian(CT_ENDIAN_LITTLE);
        seg.set_hlswap(1);

        auto result = seg.since(0, 16);
        REQUIRE(result.has_value());
        REQUIRE(result->get_endian() == CT_ENDIAN_LITTLE);
        REQUIRE(result->get_hlswap() == 1);
    }

    SUBCASE("readable_since") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        seg.seek(8);

        auto result = seg.readable_since();
        REQUIRE(result.has_value());
        REQUIRE(result->data() == buffer.data() + 8);
        REQUIRE(result->count() == 24);  // [pos, len] = [8, 32]
    }

    SUBCASE("writable_since") {
        coter::bytes seg(buffer.data(), buffer.size(), 32);
        seg.seek(8);

        auto result = seg.writable_since();
        REQUIRE(result.has_value());
        REQUIRE(result->data() == buffer.data() + 8);
        REQUIRE(result->count() == buffer.size() - 8);  // [pos, cap] = [8, 64]
    }

    SUBCASE("compact") {
        coter::bytes seg(buffer.data(), buffer.size(), 16);
        seg.seek(8);

        seg.compact();
        REQUIRE(seg.pos() == 0);
        REQUIRE(seg.count() == 8);
        REQUIRE(buffer[0] == 8);
        REQUIRE(buffer[7] == 15);
    }
}

TEST_CASE("Signed Types" * doctest::test_suite("seg") * doctest::test_suite("types")) {
    std::array<uint8_t, 128> buffer{};
    coter::bytes             seg(buffer.data(), buffer.size());

    SUBCASE("int8_t put/take") {
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

    SUBCASE("int16_t put/take") {
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

    SUBCASE("int32_t put/take") {
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

    SUBCASE("int64_t put/take") {
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

    SUBCASE("signed peek") {
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

    SUBCASE("signed set") {
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

TEST_CASE("Floating Point Types" * doctest::test_suite("seg") * doctest::test_suite("types")) {
    std::array<uint8_t, 128> buffer{};
    coter::bytes             seg(buffer.data(), buffer.size());

    SUBCASE("float put/take/peek/set") {
        seg.put<float>(0.0f);
        seg.put<float>(3.14159f);
        seg.put<float>(-123.456f);

        seg.rewind();
        REQUIRE(seg.take<float>() == 0.0f);
        REQUIRE(seg.take<float>() == doctest::Approx(3.14159f));
        REQUIRE(seg.take<float>() == doctest::Approx(-123.456f));

        seg.rewind();
        REQUIRE(seg.peek<float>(0) == 0.0f);
        REQUIRE(seg.peek<float>(4) == doctest::Approx(3.14159f));

        REQUIRE(seg.set<float>(0, 99.9f) == 0);
        REQUIRE(seg.peek<float>(0) == doctest::Approx(99.9f));
    }

    SUBCASE("double put/take/peek/set") {
        seg.put<double>(0.0);
        seg.put<double>(3.141592653589793);
        seg.put<double>(-123456.789012);

        seg.rewind();
        REQUIRE(seg.take<double>() == 0.0);
        REQUIRE(seg.take<double>() == doctest::Approx(3.141592653589793));
        REQUIRE(seg.take<double>() == doctest::Approx(-123456.789012));

        seg.rewind();
        REQUIRE(seg.peek<double>(0) == 0.0);
        REQUIRE(seg.peek<double>(8) == doctest::Approx(3.141592653589793));

        REQUIRE(seg.set<double>(0, 99.999) == 0);
        REQUIRE(seg.peek<double>(0) == doctest::Approx(99.999));
    }

    SUBCASE("special values") {
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

TEST_CASE("Set All Types" * doctest::test_suite("seg") * doctest::test_suite("set")) {
    std::array<uint8_t, 128> buffer{};
    coter::bytes             seg(buffer.data(), buffer.size());

    SUBCASE("u8/i8") {
        seg.fill(0, 16);
        REQUIRE(seg.set<uint8_t>(0, 0xAB) == 0);
        REQUIRE(seg.set<int8_t>(1, static_cast<int8_t>(-100)) == 0);

        seg.rewind();
        REQUIRE(seg.take<uint8_t>() == 0xAB);
        REQUIRE(seg.take<int8_t>() == static_cast<int8_t>(-100));
    }

    SUBCASE("u16/i16") {
        seg.fill(0, 16);
        REQUIRE(seg.set<uint16_t>(0, 0xABCD) == 0);
        REQUIRE(seg.set<int16_t>(2, static_cast<int16_t>(-12345)) == 0);

        seg.rewind();
        REQUIRE(seg.take<uint16_t>() == 0xABCD);
        REQUIRE(seg.take<int16_t>() == static_cast<int16_t>(-12345));
    }

    SUBCASE("u32/i32/float") {
        seg.fill(0, 16);
        REQUIRE(seg.set<uint32_t>(0, 0xDEADBEEF) == 0);
        REQUIRE(seg.set<int32_t>(4, static_cast<int32_t>(-123456789)) == 0);
        REQUIRE(seg.set<float>(8, 3.14f) == 0);

        seg.rewind();
        REQUIRE(seg.take<uint32_t>() == 0xDEADBEEF);
        REQUIRE(seg.take<int32_t>() == static_cast<int32_t>(-123456789));
        REQUIRE(seg.take<float>() == doctest::Approx(3.14f));
    }

    SUBCASE("u64/i64/double") {
        seg.fill(0, 32);
        REQUIRE(seg.set<uint64_t>(0, 0xDEADBEEFCAFEBABEULL) == 0);
        REQUIRE(seg.set<int64_t>(8, static_cast<int64_t>(-123456789012345LL)) == 0);
        REQUIRE(seg.set<double>(16, 3.141592653589793) == 0);

        seg.rewind();
        REQUIRE(seg.take<uint64_t>() == 0xDEADBEEFCAFEBABEULL);
        REQUIRE(seg.take<int64_t>() == static_cast<int64_t>(-123456789012345LL));
        REQUIRE(seg.take<double>() == doctest::Approx(3.141592653589793));
    }

    SUBCASE("boundary errors") {
        seg.fill(0, 4);
        REQUIRE(seg.set<uint32_t>(4, 0x12345678) == -1);
        REQUIRE(seg.set<uint64_t>(0, 0x1234567890ABCDEFULL) == -1);
    }
}

TEST_CASE("Get Operations" * doctest::test_suite("seg") * doctest::test_suite("get")) {
    std::array<uint8_t, 4096> buffer{};
    coter::bytes              seg(buffer.data(), buffer.size());

    SUBCASE("Get Primitives") {
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

    SUBCASE("Get does not change pos or count") {
        seg.put<uint32_t>(0x12345678);
        seg.rewind();

        auto pos_before   = seg.pos();
        auto count_before = seg.count();

        seg.get<uint32_t>(0);

        REQUIRE(seg.pos() == pos_before);
        REQUIRE(seg.count() == count_before);
    }

    SUBCASE("Get Bounds") {
        coter::bytes small_seg(buffer.data(), 10);
        small_seg.put<uint32_t>(0x12345678);

        REQUIRE(small_seg.get<uint32_t>(0) == 0x12345678);
        REQUIRE(small_seg.get<uint32_t>(10) == 0);
        REQUIRE(small_seg.get<uint64_t>(0) == 0);
        REQUIRE(small_seg.get<uint8_t>(100) == 0);
    }

    SUBCASE("Get Endianness Big") {
        seg.set_endian(CT_ENDIAN_BIG);
        seg.put<uint32_t>(0x11223344);

        REQUIRE(seg.get<uint8_t>(0) == 0x11);
        REQUIRE(seg.get<uint8_t>(1) == 0x22);
        REQUIRE(seg.get<uint8_t>(2) == 0x33);
        REQUIRE(seg.get<uint8_t>(3) == 0x44);
    }

    SUBCASE("Get Endianness Little") {
        seg.set_endian(CT_ENDIAN_LITTLE);
        seg.put<uint32_t>(0x11223344);

        REQUIRE(seg.get<uint8_t>(0) == 0x44);
        REQUIRE(seg.get<uint8_t>(1) == 0x33);
        REQUIRE(seg.get<uint8_t>(2) == 0x22);
        REQUIRE(seg.get<uint8_t>(3) == 0x11);
    }

    SUBCASE("Get All Types") {
        seg.put<uint8_t>(0xAB);
        seg.put<uint16_t>(0xCDEF);
        seg.put<uint32_t>(0x12345678);
        seg.put<uint64_t>(0xFEDCBA9876543210ULL);

        REQUIRE(seg.get<uint8_t>(0) == 0xAB);
        REQUIRE(seg.get<uint16_t>(1) == 0xCDEF);
        REQUIRE(seg.get<uint32_t>(3) == 0x12345678);
        REQUIRE(seg.get<uint64_t>(7) == 0xFEDCBA9876543210ULL);
    }

    SUBCASE("Get signed types") {
        seg.put<int8_t>(-100);
        seg.put<int16_t>(-12345);
        seg.put<int32_t>(-123456789);
        seg.put<int64_t>(-123456789012345LL);

        REQUIRE(seg.get<int8_t>(0) == -100);
        REQUIRE(seg.get<int16_t>(1) == -12345);
        REQUIRE(seg.get<int32_t>(3) == -123456789);
        REQUIRE(seg.get<int64_t>(7) == -123456789012345LL);
    }

    SUBCASE("Get float/double") {
        seg.put<float>(3.14159f);
        seg.put<double>(3.141592653589793);

        REQUIRE(seg.get<float>(0) == doctest::Approx(3.14159f));
        REQUIRE(seg.get<double>(4) == doctest::Approx(3.141592653589793));
    }
}

TEST_CASE("Error Marking Tiers 1-4" * doctest::test_suite("seg") * doctest::test_suite("error")) {
    std::array<uint8_t, 16> buffer{};

    SUBCASE("Tier 1: Feature Coverage - Out-of-bounds sets overflow, valid does not") {
        coter::bytes seg(buffer.data(), 8);
        REQUIRE_FALSE(seg.has_error());

        // Valid operation
        seg.put<uint32_t>(0x12345678);
        REQUIRE_FALSE(seg.has_error());

        // Out-of-bounds put
        seg.put<uint64_t>(0x1122334455667788ULL);
        REQUIRE(seg.has_error());

        // Out-of-bounds put_bytes
        coter::bytes            seg_write(buffer.data(), 8);
        std::array<uint8_t, 10> write_data{};
        seg_write.put_bytes(write_data.data(), 10);
        REQUIRE(seg_write.has_error());

        // Out-of-bounds take
        coter::bytes seg_read(buffer.data(), 8, 4);
        seg_read.take<uint64_t>();
        REQUIRE(seg_read.has_error());

        // Out-of-bounds take_bytes
        coter::bytes            seg_read_bytes(buffer.data(), 8, 4);
        std::array<uint8_t, 10> read_data{};
        seg_read_bytes.take_bytes(read_data.data(), 10);
        REQUIRE(seg_read_bytes.has_error());

        // Out-of-bounds get
        coter::bytes seg_get(buffer.data(), 8, 4);
        seg_get.get<uint32_t>(2);
        REQUIRE_FALSE(seg_get.has_error());

        // Out-of-bounds set
        coter::bytes seg_set(buffer.data(), 8, 4);
        seg_set.set<uint32_t>(2, 0x11111111);
        REQUIRE(seg_set.has_error());

        // Out-of-bounds peek
        coter::bytes seg_peek(buffer.data(), 8, 4);
        seg_peek.commit(4);
        seg_peek.peek<uint8_t>(0);  // pos=4, offset=0 -> index 4 >= len
        REQUIRE_FALSE(seg_peek.has_error());

        // Out-of-bounds poke
        coter::bytes seg_poke(buffer.data(), 8, 4);
        seg_poke.commit(4);
        seg_poke.poke_bytes(-2, write_data.data(), 4);
        REQUIRE(seg_poke.has_error());

        // Out-of-bounds seek/reseek/commit/since
        coter::bytes seg_ctrl(buffer.data(), 8, 4);
        REQUIRE(seg_ctrl.seek(5) == -1);
        REQUIRE(seg_ctrl.has_error());

        coter::bytes seg_ctrl2(buffer.data(), 8, 4);
        REQUIRE(seg_ctrl2.reseek(5) == -1);
        REQUIRE(seg_ctrl2.has_error());

        coter::bytes seg_ctrl3(buffer.data(), 8);
        REQUIRE(seg_ctrl3.commit(10) == 8);
        REQUIRE(seg_ctrl3.has_error());

        coter::bytes seg_ctrl4(buffer.data(), 8, 4);
        auto         view_opt = seg_ctrl4.since(0, 10);
        REQUIRE_FALSE(view_opt.has_value());
        REQUIRE_FALSE(seg_ctrl4.has_error());
    }

    SUBCASE("Tier 2: Boundary & Corner Cases") {
        // Empty segment
        coter::bytes seg_empty(buffer.data(), 8, 0);
        seg_empty.take<uint8_t>();
        REQUIRE(seg_empty.has_error());

        // Zero capacity segment
        coter::bytes seg_zero(nullptr, 0);
        seg_zero.put<uint8_t>(0xAA);
        REQUIRE(seg_zero.has_error());

        // Exact boundary operations
        coter::bytes seg_boundary(buffer.data(), 4);
        seg_boundary.put<uint32_t>(0x12345678);
        REQUIRE_FALSE(seg_boundary.has_error());  // exact boundary write
        seg_boundary.put<uint8_t>(0xAA);
        REQUIRE(seg_boundary.has_error());  // past boundary write

        coter::bytes seg_boundary_read(buffer.data(), 4, 4);
        seg_boundary_read.take<uint32_t>();
        REQUIRE_FALSE(seg_boundary_read.has_error());  // exact boundary read
        seg_boundary_read.take<uint8_t>();
        REQUIRE(seg_boundary_read.has_error());  // past boundary read
    }

    SUBCASE("Tier 3: Cross-Feature Combination - clearing & copying/moving") {
        coter::bytes seg(buffer.data(), 8);
        seg.put<uint64_t>(0x1122334455667788ULL);
        seg.put<uint8_t>(0xAA);  // triggers error
        REQUIRE(seg.has_error());

        // clear_error() resets
        seg.clear_error();
        REQUIRE_FALSE(seg.has_error());

        // clear() resets
        seg.put<uint8_t>(0xAA);  // triggers error again
        REQUIRE(seg.has_error());
        seg.clear();
        REQUIRE_FALSE(seg.has_error());

        // seg copy constructor preserves flag
        coter::bytes seg_orig(buffer.data(), 8);
        seg_orig.put<uint8_t>(0xAA);
        seg_orig.take<uint16_t>();  // error
        REQUIRE(seg_orig.has_error());
        coter::bytes seg_copied(seg_orig);
        REQUIRE(seg_copied.has_error());

        // seg copy assignment preserves flag
        coter::bytes seg_assigned(buffer.data(), 8);
        seg_assigned = seg_orig;
        REQUIRE(seg_assigned.has_error());

        // seg move constructor preserves flag in dest, clears in source
        coter::bytes seg_moved(std::move(seg_orig));
        REQUIRE(seg_moved.has_error());
        REQUIRE_FALSE(seg_orig.has_error());  // moved-from state gets ct_bytes_init which clears error

        // seg move assignment preserves flag in dest, clears in source
        coter::bytes seg_moved_assign(buffer.data(), 8);
        seg_moved_assign = std::move(seg_moved);
        REQUIRE(seg_moved_assign.has_error());
        REQUIRE_FALSE(seg_moved.has_error());

        // coter::byte_buffer (managed memory wrapper) copy and move
        coter::byte_buffer segment_orig(8);
        segment_orig.put<uint8_t>(0xAA);
        segment_orig.take<uint16_t>();  // error
        REQUIRE(segment_orig.has_error());

        // segment copy constructor
        coter::byte_buffer segment_copy(segment_orig);
        REQUIRE(segment_copy.has_error());

        // segment copy assignment
        coter::byte_buffer segment_assigned(8);
        segment_assigned = segment_orig;
        REQUIRE(segment_assigned.has_error());

        // segment move constructor
        coter::byte_buffer segment_moved(std::move(segment_orig));
        REQUIRE(segment_moved.has_error());
        REQUIRE_FALSE(segment_orig.has_error());

        // segment move assignment
        coter::byte_buffer segment_moved_assign(8);
        segment_moved_assign = std::move(segment_moved);
        REQUIRE(segment_moved_assign.has_error());
        REQUIRE_FALSE(segment_moved.has_error());
    }

    SUBCASE("Tier 4: Real-world Workloads - Multi-step parsing scenario") {
        // Protocol: [length: 2 bytes] [type: 1 byte] [payload: N bytes]
        // Buffer: 0x00, 0x05, 0x01, 0xAA, 0xBB (payload is truncated)
        std::array<uint8_t, 5> packet = {0x00, 0x05, 0x01, 0xAA, 0xBB};
        coter::bytes           seg(packet.data(), packet.size(), packet.size());

        REQUIRE_FALSE(seg.has_error());

        uint16_t length = seg.take<uint16_t>();
        REQUIRE(length == 5);
        REQUIRE_FALSE(seg.has_error());

        uint8_t type = seg.take<uint8_t>();
        REQUIRE(type == 1);
        REQUIRE_FALSE(seg.has_error());

        std::array<uint8_t, 5> payload{};
        int                    read_bytes = seg.take_bytes(payload.data(), length);
        REQUIRE(read_bytes < length);
        REQUIRE(seg.has_error());  // parsing fails due to payload overflow
    }
}

TEST_CASE("Extreme Boundary Values" * doctest::test_suite("seg") * doctest::test_suite("extreme")) {
    std::array<uint8_t, 16> buffer{};
    coter::bytes            seg(buffer.data(), buffer.size(), 8);

    SUBCASE("Absolute getters with extreme offsets") {
        seg.clear_error();
        REQUIRE(seg.get<uint8_t>(std::numeric_limits<size_t>::max()) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.get<uint16_t>(std::numeric_limits<size_t>::max() - 1) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.get<uint32_t>(std::numeric_limits<size_t>::max() - 3) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.get<uint64_t>(std::numeric_limits<size_t>::max() - 7) == 0);
        REQUIRE_FALSE(seg.has_error());
    }

    SUBCASE("Absolute setters with extreme offsets") {
        seg.clear_error();
        REQUIRE(seg.set<uint8_t>(std::numeric_limits<size_t>::max(), 0xFF) == -1);
        REQUIRE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.set<uint16_t>(std::numeric_limits<size_t>::max() - 1, 0xFFFF) == -1);
        REQUIRE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.set<uint32_t>(std::numeric_limits<size_t>::max() - 3, 0xFFFFFFFF) == -1);
        REQUIRE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.set<uint64_t>(std::numeric_limits<size_t>::max() - 7, 0xFFFFFFFFFFFFFFFFULL) == -1);
        REQUIRE(seg.has_error());
    }

    SUBCASE("Absolute bytes with extreme offsets") {
        std::array<uint8_t, 8> out{};
        seg.clear_error();
        REQUIRE(seg.get_bytes(std::numeric_limits<size_t>::max(), out.data(), 4) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.set_bytes(std::numeric_limits<size_t>::max(), out.data(), 4) == -1);
        REQUIRE(seg.has_error());
    }

    SUBCASE("Seek/Reseek with extreme offsets") {
        seg.clear_error();
        REQUIRE(seg.seek(std::numeric_limits<size_t>::max()) == -1);
        REQUIRE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.reseek(std::numeric_limits<size_t>::max()) == -1);
        REQUIRE(seg.has_error());
    }

    SUBCASE("Peek/Poke relative offset wrapping") {
        seg.clear_error();
        seg.seek(8);  // pos = 8
        REQUIRE(seg.peek<uint8_t>(std::numeric_limits<int>::max()) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.peek<uint16_t>(std::numeric_limits<int>::max()) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.peek<uint32_t>(std::numeric_limits<int>::max()) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.peek<uint64_t>(std::numeric_limits<int>::max()) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        std::array<uint8_t, 4> out{};
        REQUIRE(seg.peek_bytes(std::numeric_limits<int>::max(), out.data(), 4) == 0);
        REQUIRE_FALSE(seg.has_error());

        seg.clear_error();
        REQUIRE(seg.poke_bytes(std::numeric_limits<int>::max(), out.data(), 4) == -1);
        REQUIRE(seg.has_error());
    }
}
