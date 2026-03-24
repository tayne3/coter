#include "coter/encoding/pack.h"

#include <catch.hpp>

#include "coter/encoding/hex.h"

#define TEST_BUFFER_SIZE 1024
static uint8_t test_buffer[TEST_BUFFER_SIZE] = {0};

static inline void setup_buf(void) {
    memset(test_buffer, 0x00, sizeof(test_buffer));
}

static inline std::string hex_of(const uint8_t* buf, size_t n) {
    char out[4096] = {0};
    ct_hex_encode(buf, n, out, sizeof(out));
    return std::string(out);
}

TEST_CASE("pack_basic", "[pack]") {
    setup_buf();
    const uint8_t byte_in = 0xAB;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "B", byte_in) == 1);
    REQUIRE(test_buffer[0] == byte_in);
    uint8_t byte_out = 0;
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "B", &byte_out) == 1);
    REQUIRE(byte_out == byte_in);
    const char char_in = 'd';
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "c", char_in) == 1);
    REQUIRE(test_buffer[0] == (uint8_t)char_in);
    char char_out = 0;
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "c", &char_out) == 1);
    REQUIRE(char_out == char_in);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "BBcB", byte_in, byte_in, char_in, byte_in) == 4);
    REQUIRE(test_buffer[0] == byte_in);
    REQUIRE(test_buffer[1] == byte_in);
    REQUIRE(test_buffer[2] == (uint8_t)char_in);
    REQUIRE(test_buffer[3] == byte_in);
    uint8_t o1 = 0, o2 = 0, o3 = 0, o4 = 0;
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "BBcB", &o1, &o2, &o3, &o4) == 4);
    REQUIRE(o1 == byte_in);
    REQUIRE(o2 == byte_in);
    REQUIRE(o3 == (uint8_t)char_in);
    REQUIRE(o4 == byte_in);
    const uint8_t expect[] = {0xAB, 0x00, 0x87, 0x00, 0x01};
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "B>H H", byte_in, 0x0087, 0x0001) == 5);
    REQUIRE(hex_of(test_buffer, 5) == hex_of(expect, 5));
}

TEST_CASE("pack_int8", "[pack]") {
    setup_buf();
    const int8_t  b_in  = -123;
    const uint8_t B_in  = 200;
    int8_t        b_out = 0;
    uint8_t       B_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "bB", b_in, B_in) == 2);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "bB", &b_out, &B_out) == 2);
    REQUIRE(b_out == b_in);
    REQUIRE(B_out == B_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "b", SCHAR_MIN) == 1);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "b", &b_out) == 1);
    REQUIRE(b_out == SCHAR_MIN);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "B", UCHAR_MAX) == 1);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "B", &B_out) == 1);
    REQUIRE(B_out == UCHAR_MAX);
}

TEST_CASE("pack_int16", "[pack]") {
    setup_buf();
    const int16_t  h_in  = -12345;
    const uint16_t H_in  = 54321;
    int16_t        h_out = 0;
    uint16_t       H_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "h", h_in) == 2);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "h", &h_out) == 2);
    REQUIRE(h_out == h_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hH", h_in, H_in) == 4);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "hH", &h_out, &H_out) == 4);
    REQUIRE(h_out == h_in);
    REQUIRE(H_out == H_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hH", SHRT_MIN, USHRT_MAX) == 4);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "hH", &h_out, &H_out) == 4);
    REQUIRE(h_out == SHRT_MIN);
    REQUIRE(H_out == USHRT_MAX);
}

TEST_CASE("pack_int32", "[pack]") {
    setup_buf();
    const int32_t  i_in  = -1234567890;
    const uint32_t I_in  = 3210987654U;
    int32_t        i_out = 0;
    uint32_t       I_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "iI", i_in, I_in) == 8);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "iI", &i_out, &I_out) == 8);
    REQUIRE(i_out == i_in);
    REQUIRE(I_out == I_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "iI", INT_MIN, UINT_MAX) == 8);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "iI", &i_out, &I_out) == 8);
    REQUIRE(i_out == INT_MIN);
    REQUIRE(I_out == UINT_MAX);
}

TEST_CASE("pack_long", "[pack]") {
    setup_buf();
    const long          l_in         = -1234567890L;
    const unsigned long L_in         = 3210987654UL;
    long                l_out        = 0;
    unsigned long       L_out        = 0;
    const int           expected_len = (int)(sizeof(l_in) + sizeof(L_in));
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "lL", l_in, L_in) == expected_len);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "lL", &l_out, &L_out) == expected_len);
    REQUIRE(l_out == l_in);
    REQUIRE(L_out == L_in);
}

TEST_CASE("pack_int64", "[pack]") {
    setup_buf();
    const int64_t  q_in  = -1234567890123456789LL;
    const uint64_t Q_in  = 9876543210987654321ULL;
    int64_t        q_out = 0;
    uint64_t       Q_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "qQ", q_in, Q_in) == 16);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "qQ", &q_out, &Q_out) == 16);
    REQUIRE(q_out == q_in);
    REQUIRE(Q_out == Q_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "qQ", LLONG_MIN, ULLONG_MAX) == 16);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "qQ", &q_out, &Q_out) == 16);
    REQUIRE(q_out == LLONG_MIN);
    REQUIRE(Q_out == ULLONG_MAX);
}

TEST_CASE("pack_bool", "[pack]") {
    setup_buf();
    uint8_t true_in = 1, false_in = 0;
    uint8_t true_out = 0, false_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "??", true_in, false_in) == 2);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "??", &true_out, &false_out) == 2);
    REQUIRE(true_out == 1);
    REQUIRE(false_out == 0);
}

TEST_CASE("pack_floats", "[pack]") {
    setup_buf();
    const float  f_in  = 3.14159f;
    const double d_in  = 2.718281828459045;
    float        f_out = 0.0f;
    double       d_out = 0.0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "fd", f_in, d_in) == 12);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "fd", &f_out, &d_out) == 12);
    REQUIRE(std::fabs(f_out - f_in) <= 1e-6f);
    REQUIRE(std::fabs(d_out - d_in) <= 1e-15);
    setup_buf();
    float  f_nan = NAN;
    double d_inf = INFINITY;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "fd", f_nan, d_inf) == 12);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "fd", &f_out, &d_out) == 12);
    REQUIRE(std::isnan(f_out));
    REQUIRE(std::isinf(d_out));
    REQUIRE(d_out > 0);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "fd", FLT_MAX, DBL_MIN) == 12);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "fd", &f_out, &d_out) == 12);
    REQUIRE(std::fabs(f_out - FLT_MAX) <= std::fabs(FLT_MAX * 1e-6f));
    REQUIRE(std::fabs(d_out - DBL_MIN) <= std::fabs(DBL_MIN * 1e-15) + DBL_EPSILON);
}

TEST_CASE("pack_strings", "[pack]") {
    setup_buf();
    const char* z_in      = "hello";
    char        z_out[16] = {0};
    const char* p_in      = "world";
    char        p_out[16] = {0};
    const char* s3_in     = "abc";
    char        s3_out[8] = {0};
    const char* s5_in     = "fixed";
    char        s5_out[8] = {0};
    const char* s0_in     = "";
    char        s0_out[8] = {0};
    size_t      result    = 0;
    result += (strlen(z_in) + 1);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "z", z_in) == (int)result);
    result += 1 + strlen(p_in);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zp", z_in, p_in) == (int)result);
    result += 3;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zps3", z_in, p_in, s3_in) == (int)result);
    result += 5;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zps3s5", z_in, p_in, s3_in, s5_in) == (int)result);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zps3s5s0", z_in, p_in, s3_in, s5_in, s0_in) == (int)result);

    memset(z_out, 'X', sizeof(z_out));
    memset(p_out, 'X', sizeof(p_out));
    memset(s3_out, 'X', sizeof(s3_out));
    memset(s5_out, 'X', sizeof(s5_out));
    memset(s0_out, 'X', sizeof(s0_out));

    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "z", z_in) == 6);
    REQUIRE(ct_unpack(test_buffer, 6, "z", z_out) == 6);
    REQUIRE(strcmp(z_out, z_in) == 0);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "p", p_in) == 6);
    REQUIRE(ct_unpack(test_buffer, 6, "p", p_out) == 6);
    REQUIRE(strncmp(p_out, p_in, 5) == 0);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "s3", s3_in) == 3);
    REQUIRE(ct_unpack(test_buffer, 3, "s3", s3_out) == 3);
    REQUIRE(strncmp(s3_out, s3_in, 3) == 0);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "s5", s5_in) == 5);
    REQUIRE(ct_unpack(test_buffer, 5, "s5", s5_out) == 5);
    REQUIRE(strncmp(s5_out, s5_in, 5) == 0);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "s0") == 0);
    REQUIRE(ct_unpack(test_buffer, 0, "s0", s0_out) == 0);
    REQUIRE((int8_t)s0_out[0] == 'X');
}

TEST_CASE("pack_A_alias", "[pack]") {
    setup_buf();
    const char* s_in     = "test";
    char        s_out[8] = {0};
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "A4", s_in) == 4);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "s4", s_in) == 4);
    REQUIRE(ct_unpack(test_buffer, 4, "A4", s_out) == 4);
    REQUIRE(strncmp(s_out, s_in, 4) == 0);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "A4", s_in) == 4);
    memset(s_out, 'X', sizeof(s_out));
    REQUIRE(ct_unpack(test_buffer, 4, "s4", s_out) == 4);
    REQUIRE(strncmp(s_out, s_in, 4) == 0);
}

TEST_CASE("pack_endianness", "[pack]") {
    setup_buf();
    const int32_t i_in  = 0x12345678;
    const int64_t q_in  = 0xAABBCCDDEEFF0011LL;
    int32_t       i_out = 0;
    int64_t       q_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "<i q", i_in, q_in) == 12);
    uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12, 0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
    REQUIRE(hex_of(test_buffer, sizeof(expected_le)) == hex_of(expected_le, sizeof(expected_le)));
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "<i q", &i_out, &q_out) == 12);
    REQUIRE(i_out == i_in);
    REQUIRE(q_out == q_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, ">i q", i_in, q_in) == 12);
    uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    REQUIRE(hex_of(test_buffer, sizeof(expected_be)) == hex_of(expected_be, sizeof(expected_be)));
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, ">i q", &i_out, &q_out) == 12);
    REQUIRE(i_out == i_in);
    REQUIRE(q_out == q_in);
}

TEST_CASE("pack_endianness_network", "[pack]") {
    setup_buf();
    const int32_t i_in  = 0x12345678;
    int32_t       i_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "!i", i_in) == 4);
    uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78};
    REQUIRE(hex_of(test_buffer, sizeof(expected_be)) == hex_of(expected_be, sizeof(expected_be)));
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "!i", &i_out) == 4);
    REQUIRE(i_out == i_in);
}

TEST_CASE("pack_native_align", "[pack]") {
    setup_buf();
    const int32_t i_in  = 0x12345678;
    int32_t       i_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "@i", i_in) == 4);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "@i", &i_out) == 4);
    REQUIRE(i_out == i_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "=i", i_in) == 4);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "=i", &i_out) == 4);
    REQUIRE(i_out == i_in);
}

TEST_CASE("pack_swap", "[pack]") {
    setup_buf();
    const int32_t i_in  = 0x12345678;
    const int64_t q_in  = 0xAABBCCDDEEFF0011LL;
    int32_t       i_out = 0;
    int64_t       q_out = 0;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "+i q", i_in, q_in) == 12);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "+i q", &i_out, &q_out) == 12);
    REQUIRE(i_out == i_in);
    REQUIRE(q_out == q_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "-i q", i_in, q_in) == 12);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "-i q", &i_out, &q_out) == 12);
    REQUIRE(i_out == i_in);
    REQUIRE(q_out == q_in);
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, ">+i q", i_in, q_in) == 12);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, ">+i q", &i_out, &q_out) == 12);
    REQUIRE(i_out == i_in);
    REQUIRE(q_out == q_in);
}

TEST_CASE("pack_swap_hex_verification", "[pack]") {
    // Test '+' byte swapping with actual byte verification
    // '+' swaps adjacent bytes in memory (little-endian layout)
    setup_buf();
    const int32_t i_in = 0x12345678;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "+i", i_in) == 4);
    // Little-endian: 0x12345678 -> {0x78, 0x56, 0x34, 0x12}
    // '+' swaps adjacent: {0x56, 0x78, 0x12, 0x34}
    uint8_t expected_swap[] = {0x56, 0x78, 0x12, 0x34};
    REQUIRE(hex_of(test_buffer, 4) == hex_of(expected_swap, 4));

    // Verify round-trip
    int32_t i_out = 0;
    REQUIRE(ct_unpack(test_buffer, 4, "+i", &i_out) == 4);
    REQUIRE(i_out == i_in);

    // Test with 16-bit value
    setup_buf();
    const int16_t h_in = 0x1234;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "+h", h_in) == 2);
    // Little-endian: 0x1234 -> {0x34, 0x12}
    // '+' swaps adjacent: {0x12, 0x34}
    uint8_t expected_h_swap[] = {0x12, 0x34};
    REQUIRE(hex_of(test_buffer, 2) == hex_of(expected_h_swap, 2));

    // Test '+' combined with big-endian
    setup_buf();
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, ">+i", i_in) == 4);
    // Big-endian: 0x12345678 -> {0x12, 0x34, 0x56, 0x78}
    // Then '+' swaps: {0x34, 0x12, 0x78, 0x56}
    uint8_t expected_be_swap[] = {0x34, 0x12, 0x78, 0x56};
    REQUIRE(hex_of(test_buffer, 4) == hex_of(expected_be_swap, 4));

    // Test 64-bit with '+'
    setup_buf();
    const int64_t q_in = 0xAABBCCDDEEFF0011LL;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "+q", q_in) == 8);
    // Little-endian: 0xAABBCCDDEEFF0011 -> {0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA}
    // '+' swaps adjacent: {0x00, 0x11, 0xEE, 0xFF, 0xCC, 0xDD, 0xAA, 0xBB}
    uint8_t expected_q_swap[] = {0x00, 0x11, 0xEE, 0xFF, 0xCC, 0xDD, 0xAA, 0xBB};
    REQUIRE(hex_of(test_buffer, 8) == hex_of(expected_q_swap, 8));
}

TEST_CASE("pack_combinations", "[pack]") {
    setup_buf();
    const char     c            = 'X';
    const uint8_t  B            = 0xAB;
    const int16_t  h            = -1000;
    const uint32_t I            = 0xDEADBEEF;
    const float    f            = 1.5f;
    const char*    z            = "combo";
    const int64_t  q            = 0x1122334455667788LL;
    char           c_out        = 0;
    uint8_t        B_out        = 0;
    int16_t        h_out        = 0;
    uint32_t       I_out        = 0;
    float          f_out        = 0.0f;
    char           z_out[16]    = {0};
    int64_t        q_out        = 0;
    const char*    fmt          = "cB<h>I f z =q";
    size_t         expected_len = 1 + 1 + 2 + 4 + 4 + (strlen(z) + 1) + 8;
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, fmt, c, B, h, I, f, z, q) == static_cast<int>(expected_len));
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, fmt, &c_out, &B_out, &h_out, &I_out, &f_out, z_out, &q_out) ==
            static_cast<int>(expected_len));
    REQUIRE(c_out == c);
    REQUIRE(B_out == B);
    REQUIRE(h_out == h);
    REQUIRE(I_out == I);
    REQUIRE(std::fabs(f_out - f) <= 1e-6f);
    REQUIRE(strcmp(z_out, z) == 0);
    REQUIRE(q_out == q);
}

TEST_CASE("pack_edge_cases", "[pack]") {
    setup_buf();
    REQUIRE(ct_pack(test_buffer, 1, "h", (int16_t)123) == 0);
    REQUIRE(ct_pack(test_buffer, 2, "h", (int16_t)123) == 2);
    REQUIRE(ct_pack(test_buffer, 3, "hi", (int16_t)123, (int32_t)456) == 2);
    REQUIRE(ct_pack(test_buffer, 6, "hi", (int16_t)123, (int32_t)456) == 6);
    int16_t h_out = 0;
    int32_t i_out = 0;
    REQUIRE(ct_pack(test_buffer, 6, "hi", (int16_t)123, (int32_t)456) == 6);
    REQUIRE(ct_unpack(test_buffer, 1, "h", &h_out) == 0);
    REQUIRE(ct_unpack(test_buffer, 2, "h", &h_out) == 2);
    REQUIRE(h_out == 123);
    REQUIRE(ct_unpack(test_buffer, 5, "hi", &h_out, &i_out) == 2);
    REQUIRE(ct_unpack(test_buffer, 6, "hi", &h_out, &i_out) == 6);
    REQUIRE(h_out == 123);
    REQUIRE(i_out == 456);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "BXh", (uint8_t)1, (int16_t)2) == -1);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "BYh", &h_out, &i_out) == -1);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "") == 0);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "") == 0);
}

TEST_CASE("pack_null_buf_or_fmt", "[pack]") {
    REQUIRE(ct_pack(NULL, 10, "h", (int16_t)1) == -1);
    REQUIRE(ct_pack(test_buffer, 10, NULL, (int16_t)1) == -1);
    REQUIRE(ct_unpack(NULL, 10, "h", NULL) == -1);
    REQUIRE(ct_unpack(test_buffer, 10, NULL, NULL) == -1);
}

TEST_CASE("pack_invalid_fmt", "[pack]") {
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "X") == -1);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "Y", NULL) == -1);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "s") == -1);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "s", NULL) == -1);
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "A") == -1);
    REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "A", NULL) == -1);
}

TEST_CASE("pack_binary_string_with_zeros", "[pack]") {
    setup_buf();
    const char data_in[]   = "\x00\x01\x02\x00\x03";
    char       data_out[8] = {0};
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "s5", data_in) == 5);
    REQUIRE(ct_unpack(test_buffer, 5, "s5", data_out) == 5);
    REQUIRE(memcmp(data_in, data_out, 5) == 0);
}

TEST_CASE("pack_pascal_string_truncation", "[pack]") {
    setup_buf();
    const char* long_str =
        "this string is way longer than 255 characters and will definitely cause truncation if we try to pack it as a "
        "pascal string because pascal strings can only hold 1 byte for length which means maximum 255 bytes of data "
        "can be stored in a pascal string format and anything beyond that will be truncated or cause an error";
    REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "p", long_str) == -1);
}
