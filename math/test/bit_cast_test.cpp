#include "coter/math/bit_cast.h"

#include <catch.hpp>

TEST_CASE("BitCast: Float32 Conversion", "[bit_cast]") {
	float    f1 = 1.0f;
	uint32_t u1 = ct_bits_from_float32(f1);
	REQUIRE(u1 == 0x3F800000);

	REQUIRE(ct_bits_to_float32(0x3F800000) == 1.0f);

	float    f2 = -1.0f;
	uint32_t u2 = ct_bits_from_float32(f2);
	REQUIRE(u2 == 0xBF800000);
	REQUIRE(ct_bits_to_float32(0xBF800000) == -1.0f);

	float    f3 = 0.0f;
	uint32_t u3 = ct_bits_from_float32(f3);
	REQUIRE(u3 == 0x00000000);
	REQUIRE(ct_bits_to_float32(0x00000000) == 0.0f);

	float    f4 = -0.0f;
	uint32_t u4 = ct_bits_from_float32(f4);
	REQUIRE(u4 == 0x80000000);
	REQUIRE(ct_bits_to_float32(0x80000000) == -0.0f);
}

TEST_CASE("BitCast: Float64 Conversion", "[bit_cast]") {
	double   d1 = 1.0;
	uint64_t u1 = ct_bits_from_float64(d1);
	REQUIRE(u1 == 0x3FF0000000000000ULL);
	REQUIRE(ct_bits_to_float64(0x3FF0000000000000ULL) == 1.0);

	double   d2 = -1.0;
	uint64_t u2 = ct_bits_from_float64(d2);
	REQUIRE(u2 == 0xBFF0000000000000ULL);
	REQUIRE(ct_bits_to_float64(0xBFF0000000000000ULL) == -1.0);
}

TEST_CASE("BitCast: Round Trip", "[bit_cast]") {
	float    f_in   = 123.456f;
	uint32_t f_bits = ct_bits_from_float32(f_in);
	float    f_out  = ct_bits_to_float32(f_bits);
	REQUIRE(f_in == f_out);

	double   d_in   = 9876.54321;
	uint64_t d_bits = ct_bits_from_float64(d_in);
	double   d_out  = ct_bits_to_float64(d_bits);
	REQUIRE(d_in == d_out);
}
