/**
 * @file random_test.c
 * @brief Comprehensive random number generator tests
 */
#include "coter/base/random.h"

#include "cunit.h"

#define BOUNDARY_TEST_ITERATIONS  1000
#define DISTRIBUTION_SAMPLE_COUNT 100000
#define DISTRIBUTION_TOLERANCE    0.2

/**
 * @brief Check if distribution is uniform within tolerance
 * @param counts Array of occurrence counts
 * @param size Size of the counts array
 * @param expected Expected count per bucket
 * @param tolerance Tolerance percentage (e.g., 0.2 for ±20%)
 * @return true if distribution is uniform, false otherwise
 */
static bool is_distribution_uniform(const uint64_t *counts, size_t size, uint64_t expected, double tolerance) {
	const uint64_t min_count = (uint64_t)(expected * (1.0 - tolerance));
	const uint64_t max_count = (uint64_t)(expected * (1.0 + tolerance));

	for (size_t i = 0; i < size; ++i) {
		if (counts[i] < min_count || counts[i] > max_count) {
			return false;
		}
	}
	return true;
}

static void test_null_safety(void) {
	/* All functions should handle NULL gracefully without crashing */
	assert_false(ct_random_bool(NULL));
	assert_uint8_eq(ct_random_uint8(NULL, 0, 100), 0);
	assert_int8_eq(ct_random_int8(NULL, -50, 50), 0);
	assert_uint16_eq(ct_random_uint16(NULL, 0, 1000), 0);
	assert_int16_eq(ct_random_int16(NULL, -500, 500), 0);
	assert_uint32_eq(ct_random_uint32(NULL, 0, 10000), 0);
	assert_int32_eq(ct_random_int32(NULL, -5000, 5000), 0);
	assert_uint64_eq(ct_random_uint64(NULL, 0, 100000), 0);
	assert_int64_eq(ct_random_int64(NULL, -50000, 50000), 0);
	assert_float_eq(ct_random_float(NULL, 0.0f, 1.0f), 0.0f);
	assert_double_eq(ct_random_double(NULL, 0.0, 1.0), 0.0);

	/* String function should not crash with NULL */
	char buffer[10];
	ct_random_string(NULL, buffer, 5);
	ct_random_string(NULL, NULL, 5);
}

static void test_edge_min_equals_max(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* When min == max, should always return min */
	for (int i = 0; i < 100; ++i) {
		assert_uint8_eq(ct_random_uint8(&rng, 42, 42), 42);
		assert_int8_eq(ct_random_int8(&rng, -7, -7), -7);
		assert_uint16_eq(ct_random_uint16(&rng, 1234, 1234), 1234);
		assert_int16_eq(ct_random_int16(&rng, -999, -999), -999);
		assert_uint32_eq(ct_random_uint32(&rng, 88888, 88888), 88888);
		assert_int32_eq(ct_random_int32(&rng, -77777, -77777), -77777);
		assert_uint64_eq(ct_random_uint64(&rng, 123456789, 123456789), 123456789);
		assert_int64_eq(ct_random_int64(&rng, -987654321, -987654321), -987654321);
	}
}

static void test_edge_min_greater_than_max(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* When min > max, should return min */
	for (int i = 0; i < 100; ++i) {
		assert_uint8_eq(ct_random_uint8(&rng, 100, 50), 100);
		assert_int8_eq(ct_random_int8(&rng, 50, -50), 50);
		assert_uint16_eq(ct_random_uint16(&rng, 5000, 1000), 5000);
		assert_int16_eq(ct_random_int16(&rng, 1000, -1000), 1000);
		assert_uint32_eq(ct_random_uint32(&rng, 100000, 50000), 100000);
		assert_int32_eq(ct_random_int32(&rng, 50000, -50000), 50000);
		assert_uint64_eq(ct_random_uint64(&rng, 1000000, 500000), 1000000);
		assert_int64_eq(ct_random_int64(&rng, 500000, -500000), 500000);
	}
}

static void test_boundary_uint8(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test full range [0, 255] */
	for (int i = 0; i < 1000; ++i) {
		uint8_t val = ct_random_uint8(&rng, 0, 255);
		assert_uint8_ge(val, 0);
		assert_uint8_le(val, 255);
	}

	/* Test boundary values */
	for (int i = 0; i < 1000; ++i) {
		uint8_t val = ct_random_uint8(&rng, 254, 255);
		assert_true(val == 254 || val == 255);
	}

	/* Test single value range */
	for (int i = 0; i < 100; ++i) {
		assert_uint8_eq(ct_random_uint8(&rng, 0, 1), 0);
	}
}

static void test_boundary_int8(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test full range [-128, 127] */
	for (int i = 0; i < 1000; ++i) {
		int8_t val = ct_random_int8(&rng, INT8_MIN, INT8_MAX);
		assert_int8_ge(val, INT8_MIN);
		assert_int8_le(val, INT8_MAX);
	}

	/* Test negative range */
	for (int i = 0; i < 1000; ++i) {
		int8_t val = ct_random_int8(&rng, -100, -50);
		assert_int8_ge(val, -100);
		assert_int8_lt(val, -50);
	}

	/* Test positive range */
	for (int i = 0; i < 1000; ++i) {
		int8_t val = ct_random_int8(&rng, 50, 100);
		assert_int8_ge(val, 50);
		assert_int8_lt(val, 100);
	}

	/* Test crossing zero */
	for (int i = 0; i < 1000; ++i) {
		int8_t val = ct_random_int8(&rng, -50, 50);
		assert_int8_ge(val, -50);
		assert_int8_lt(val, 50);
	}
}

static void test_boundary_uint16(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test full range [0, 65535] */
	for (int i = 0; i < 1000; ++i) {
		uint16_t val = ct_random_uint16(&rng, 0, 65535);
		assert_uint16_ge(val, 0);
		assert_uint16_le(val, 65535);
	}

	/* Test large values */
	for (int i = 0; i < 1000; ++i) {
		uint16_t val = ct_random_uint16(&rng, 60000, 65535);
		assert_uint16_ge(val, 60000);
		assert_uint16_le(val, 65535);
	}
}

static void test_boundary_int16(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test full range [-32768, 32767] */
	for (int i = 0; i < 1000; ++i) {
		int16_t val = ct_random_int16(&rng, INT16_MIN, INT16_MAX);
		assert_int16_ge(val, INT16_MIN);
		assert_int16_le(val, INT16_MAX);
	}

	/* Test extreme negative */
	for (int i = 0; i < 1000; ++i) {
		int16_t val = ct_random_int16(&rng, INT16_MIN, INT16_MIN + 1000);
		assert_int16_ge(val, INT16_MIN);
		assert_int16_lt(val, INT16_MIN + 1000);
	}

	/* Test extreme positive */
	for (int i = 0; i < 1000; ++i) {
		int16_t val = ct_random_int16(&rng, INT16_MAX - 1000, INT16_MAX);
		assert_int16_ge(val, INT16_MAX - 1000);
		assert_int16_le(val, INT16_MAX);
	}
}

static void test_boundary_uint32(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test large range */
	for (int i = 0; i < 1000; ++i) {
		uint32_t val = ct_random_uint32(&rng, 0, UINT32_MAX);
		assert_uint32_ge(val, 0);
		assert_uint32_le(val, UINT32_MAX);
	}

	/* Test high values */
	for (int i = 0; i < 1000; ++i) {
		uint32_t val = ct_random_uint32(&rng, UINT32_MAX - 10000, UINT32_MAX);
		assert_uint32_ge(val, UINT32_MAX - 10000);
		assert_uint32_le(val, UINT32_MAX);
	}
}

static void test_boundary_int32(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test full range */
	for (int i = 0; i < 1000; ++i) {
		int32_t val = ct_random_int32(&rng, INT32_MIN, INT32_MAX);
		assert_int32_ge(val, INT32_MIN);
		assert_int32_le(val, INT32_MAX);
	}

	/* Test extreme values */
	for (int i = 0; i < 1000; ++i) {
		int32_t val = ct_random_int32(&rng, INT32_MIN, INT32_MIN + 100000);
		assert_int32_ge(val, INT32_MIN);
		assert_int32_lt(val, INT32_MIN + 100000);
	}
}

static void test_boundary_uint64(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test large range */
	for (int i = 0; i < 1000; ++i) {
		uint64_t val = ct_random_uint64(&rng, 0, UINT64_MAX);
		assert_uint64_ge(val, 0);
		assert_uint64_le(val, UINT64_MAX);
	}

	/* Test very large values */
	for (int i = 0; i < 1000; ++i) {
		uint64_t val = ct_random_uint64(&rng, UINT64_MAX - 1000000, UINT64_MAX);
		assert_uint64_ge(val, UINT64_MAX - 1000000);
		assert_uint64_le(val, UINT64_MAX);
	}
}

static void test_boundary_int64(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test full range */
	for (int i = 0; i < 1000; ++i) {
		int64_t val = ct_random_int64(&rng, INT64_MIN, INT64_MAX);
		assert_int64_ge(val, INT64_MIN);
		assert_int64_le(val, INT64_MAX);
	}

	/* Test extreme negative */
	for (int i = 0; i < 1000; ++i) {
		int64_t val = ct_random_int64(&rng, INT64_MIN, INT64_MIN + 1000000);
		assert_int64_ge(val, INT64_MIN);
		assert_int64_lt(val, INT64_MIN + 1000000);
	}

	/* Test extreme positive */
	for (int i = 0; i < 1000; ++i) {
		int64_t val = ct_random_int64(&rng, INT64_MAX - 1000000, INT64_MAX);
		assert_int64_ge(val, INT64_MAX - 1000000);
		assert_int64_le(val, INT64_MAX);
	}
}

static void test_boundary_float(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test [0.0, 1.0] range */
	for (int i = 0; i < 1000; ++i) {
		float val = ct_random_float(&rng, 0.0f, 1.0f);
		assert_float_ge(val, 0.0f);
		assert_float_lt(val, 1.0f);
	}

	/* Test negative range */
	for (int i = 0; i < 1000; ++i) {
		float val = ct_random_float(&rng, -100.0f, -50.0f);
		assert_float_ge(val, -100.0f);
		assert_float_lt(val, -50.0f);
	}

	/* Test large range */
	for (int i = 0; i < 1000; ++i) {
		float val = ct_random_float(&rng, -1000.0f, 1000.0f);
		assert_float_ge(val, -1000.0f);
		assert_float_lt(val, 1000.0f);
	}
}

static void test_boundary_double(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test [0.0, 1.0] range */
	for (int i = 0; i < 1000; ++i) {
		double val = ct_random_double(&rng, 0.0, 1.0);
		assert_double_ge(val, 0.0);
		assert_double_lt(val, 1.0);
	}

	/* Test negative range */
	for (int i = 0; i < 1000; ++i) {
		double val = ct_random_double(&rng, -1000.0, -500.0);
		assert_double_ge(val, -1000.0);
		assert_double_lt(val, -500.0);
	}

	/* Test large range */
	for (int i = 0; i < 1000; ++i) {
		double val = ct_random_double(&rng, -10000.0, 10000.0);
		assert_double_ge(val, -10000.0);
		assert_double_lt(val, 10000.0);
	}
}

static void test_distribution_bool(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	uint64_t       counts[2]  = {0};
	const uint64_t iterations = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		bool val = ct_random_bool(&rng);
		assert_true(val == 0 || val == 1);
		counts[val]++;
	}

	/* Each value should appear ~50000 times, allow ±20% tolerance */
	const uint64_t expected = iterations / 2;
	assert_true(is_distribution_uniform(counts, 2, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_uint8(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const uint8_t  range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		uint8_t val = ct_random_uint8(&rng, 0, range_size);
		assert_uint8_lt(val, range_size);
		counts[val]++;
	}

	/* Each value should appear ~1000 times, allow ±20% tolerance */
	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_uint16(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const uint16_t range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		uint16_t val = ct_random_uint16(&rng, 0, range_size);
		assert_uint16_lt(val, range_size);
		counts[val]++;
	}

	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_uint32(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const uint32_t range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		uint32_t val = ct_random_uint32(&rng, 0, range_size);
		assert_uint32_lt(val, range_size);
		counts[val]++;
	}

	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_uint64(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const uint64_t range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		uint64_t val = ct_random_uint64(&rng, 0, range_size);
		assert_uint64_lt(val, range_size);
		counts[val]++;
	}

	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_int8(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const int8_t   min         = -50;
	const int8_t   max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		int8_t val = ct_random_int8(&rng, min, max);
		assert_int8_ge(val, min);
		assert_int8_lt(val, max);
		counts[val - min]++;
	}

	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_int16(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const int16_t  min         = -50;
	const int16_t  max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		int16_t val = ct_random_int16(&rng, min, max);
		assert_int16_ge(val, min);
		assert_int16_lt(val, max);
		counts[val - min]++;
	}

	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_int32(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const int32_t  min         = -50;
	const int32_t  max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		int32_t val = ct_random_int32(&rng, min, max);
		assert_int32_ge(val, min);
		assert_int32_lt(val, max);
		counts[val - min]++;
	}

	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_int64(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const int64_t  min         = -50;
	const int64_t  max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		int64_t val = ct_random_int64(&rng, min, max);
		assert_int64_ge(val, min);
		assert_int64_lt(val, max);
		counts[val - min]++;
	}

	const uint64_t expected = iterations / range_size;
	assert_true(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_float(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const size_t   buckets     = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		float val = ct_random_float(&rng, 0.0f, 1.0f);
		assert_float_ge(val, 0.0f);
		assert_float_lt(val, 1.0f);

		size_t bucket = (size_t)(val * buckets);
		if (bucket >= buckets)
			bucket = buckets - 1;
		counts[bucket]++;
	}

	const uint64_t expected = iterations / buckets;
	assert_true(is_distribution_uniform(counts, buckets, expected, DISTRIBUTION_TOLERANCE));
}

static void test_distribution_double(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	const size_t   buckets     = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;

	for (uint64_t i = 0; i < iterations; ++i) {
		double val = ct_random_double(&rng, 0.0, 1.0);
		assert_double_ge(val, 0.0);
		assert_double_lt(val, 1.0);

		size_t bucket = (size_t)(val * buckets);
		if (bucket >= buckets)
			bucket = buckets - 1;
		counts[bucket]++;
	}

	const uint64_t expected = iterations / buckets;
	assert_true(is_distribution_uniform(counts, buckets, expected, DISTRIBUTION_TOLERANCE));
}

static void test_string_generation(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Test various lengths */
	const size_t lengths[] = {0, 1, 5, 10, 32, 64, 128};
	for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i) {
		char buffer[256] = {0};
		ct_random_string(&rng, buffer, lengths[i]);

		/* Check null terminator */
		assert_uint8_eq(buffer[lengths[i]], '\0');

		/* Check all characters are valid */
		for (size_t j = 0; j < lengths[i]; j++) {
			char c        = buffer[j];
			bool is_valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
			assert_true(is_valid);
		}
	}
}

static void test_string_uniqueness(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	/* Generate multiple strings and check they're different */
	char str1[33], str2[33], str3[33];
	ct_random_string(&rng, str1, 32);
	ct_random_string(&rng, str2, 32);
	ct_random_string(&rng, str3, 32);

	/* Statistically, these should be different */
	bool all_same = (strcmp(str1, str2) == 0 && strcmp(str2, str3) == 0);
	assert_false(all_same);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("Safety and Edge Cases", NULL, NULL)
	CUNIT_TEST("Ensures all functions handle NULL pointers gracefully", test_null_safety)
	CUNIT_TEST("Ensures min == max returns min", test_edge_min_equals_max)
	CUNIT_TEST("Ensures min > max returns min", test_edge_min_greater_than_max)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Boundary Values", NULL, NULL)
	CUNIT_TEST("Ensures uint8 boundary values [0, 255]", test_boundary_uint8)
	CUNIT_TEST("Ensures int8 boundary values [-128, 127]", test_boundary_int8)
	CUNIT_TEST("Ensures uint16 boundary values [0, 65535]", test_boundary_uint16)
	CUNIT_TEST("Ensures int16 boundary values [-32768, 32767]", test_boundary_int16)
	CUNIT_TEST("Ensures uint32 boundary values near UINT32_MAX", test_boundary_uint32)
	CUNIT_TEST("Ensures int32 boundary values near INT32_MIN/MAX", test_boundary_int32)
	CUNIT_TEST("Ensures uint64 boundary values near UINT64_MAX", test_boundary_uint64)
	CUNIT_TEST("Ensures int64 boundary values near INT64_MIN/MAX", test_boundary_int64)
	CUNIT_TEST("Ensures float boundary values in various ranges", test_boundary_float)
	CUNIT_TEST("Ensures double boundary values in various ranges", test_boundary_double)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Distribution Uniformity", NULL, NULL)
	CUNIT_TEST("Verifies bool distribution uniformity", test_distribution_bool)
	CUNIT_TEST("Verifies uint8 distribution uniformity", test_distribution_uint8)
	CUNIT_TEST("Verifies uint16 distribution uniformity", test_distribution_uint16)
	CUNIT_TEST("Verifies uint32 distribution uniformity", test_distribution_uint32)
	CUNIT_TEST("Verifies uint64 distribution uniformity", test_distribution_uint64)
	CUNIT_TEST("Verifies int8 distribution uniformity", test_distribution_int8)
	CUNIT_TEST("Verifies int16 distribution uniformity", test_distribution_int16)
	CUNIT_TEST("Verifies int32 distribution uniformity", test_distribution_int32)
	CUNIT_TEST("Verifies int64 distribution uniformity", test_distribution_int64)
	CUNIT_TEST("Verifies float distribution uniformity", test_distribution_float)
	CUNIT_TEST("Verifies double distribution uniformity", test_distribution_double)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("String Generation", NULL, NULL)
	CUNIT_TEST("Validates string generation with various lengths", test_string_generation)
	CUNIT_TEST("Ensures generated strings are statistically unique", test_string_uniqueness)
	CUNIT_SUITE_END()

	return cunit_run();
}
