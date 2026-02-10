#include "coter/math/rand.h"

#include <catch.hpp>
#include <cstring>

#define BOUNDARY_TEST_ITERATIONS  1000
#define DISTRIBUTION_SAMPLE_COUNT 100000
#define DISTRIBUTION_TOLERANCE    0.2

static bool is_distribution_uniform(const uint64_t *counts, size_t size, uint64_t expected, double tolerance) {
	const uint64_t min_count = (uint64_t)(expected * (1.0 - tolerance));
	const uint64_t max_count = (uint64_t)(expected * (1.0 + tolerance));
	for (size_t i = 0; i < size; ++i) {
		if (counts[i] < min_count || counts[i] > max_count) { return false; }
	}
	return true;
}

TEST_CASE("rand_null_safety", "[rand]") {
	REQUIRE_FALSE(ct_random_bool(nullptr));
	REQUIRE(ct_random_uint8(nullptr, 0, 100) == 0);
	REQUIRE(ct_random_int8(nullptr, -50, 50) == 0);
	REQUIRE(ct_random_uint16(nullptr, 0, 1000) == 0);
	REQUIRE(ct_random_int16(nullptr, -500, 500) == 0);
	REQUIRE(ct_random_uint32(nullptr, 0, 10000) == 0);
	REQUIRE(ct_random_int32(nullptr, -5000, 5000) == 0);
	REQUIRE(ct_random_uint64(nullptr, 0, 100000) == 0);
	REQUIRE(ct_random_int64(nullptr, -50000, 50000) == 0);
	REQUIRE(ct_random_float(nullptr, 0.0f, 1.0f) == 0.0f);
	REQUIRE(ct_random_double(nullptr, 0.0, 1.0) == 0.0);
	char buffer[10];
	ct_random_string(nullptr, buffer, 5);
	ct_random_string(nullptr, nullptr, 5);
}

TEST_CASE("rand_edge_min_equals_max", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < 100; ++i) {
		REQUIRE(ct_random_uint8(&rng, 42, 42) == 42);
		REQUIRE(ct_random_int8(&rng, -7, -7) == -7);
		REQUIRE(ct_random_uint16(&rng, 1234, 1234) == 1234);
		REQUIRE(ct_random_int16(&rng, -999, -999) == -999);
		REQUIRE(ct_random_uint32(&rng, 88888, 88888) == 88888);
		REQUIRE(ct_random_int32(&rng, -77777, -77777) == -77777);
		REQUIRE(ct_random_uint64(&rng, 123456789, 123456789) == 123456789);
		REQUIRE(ct_random_int64(&rng, -987654321, -987654321) == -987654321);
	}
}

TEST_CASE("rand_edge_min_greater_than_max", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < 100; ++i) {
		REQUIRE(ct_random_uint8(&rng, 100, 50) == 100);
		REQUIRE(ct_random_int8(&rng, 50, -50) == 50);
		REQUIRE(ct_random_uint16(&rng, 5000, 1000) == 5000);
		REQUIRE(ct_random_int16(&rng, 1000, -1000) == 1000);
		REQUIRE(ct_random_uint32(&rng, 100000, 50000) == 100000);
		REQUIRE(ct_random_int32(&rng, 50000, -50000) == 50000);
		REQUIRE(ct_random_uint64(&rng, 1000000, 500000) == 1000000);
		REQUIRE(ct_random_int64(&rng, 500000, -500000) == 500000);
	}
}

TEST_CASE("rand_boundary_uint8", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint8_t val = ct_random_uint8(&rng, 0, 255);
		REQUIRE(val >= 0);
		REQUIRE(val <= 255);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint8_t val = ct_random_uint8(&rng, 254, 255);
		REQUIRE(((int)val - 254) * ((int)val - 255) == 0);
	}
	for (int i = 0; i < 100; ++i) { REQUIRE(ct_random_uint8(&rng, 0, 1) == 0); }
}

TEST_CASE("rand_boundary_int8", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int8_t val = ct_random_int8(&rng, INT8_MIN, INT8_MAX);
		REQUIRE(val >= INT8_MIN);
		REQUIRE(val <= INT8_MAX);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int8_t val = ct_random_int8(&rng, -100, -50);
		REQUIRE(val >= -100);
		REQUIRE(val < -50);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int8_t val = ct_random_int8(&rng, 50, 100);
		REQUIRE(val >= 50);
		REQUIRE(val < 100);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int8_t val = ct_random_int8(&rng, -50, 50);
		REQUIRE(val >= -50);
		REQUIRE(val < 50);
	}
}

TEST_CASE("rand_boundary_uint16", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint16_t val = ct_random_uint16(&rng, 0, 65535);
		REQUIRE(val >= 0);
		REQUIRE(val <= 65535);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint16_t val = ct_random_uint16(&rng, 60000, 65535);
		REQUIRE(val >= 60000);
		REQUIRE(val <= 65535);
	}
}

TEST_CASE("rand_boundary_int16", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int16_t val = ct_random_int16(&rng, INT16_MIN, INT16_MAX);
		REQUIRE(val >= INT16_MIN);
		REQUIRE(val <= INT16_MAX);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int16_t val = ct_random_int16(&rng, INT16_MIN, INT16_MIN + 1000);
		REQUIRE(val >= INT16_MIN);
		REQUIRE(val < INT16_MIN + 1000);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int16_t val = ct_random_int16(&rng, INT16_MAX - 1000, INT16_MAX);
		REQUIRE(val >= INT16_MAX - 1000);
		REQUIRE(val <= INT16_MAX);
	}
}

TEST_CASE("rand_boundary_uint32", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint32_t val = ct_random_uint32(&rng, 0, UINT32_MAX);
		REQUIRE(val >= 0);
		REQUIRE(val <= UINT32_MAX);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint32_t val = ct_random_uint32(&rng, UINT32_MAX - 10000, UINT32_MAX);
		REQUIRE(val >= UINT32_MAX - 10000);
		REQUIRE(val <= UINT32_MAX);
	}
}

TEST_CASE("rand_boundary_int32", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int32_t val = ct_random_int32(&rng, INT32_MIN, INT32_MAX);
		REQUIRE(val >= INT32_MIN);
		REQUIRE(val <= INT32_MAX);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int32_t val = ct_random_int32(&rng, INT32_MIN, INT32_MIN + 100000);
		REQUIRE(val >= INT32_MIN);
		REQUIRE(val < INT32_MIN + 100000);
	}
}

TEST_CASE("rand_boundary_uint64", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint64_t val = ct_random_uint64(&rng, 0, UINT64_MAX);
		REQUIRE(val >= 0);
		REQUIRE(val <= UINT64_MAX);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		uint64_t val = ct_random_uint64(&rng, UINT64_MAX - 1000000, UINT64_MAX);
		REQUIRE(val >= UINT64_MAX - 1000000);
		REQUIRE(val <= UINT64_MAX);
	}
}

TEST_CASE("rand_boundary_int64", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int64_t val = ct_random_int64(&rng, INT64_MIN, INT64_MAX);
		REQUIRE(val >= INT64_MIN);
		REQUIRE(val <= INT64_MAX);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int64_t val = ct_random_int64(&rng, INT64_MIN, INT64_MIN + 1000000);
		REQUIRE(val >= INT64_MIN);
		REQUIRE(val < INT64_MIN + 1000000);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		int64_t val = ct_random_int64(&rng, INT64_MAX - 1000000, INT64_MAX);
		REQUIRE(val >= INT64_MAX - 1000000);
		REQUIRE(val <= INT64_MAX);
	}
}

TEST_CASE("rand_boundary_float", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		float val = ct_random_float(&rng, 0.0f, 1.0f);
		REQUIRE(val >= 0.0f);
		REQUIRE(val < 1.0f);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		float val = ct_random_float(&rng, -100.0f, -50.0f);
		REQUIRE(val >= -100.0f);
		REQUIRE(val < -50.0f);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		float val = ct_random_float(&rng, -1000.0f, 1000.0f);
		REQUIRE(val >= -1000.0f);
		REQUIRE(val < 1000.0f);
	}
}

TEST_CASE("rand_boundary_double", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		double val = ct_random_double(&rng, 0.0, 1.0);
		REQUIRE(val >= 0.0);
		REQUIRE(val < 1.0);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		double val = ct_random_double(&rng, -1000.0, -500.0);
		REQUIRE(val >= -1000.0);
		REQUIRE(val < -500.0);
	}
	for (int i = 0; i < BOUNDARY_TEST_ITERATIONS; ++i) {
		double val = ct_random_double(&rng, -10000.0, 10000.0);
		REQUIRE(val >= -10000.0);
		REQUIRE(val < 10000.0);
	}
}

TEST_CASE("rand_distribution_bool", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	uint64_t       counts[2]  = {0};
	const uint64_t iterations = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		bool val = ct_random_bool(&rng);
		REQUIRE((val == 0) != (val == 1));
		counts[val]++;
	}
	const uint64_t expected = iterations / 2;
	REQUIRE(is_distribution_uniform(counts, 2, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_uint8", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const uint8_t  range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		uint8_t val = ct_random_uint8(&rng, 0, range_size);
		REQUIRE(val < range_size);
		counts[val]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_uint16", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const uint16_t range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		uint16_t val = ct_random_uint16(&rng, 0, range_size);
		REQUIRE(val < range_size);
		counts[val]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_uint32", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const uint32_t range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		uint32_t val = ct_random_uint32(&rng, 0, range_size);
		REQUIRE(val < range_size);
		counts[val]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_uint64", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const uint64_t range_size  = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		uint64_t val = ct_random_uint64(&rng, 0, range_size);
		REQUIRE(val < range_size);
		counts[val]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_int8", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const int8_t   min         = -50;
	const int8_t   max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		int8_t val = ct_random_int8(&rng, min, max);
		REQUIRE(val >= min);
		REQUIRE(val < max);
		counts[val - min]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_int16", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const int16_t  min         = -50;
	const int16_t  max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		int16_t val = ct_random_int16(&rng, min, max);
		REQUIRE(val >= min);
		REQUIRE(val < max);
		counts[val - min]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_int32", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const int32_t  min         = -50;
	const int32_t  max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		int32_t val = ct_random_int32(&rng, min, max);
		REQUIRE(val >= min);
		REQUIRE(val < max);
		counts[val - min]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_int64", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const int64_t  min         = -50;
	const int64_t  max         = 50;
	const size_t   range_size  = max - min;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		int64_t val = ct_random_int64(&rng, min, max);
		REQUIRE(val >= min);
		REQUIRE(val < max);
		counts[val - min]++;
	}
	const uint64_t expected = iterations / range_size;
	REQUIRE(is_distribution_uniform(counts, range_size, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_float", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const size_t   buckets     = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		float val = ct_random_float(&rng, 0.0f, 1.0f);
		REQUIRE(val >= 0.0f);
		REQUIRE(val < 1.0f);
		size_t bucket = (size_t)(val * buckets);
		if (bucket >= buckets) bucket = buckets - 1;
		counts[bucket]++;
	}
	const uint64_t expected = iterations / buckets;
	REQUIRE(is_distribution_uniform(counts, buckets, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_distribution_double", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const size_t   buckets     = 100;
	uint64_t       counts[100] = {0};
	const uint64_t iterations  = DISTRIBUTION_SAMPLE_COUNT;
	for (uint64_t i = 0; i < iterations; ++i) {
		double val = ct_random_double(&rng, 0.0, 1.0);
		REQUIRE(val >= 0.0);
		REQUIRE(val < 1.0);
		size_t bucket = (size_t)(val * buckets);
		if (bucket >= buckets) bucket = buckets - 1;
		counts[bucket]++;
	}
	const uint64_t expected = iterations / buckets;
	REQUIRE(is_distribution_uniform(counts, buckets, expected, DISTRIBUTION_TOLERANCE));
}

TEST_CASE("rand_string_generation", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	const size_t lengths[] = {0, 1, 5, 10, 32, 64, 128};
	for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i) {
		char buffer[256] = {0};
		ct_random_string(&rng, buffer, lengths[i]);
		REQUIRE(buffer[lengths[i]] == '\0');
		for (size_t j = 0; j < lengths[i]; ++j) {
			char c        = buffer[j];
			bool is_valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
			REQUIRE(is_valid);
		}
	}
}

TEST_CASE("rand_string_uniqueness", "[rand]") {
	ct_random_t rng;
	ct_random_init(&rng);
	char str1[33], str2[33], str3[33];
	ct_random_string(&rng, str1, 32);
	ct_random_string(&rng, str2, 32);
	ct_random_string(&rng, str3, 32);
	bool all_same = (std::strcmp(str1, str2) == 0 && std::strcmp(str2, str3) == 0);
	REQUIRE_FALSE(all_same);
}
