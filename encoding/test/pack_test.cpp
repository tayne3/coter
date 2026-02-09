#include "coter/encoding/pack.h"

#include <catch.hpp>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstring>

#include "coter/encoding/hex.h"

#define TEST_BUFFER_SIZE 1024
static uint8_t test_buffer[TEST_BUFFER_SIZE] = {0};

static inline void setup_buf(void) {
	std::memset(test_buffer, 0x00, sizeof(test_buffer));
}

static inline std::string hex_of(const uint8_t *buf, size_t n) {
	char out[4096] = {0};
	ct_hex_encode(buf, n, out, sizeof(out));
	return std::string(out);
}

TEST_CASE("pack_basic", "[pack]") {
	setup_buf();
	const uint8_t byte_in = 0xAB;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "b", byte_in) == 1);
	REQUIRE(test_buffer[0] == byte_in);
	uint8_t byte_out = 0;
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "b", &byte_out) == 1);
	REQUIRE(byte_out == byte_in);
	const char char_in = 'd';
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "c", char_in) == 1);
	REQUIRE((int8_t)test_buffer[0] == char_in);
	uint8_t char_out = 0;
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "c", &char_out) == 1);
	REQUIRE((int8_t)char_out == char_in);
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "bccb", byte_in, char_in, char_in, byte_in) == 4);
	REQUIRE(test_buffer[0] == byte_in);
	REQUIRE((int8_t)test_buffer[1] == char_in);
	REQUIRE((int8_t)test_buffer[2] == char_in);
	REQUIRE(test_buffer[3] == byte_in);
	uint8_t o1 = 0, o4 = 0;
	char    o2 = 0, o3 = 0;
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "bccb", &o1, &o2, &o3, &o4) == 4);
	REQUIRE(o1 == byte_in);
	REQUIRE(o2 == char_in);
	REQUIRE(o3 == char_in);
	REQUIRE(o4 == byte_in);
	const uint8_t expect[] = {0x03, 0x00, 0x87, 0x00, 0x01};
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "b>HH", 0x03, 0x0087, 0x0001) == 5);
	REQUIRE(hex_of(test_buffer, sizeof(expect)) == hex_of(expect, sizeof(expect)));
}

TEST_CASE("pack_integers", "[pack]") {
	setup_buf();
	const int16_t  h_in = -12345;
	const uint16_t H_in = 54321;
	const int32_t  i_in = -1234567890;
	const uint32_t I_in = 3210987654U;
	const int64_t  l_in = -1234567890123456789LL;
	const uint64_t L_in = 9876543210987654321ULL;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "h", h_in) == 2);
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hH", h_in, H_in) == 4);
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHi", h_in, H_in, i_in) == 8);
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHiI", h_in, H_in, i_in, I_in) == 12);
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHiIl", h_in, H_in, i_in, I_in, l_in) == 20);
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHiIlL", h_in, H_in, i_in, I_in, l_in, L_in) == 28);
	int16_t  h_out = 0;
	uint16_t H_out = 0;
	int32_t  i_out = 0;
	uint32_t I_out = 0;
	int64_t  l_out = 0;
	uint64_t L_out = 0;
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "hHiIlL", &h_out, &H_out, &i_out, &I_out, &l_out, &L_out) == 28);
	REQUIRE(h_out == h_in);
	REQUIRE(H_out == H_in);
	REQUIRE(i_out == i_in);
	REQUIRE(I_out == I_in);
	REQUIRE(l_out == l_in);
	REQUIRE(L_out == L_in);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "hH", SHRT_MIN, USHRT_MAX) == 4);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "hH", &h_out, &H_out) == 4);
	REQUIRE(h_out == SHRT_MIN);
	REQUIRE(H_out == USHRT_MAX);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "iI", INT_MIN, UINT_MAX) == 8);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "iI", &i_out, &I_out) == 8);
	REQUIRE(i_out == INT_MIN);
	REQUIRE(I_out == UINT_MAX);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "lL", LLONG_MIN, ULLONG_MAX) == 16);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "lL", &l_out, &L_out) == 16);
	REQUIRE(l_out == LLONG_MIN);
	REQUIRE(L_out == ULLONG_MAX);
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
	int         result = 0;
	const char *z_in   = "hello";
	char        z_out[10];
	const char *p_in = "world";
	char        p_out[10];
	const char *P_in = "testing";
	char        P_out[10];
	const char *a_in = "pack";
	char        a_out[10];
	const char *A5_in = "fixed";
	char        A5_out[10];
	char        A0_out[10];
	result = 6;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "z", z_in) == result);
	result += 1 + 5;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zp", z_in, p_in) == result);
	result += 2 + 7;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpP", z_in, p_in, P_in) == result);
	result += sizeof(size_t) + 4;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpPa", z_in, p_in, P_in, a_in) == result);
	result += 5;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpPaA5", z_in, p_in, P_in, a_in, A5_in) == result);
	result += 0;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpPaA5A0", z_in, p_in, P_in, a_in, A5_in) == result);
	std::memset(z_out, 'X', sizeof(z_out));
	std::memset(p_out, 'X', sizeof(p_out));
	std::memset(P_out, 'X', sizeof(P_out));
	std::memset(a_out, 'X', sizeof(a_out));
	std::memset(A5_out, 'X', sizeof(A5_out));
	std::memset(A0_out, 'X', sizeof(A0_out));
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "z", z_in) == 6);
	REQUIRE(ct_unpack(test_buffer, 6, "z", z_out) == 6);
	REQUIRE(std::strcmp(z_out, z_in) == 0);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "p", p_in) == 6);
	REQUIRE(ct_unpack(test_buffer, 6, "p", p_out) == 6);
	REQUIRE(std::strncmp(p_out, p_in, 5) == 0);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "P", P_in) == 9);
	REQUIRE(ct_unpack(test_buffer, 9, "P", P_out) == 9);
	REQUIRE(std::strncmp(P_out, P_in, 7) == 0);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "a", a_in) == sizeof(size_t) + 4);
	REQUIRE(ct_unpack(test_buffer, sizeof(size_t) + 4, "a", a_out) == sizeof(size_t) + 4);
	REQUIRE(std::strncmp(a_out, a_in, 4) == 0);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "A5", A5_in) == 5);
	REQUIRE(ct_unpack(test_buffer, 5, "A5", A5_out) == 5);
	REQUIRE(std::strncmp(A5_out, A5_in, 5) == 0);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "A0") == 0);
	REQUIRE(ct_unpack(test_buffer, 0, "A0", A0_out) == 0);
	REQUIRE((int8_t)A0_out[0] == 'X');
}

TEST_CASE("pack_endianness", "[pack]") {
	setup_buf();
	const int32_t  i_in  = 0x12345678;
	const uint64_t L_in  = 0xAABBCCDDEEFF0011ULL;
	int32_t        i_out = 0;
	uint64_t       L_out = 0;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "<iL", i_in, L_in) == 12);
	uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12, 0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
	REQUIRE(hex_of(test_buffer, sizeof(expected_le)) == hex_of(expected_le, sizeof(expected_le)));
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "<iL", &i_out, &L_out) == 12);
	REQUIRE(i_out == i_in);
	REQUIRE(L_out == L_in);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, ">iL", i_in, L_in) == 12);
	uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
	REQUIRE(hex_of(test_buffer, sizeof(expected_be)) == hex_of(expected_be, sizeof(expected_be)));
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, ">iL", &i_out, &L_out) == 12);
	REQUIRE(i_out == i_in);
	REQUIRE(L_out == L_in);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "=iL", i_in, L_in) == 12);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "=iL", &i_out, &L_out) == 12);
	REQUIRE(i_out == i_in);
	REQUIRE(L_out == L_in);
}

TEST_CASE("pack_swap", "[pack]") {
	setup_buf();
	const int32_t  i_in  = 0x12345678;
	const uint64_t L_in  = 0xAABBCCDDEEFF0011ULL;
	int32_t        i_out = 0;
	uint64_t       L_out = 0;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "+iL", i_in, L_in) == 12);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "+iL", &i_out, &L_out) == 12);
	REQUIRE(i_out == i_in);
	REQUIRE(L_out == L_in);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "-iL", i_in, L_in) == 12);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "-iL", &i_out, &L_out) == 12);
	REQUIRE(i_out == i_in);
	REQUIRE(L_out == L_in);
	setup_buf();
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, ">+iL", i_in, L_in) == 12);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, ">+iL", &i_out, &L_out) == 12);
	REQUIRE(i_out == i_in);
	REQUIRE(L_out == L_in);
}

TEST_CASE("pack_combinations", "[pack]") {
	setup_buf();
	const char     c            = 'X';
	const uint8_t  b            = 0xAB;
	const int16_t  h            = -1000;
	const uint32_t I            = 0xDEADBEEF;
	const float    f            = 1.5f;
	const char    *z            = "combo";
	const uint64_t L            = 0x1122334455667788ULL;
	char           c_out        = 0;
	uint8_t        b_out        = 0;
	int16_t        h_out        = 0;
	uint32_t       I_out        = 0;
	float          f_out        = 0.0f;
	char           z_out[10]    = {0};
	uint64_t       L_out        = 0;
	const char    *fmt          = "cb<h>I f z =L";
	size_t         expected_len = 1 + 1 + 2 + 4 + 4 + (std::strlen(z) + 1) + 8;
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, fmt, c, b, h, I, f, z, L) == expected_len);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, fmt, &c_out, &b_out, &h_out, &I_out, &f_out, z_out, &L_out) == expected_len);
	REQUIRE(c_out == c);
	REQUIRE(b_out == b);
	REQUIRE(h_out == h);
	REQUIRE(I_out == I);
	REQUIRE(std::fabs(f_out - f) <= 1e-6f);
	REQUIRE(std::strcmp(z_out, z) == 0);
	REQUIRE(L_out == L);
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
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "bXh", (uint8_t)1, (int16_t)2) == -1);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "bYh", &h_out, &i_out) == -1);
	REQUIRE(ct_pack(test_buffer, TEST_BUFFER_SIZE, "") == 0);
	REQUIRE(ct_unpack(test_buffer, TEST_BUFFER_SIZE, "") == 0);
}
