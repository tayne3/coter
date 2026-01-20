#include "coter/math/bit_cast.h"

#include "cunit.h"

static void test_float32_conversion(void) {
	float    f1 = 1.0f;
	uint32_t u1 = ct_float32_to_bits(f1);
	assert_uint32_eq(u1, 0x3F800000);

	assert_float_eq(ct_float32_from_bits(0x3F800000), 1.0f);

	float    f2 = -1.0f;
	uint32_t u2 = ct_float32_to_bits(f2);
	assert_uint32_eq(u2, 0xBF800000);
	assert_float_eq(ct_float32_from_bits(0xBF800000), -1.0f);

	float    f3 = 0.0f;
	uint32_t u3 = ct_float32_to_bits(f3);
	assert_uint32_eq(u3, 0x00000000);
	assert_float_eq(ct_float32_from_bits(0x00000000), 0.0f);

	float    f4 = -0.0f;
	uint32_t u4 = ct_float32_to_bits(f4);
	assert_uint32_eq(u4, 0x80000000);
	assert_float_eq(ct_float32_from_bits(0x80000000), -0.0f);
}

static void test_float64_conversion(void) {
	double   d1 = 1.0;
	uint64_t u1 = ct_float64_to_bits(d1);
	assert_uint64_eq(u1, 0x3FF0000000000000ULL);
	assert_double_eq(ct_float64_from_bits(0x3FF0000000000000ULL), 1.0);

	double   d2 = -1.0;
	uint64_t u2 = ct_float64_to_bits(d2);
	assert_uint64_eq(u2, 0xBFF0000000000000ULL);
	assert_double_eq(ct_float64_from_bits(0xBFF0000000000000ULL), -1.0);
}

static void test_round_trip(void) {
	float    f_in   = 123.456f;
	uint32_t f_bits = ct_float32_to_bits(f_in);
	float    f_out  = ct_float32_from_bits(f_bits);
	assert_float_eq(f_in, f_out);

	double   d_in   = 9876.54321;
	uint64_t d_bits = ct_float64_to_bits(d_in);
	double   d_out  = ct_float64_from_bits(d_bits);
	assert_double_eq(d_in, d_out);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("Bit Cast Operations", NULL, NULL)
	CUNIT_TEST("Float32 Conversion", test_float32_conversion)
	CUNIT_TEST("Float64 Conversion", test_float64_conversion)
	CUNIT_TEST("Round Trip", test_round_trip)
	CUNIT_SUITE_END()

	return cunit_run();
}
