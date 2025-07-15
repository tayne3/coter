/*
 * MIT License
 *
 * Copyright (c) 2025 tayne3
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 * 2. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */
#include <float.h>  // For FLT_MAX, DBL_MAX etc.
#include <limits.h>
#include <math.h>  // For NAN, INFINITY
#include <stdio.h>
#include <string.h>

#include "coter/container/pack.h"
#include "cunit.h"

#define TEST_BUFFER_SIZE 1024

static uint8_t test_buffer[TEST_BUFFER_SIZE] = {0};

// Helper to setup test buffer
static void setup(void) {
	memset(test_buffer, 0x00, sizeof(test_buffer));
	return;
}

static void test_pack_basic(void) {
	setup();

	const uint8_t byte_in = 0xAB;
	assert_int_eq(1, ct_pack(test_buffer, TEST_BUFFER_SIZE, "b", byte_in));
	assert_uint8_eq(test_buffer[0], byte_in);

	uint8_t byte_out = 0;
	assert_int_eq(1, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "b", &byte_out));
	assert_uint8_eq(byte_out, byte_in);

	const char char_in = 'd';
	assert_int_eq(1, ct_pack(test_buffer, TEST_BUFFER_SIZE, "c", char_in));
	assert_int8_eq(test_buffer[0], char_in);

	uint8_t char_out = 0;
	assert_int_eq(1, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "c", &char_out));
	assert_int8_eq(char_out, char_in);

	assert_int_eq(4, ct_pack(test_buffer, TEST_BUFFER_SIZE, "bccb", byte_in, char_in, char_in, byte_in));
	assert_uint8_eq(test_buffer[0], byte_in);
	assert_int8_eq(test_buffer[1], char_in);
	assert_int8_eq(test_buffer[2], char_in);
	assert_uint8_eq(test_buffer[3], byte_in);

	uint8_t o1 = 0, o4 = 0;
	char    o2 = 0, o3 = 0;
	assert_int_eq(4, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "bccb", &o1, &o2, &o3, &o4));
	assert_uint8_eq(o1, byte_in);
	assert_int8_eq(o2, char_in);
	assert_int8_eq(o3, char_in);
	assert_uint8_eq(o4, byte_in);

	const uint8_t expect[] = {0x03, 0x00, 0x87, 0x00, 0x01};
	assert_int_eq(5, ct_pack(test_buffer, TEST_BUFFER_SIZE, "b>HH", 0x03, 0x0087, 0x0001));
	assert_str_hex(test_buffer, expect, sizeof(expect));
}

static void test_pack_integers(void) {
	setup();
	const int16_t  h_in  = -12345;
	const uint16_t H_in  = 54321;
	const int32_t  i_in  = -1234567890;
	const uint32_t I_in  = 3210987654;
	const int64_t  l_in  = -1234567890123456789LL;
	const uint64_t L_in  = 9876543210987654321ULL;
	int16_t        h_out = 0;
	uint16_t       H_out = 0;
	int32_t        i_out = 0;
	uint32_t       I_out = 0;
	int64_t        l_out = 0;
	uint64_t       L_out = 0;

	// Pack
	assert_int_eq(2, ct_pack(test_buffer, TEST_BUFFER_SIZE, "h", h_in));
	assert_int_eq(2 + 2, ct_pack(test_buffer, TEST_BUFFER_SIZE, "hH", h_in, H_in));
	assert_int_eq(2 + 2 + 4, ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHi", h_in, H_in, i_in));
	assert_int_eq(2 + 2 + 4 + 4, ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHiI", h_in, H_in, i_in, I_in));
	assert_int_eq(2 + 2 + 4 + 4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHiIl", h_in, H_in, i_in, I_in, l_in));
	assert_int_eq(2 + 2 + 4 + 4 + 8 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "hHiIlL", h_in, H_in, i_in, I_in, l_in, L_in));

	// Unpack
	int16_t  h_out_t = 0;
	uint16_t H_out_t = 0;
	int32_t  i_out_t = 0;
	uint32_t I_out_t = 0;
	int64_t  l_out_t = 0;
	uint64_t L_out_t = 0;
	assert_int_eq(2 + 2 + 4 + 4 + 8 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "hHiIlL", &h_out_t, &H_out_t, &i_out_t, &I_out_t, &l_out_t, &L_out_t));

	// Verify
	assert_int16_eq(h_out_t, h_in);
	assert_uint16_eq(H_out_t, H_in);
	assert_int32_eq(i_out_t, i_in);
	assert_uint32_eq(I_out_t, I_in);
	assert_int64_eq(l_out_t, l_in);
	assert_uint64_eq(L_out_t, L_in);

	// Test limits
	setup();
	assert_int_eq(2 + 2, ct_pack(test_buffer, TEST_BUFFER_SIZE, "hH", SHRT_MIN, USHRT_MAX));
	assert_int_eq(2 + 2, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "hH", &h_out, &H_out));
	assert_int16_eq(h_out, SHRT_MIN);
	assert_uint16_eq(H_out, USHRT_MAX);

	setup();
	assert_int_eq(4 + 4, ct_pack(test_buffer, TEST_BUFFER_SIZE, "iI", INT_MIN, UINT_MAX));
	assert_int_eq(4 + 4, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "iI", &i_out, &I_out));
	assert_int32_eq(i_out, INT_MIN);
	assert_uint32_eq(I_out, UINT_MAX);

	// Note: long long (used for 'l' and 'L') might not be 8 bytes everywhere,
	// but the pack format assumes 8 bytes. This test assumes 64-bit longs.
	setup();
	assert_int_eq(8 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "lL", LLONG_MIN, ULLONG_MAX));
	assert_int_eq(8 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "lL", &l_out, &L_out));
	assert_int64_eq(l_out, LLONG_MIN);
	assert_uint64_eq(L_out, ULLONG_MAX);
}

static void test_pack_floats(void) {
	setup();
	const float  f_in  = 3.14159f;
	const double d_in  = 2.718281828459045;
	float        f_out = 0.0f;
	double       d_out = 0.0;

	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "fd", f_in, d_in));
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "fd", &f_out, &d_out));
	assert_true(fabsf(f_out - f_in) <= 1e-6f, "Float basic pack/unpack failed");  // Use tolerance for float comparison
	assert_true(fabs(d_out - d_in) <= 1e-15, "Double basic pack/unpack failed");  // Use tolerance for double comparison

	// Test special float values if supported
	setup();
	float  f_nan = NAN;
	double d_inf = INFINITY;
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "fd", f_nan, d_inf));
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "fd", &f_out, &d_out));
	assert_true(isnan(f_out), "Unpacked float should be NaN");
	assert_true(isinf(d_out) && d_out > 0, "Unpacked double should be +Infinity");

	// Test float limits
	setup();
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "fd", FLT_MAX, DBL_MIN));
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "fd", &f_out, &d_out));
	// Use relative tolerance for large/small values
	assert_true(fabsf(f_out - FLT_MAX) <= fabsf(FLT_MAX * 1e-6f), "Float max limit failed");
	assert_true(fabs(d_out - DBL_MIN) <= fabs(DBL_MIN * 1e-15) + DBL_EPSILON, "Double min limit failed");  // Add epsilon for very small numbers
}

static void test_pack_strings(void) {
	setup();
	int         result = 0;
	const char *z_in   = "hello";  // 5 chars + null
	char        z_out[10];
	const char *p_in = "world";  // 5 chars, len=5 (uint8_t)
	char        p_out[10];
	const char *P_in = "testing";  // 7 chars, len=7 (uint16_t)
	char        P_out[10];
	const char *a_in = "pack";  // 4 chars, len=4 (size_t)
	char        a_out[10];
	const char *A5_in = "fixed";  // 5 chars fixed length
	char        A5_out[10];
	char        A0_out[10];  // Test A0 edge case

	// Pack z, p, P, a, A5
	result = 6;
	assert_int_eq(result, ct_pack(test_buffer, TEST_BUFFER_SIZE, "z", z_in));  // "hello\0"
	result += 1 + 5;
	assert_int_eq(result, ct_pack(test_buffer, TEST_BUFFER_SIZE, "zp", z_in, p_in));  // "hello\0" + 0x05 + "world"
	result += 2 + 7;
	assert_int_eq(result, ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpP", z_in, p_in, P_in));  // ... + 0x0007 + "testing"
	result += sizeof(size_t) + 4;
	assert_int_eq(result, ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpPa", z_in, p_in, P_in, a_in));  // ... + size_t(4) + "pack"
	result += 5;
	assert_int_eq(result, ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpPaA5", z_in, p_in, P_in, a_in, A5_in));  // ... + "fixed"
	result += 0;
	assert_int_eq(result, ct_pack(test_buffer, TEST_BUFFER_SIZE, "zpPaA5A0", z_in, p_in, P_in, a_in, A5_in));  // ... + "" (A0 adds nothing)

	// Unpack z, p, P, a, A5, A0
	memset(z_out, 'X', sizeof(z_out));
	memset(p_out, 'X', sizeof(p_out));
	memset(P_out, 'X', sizeof(P_out));
	memset(a_out, 'X', sizeof(a_out));
	memset(A5_out, 'X', sizeof(A5_out));
	memset(A0_out, 'X', sizeof(A0_out));

	// Note: unpack needs separate buffers or careful offset calculation if unpacking from the same buffer packed sequentially.
	// Let's test unpacking individually for clarity.
	setup();  // Repack just z
	assert_int_eq(6, ct_pack(test_buffer, TEST_BUFFER_SIZE, "z", z_in));
	assert_int_eq(6, ct_unpack(test_buffer, 6, "z", z_out));
	assert_true(strcmp(z_out, z_in) == 0, "Unpack 'z' string failed");

	setup();  // Repack just p
	assert_int_eq(1 + 5, ct_pack(test_buffer, TEST_BUFFER_SIZE, "p", p_in));
	assert_int_eq(1 + 5, ct_unpack(test_buffer, 1 + 5, "p", p_out));
	assert_true(strncmp(p_out, p_in, 5) == 0, "Unpack 'p' string failed");  // Compare only 5 chars

	setup();  // Repack just P
	assert_int_eq(2 + 7, ct_pack(test_buffer, TEST_BUFFER_SIZE, "P", P_in));
	assert_int_eq(2 + 7, ct_unpack(test_buffer, 2 + 7, "P", P_out));
	assert_true(strncmp(P_out, P_in, 7) == 0, "Unpack 'P' string failed");  // Compare only 7 chars

	setup();  // Repack just a
	assert_int_eq(sizeof(size_t) + 4, ct_pack(test_buffer, TEST_BUFFER_SIZE, "a", a_in));
	assert_int_eq(sizeof(size_t) + 4, ct_unpack(test_buffer, sizeof(size_t) + 4, "a", a_out));
	assert_true(strncmp(a_out, a_in, 4) == 0, "Unpack 'a' string failed");  // Compare only 4 chars

	setup();  // Repack just A5
	assert_int_eq(5, ct_pack(test_buffer, TEST_BUFFER_SIZE, "A5", A5_in));
	assert_int_eq(5, ct_unpack(test_buffer, 5, "A5", A5_out));
	assert_true(strncmp(A5_out, A5_in, 5) == 0, "Unpack 'A5' string failed");  // Compare only 5 chars

	setup();  // Repack A0 - should write nothing
	assert_int_eq(0, ct_pack(test_buffer, TEST_BUFFER_SIZE, "A0"));
	assert_int_eq(0, ct_unpack(test_buffer, 0, "A0", A0_out));  // Unpacking A0 also reads 0 bytes
	assert_int8_eq(A0_out[0], 'X');
}

static void test_pack_endianness(void) {
	setup();
	const int32_t  i_in  = 0x12345678;
	const uint64_t L_in  = 0xAABBCCDDEEFF0011ULL;
	int32_t        i_out = 0;
	uint64_t       L_out = 0;

	// Pack Little Endian (<)
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "<iL", i_in, L_in));
	// Verify manually based on known little endian representation
	uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12, 0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
	assert_str_hex(test_buffer, expected_le, sizeof(expected_le), "Little Endian pack failed");
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "<iL", &i_out, &L_out));
	assert_int32_eq(i_out, i_in);
	assert_uint64_eq(L_out, L_in);

	// Pack Big Endian (>)
	setup();
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, ">iL", i_in, L_in));
	// Verify manually based on known big endian representation
	uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
	assert_str_hex(test_buffer, expected_be, sizeof(expected_be), "Big Endian pack failed");
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, ">iL", &i_out, &L_out));
	assert_int32_eq(i_out, i_in);
	assert_uint64_eq(L_out, L_in);

	// Pack Native (=) - relies on system endianness check in ct_pack
	setup();
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "=iL", i_in, L_in));
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "=iL", &i_out, &L_out));
	assert_int32_eq(i_out, i_in);
	assert_uint64_eq(L_out, L_in);
}

static void test_pack_swap(void) {
	setup();
	const int32_t  i_in  = 0x12345678;
	const uint64_t L_in  = 0xAABBCCDDEEFF0011ULL;
	int32_t        i_out = 0;
	uint64_t       L_out = 0;

	// Pack with Swap (+) - assumes native endianness first
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "+iL", i_in, L_in));
	// Verify manually (example assumes little endian native + swap)
	// Native LE: 78 56 34 12 | 11 00 FF EE DD CC BB AA
	// Swapped:   56 78 12 34 | 00 11 EE FF CC DD AA BB
	// We need to check this programmatically based on native endianness later.
	// For now, just check if unpack reverses it correctly.
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "+iL", &i_out, &L_out));
	assert_int32_eq(i_out, i_in);
	assert_uint64_eq(L_out, L_in);

	// Pack with No Swap (-) - should behave like native
	setup();
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, "-iL", i_in, L_in));
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "-iL", &i_out, &L_out));
	assert_int32_eq(i_out, i_in);
	assert_uint64_eq(L_out, L_in);

	// Combine Endianness and Swap
	setup();
	// Pack Big Endian + Swap
	assert_int_eq(4 + 8, ct_pack(test_buffer, TEST_BUFFER_SIZE, ">+iL", i_in, L_in));
	// Expected BE: 12 34 56 78 | AA BB CC DD EE FF 00 11
	// Swapped BE:  34 12 78 56 | BB AA DD CC FF EE 11 00
	assert_int_eq(4 + 8, ct_unpack(test_buffer, TEST_BUFFER_SIZE, ">+iL", &i_out, &L_out));
	assert_int32_eq(i_out, i_in);
	assert_uint64_eq(L_out, L_in);
}

static void test_pack_combinations(void) {
	setup();
	const char     c = 'X';
	const uint8_t  b = 0xAB;
	const int16_t  h = -1000;
	const uint32_t I = 0xDEADBEEF;
	const float    f = 1.5f;
	const char    *z = "combo";
	const uint64_t L = 0x1122334455667788ULL;

	char     c_out     = 0;
	uint8_t  b_out     = 0;
	int16_t  h_out     = 0;
	uint32_t I_out     = 0;
	float    f_out     = 0.0f;
	char     z_out[10] = {0};
	uint64_t L_out     = 0;

	// Format: char, byte, little-endian short, big-endian uint32, float, zero-term string, native uint64
	const char *fmt          = "cb<h>I f z =L";
	size_t      expected_len = 1 + 1 + 2 + 4 + 4 + (strlen(z) + 1) + 8;

	assert_int_eq(expected_len, ct_pack(test_buffer, TEST_BUFFER_SIZE, fmt, c, b, h, I, f, z, L));

	assert_int_eq(expected_len, ct_unpack(test_buffer, TEST_BUFFER_SIZE, fmt, &c_out, &b_out, &h_out, &I_out, &f_out, z_out, &L_out));

	assert_int8_eq(c_out, c);
	assert_uint8_eq(b_out, b);
	assert_int16_eq(h_out, h);
	assert_uint32_eq(I_out, I);
	assert_true(fabsf(f_out - f) <= 1e-6f, "Combination float failed");
	assert_true(strcmp(z_out, z) == 0, "Combination 'z' string failed");
	assert_uint64_eq(L_out, L);
}

static void test_pack_edge_cases(void) {
	setup();

	// Buffer too small for pack
	assert_int_eq(0, ct_pack(test_buffer, 1, "h", (int16_t)123));                 // Need 2 bytes, have 1
	assert_int_eq(2, ct_pack(test_buffer, 2, "h", (int16_t)123));                 // Need 2 bytes, have 2 - OK
	assert_int_eq(2, ct_pack(test_buffer, 3, "hi", (int16_t)123, (int32_t)456));  // Need 2+4=6, have 3 - fails after h
	assert_int_eq(6, ct_pack(test_buffer, 6, "hi", (int16_t)123, (int32_t)456));  // Need 6, have 6 - OK

	// Buffer too small for unpack
	int16_t h_out = 0;
	int32_t i_out = 0;
	assert_int_eq(6, ct_pack(test_buffer, 6, "hi", (int16_t)123, (int32_t)456));  // Pack first
	assert_int_eq(0, ct_unpack(test_buffer, 1, "h", &h_out));                     // Need 2, have 1
	assert_int_eq(2, ct_unpack(test_buffer, 2, "h", &h_out));                     // Need 2, have 2 - OK
	assert_int16_eq(h_out, 123);
	assert_int_eq(2, ct_unpack(test_buffer, 5, "hi", &h_out, &i_out));  // Need 6, have 5 - fails after h
	assert_int_eq(6, ct_unpack(test_buffer, 6, "hi", &h_out, &i_out));  // Need 6, have 6 - OK
	assert_int16_eq(h_out, 123);
	assert_int32_eq(i_out, 456);

	// Invalid format character
	assert_int_eq(-1, ct_pack(test_buffer, TEST_BUFFER_SIZE, "bXh", (uint8_t)1, (int16_t)2));
	assert_int_eq(-1, ct_unpack(test_buffer, TEST_BUFFER_SIZE, "bYh", &h_out, &i_out));  // Using wrong vars intentionally

	// Empty format string
	assert_int_eq(0, ct_pack(test_buffer, TEST_BUFFER_SIZE, ""));
	assert_int_eq(0, ct_unpack(test_buffer, TEST_BUFFER_SIZE, ""));
}

int main(void) {
	test_pack_basic();
	cunit_println("Finish! test_pack_basic();");

	test_pack_integers();
	cunit_println("Finish! test_pack_integers();");

	test_pack_floats();
	cunit_println("Finish! test_pack_floats();");

	test_pack_strings();
	cunit_println("Finish! test_pack_strings();");

	test_pack_endianness();
	cunit_println("Finish! test_pack_endianness();");

	test_pack_swap();
	cunit_println("Finish! test_pack_swap();");

	test_pack_combinations();
	cunit_println("Finish! test_pack_combinations();");

	test_pack_edge_cases();
	cunit_println("Finish! test_pack_edge_cases();");

	cunit_pass();
}
