/**
 * @file random_test.c
 * @brief 随机数测试
 */
#include "coter/base/random.h"

#include "coter/base/platform.h"
#include "cunit.h"

static inline void test_random_bool(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_t rng;
	ct_random_init(&rng);

	// 定义统计数组，用于记录每个随机数的出现次数
	int count[2] = {0};

	bool random_num = 0;
	for (register uint64_t i = 0; i < 2 * test_number;) {
		for (uint64_t n = 0; n < test_number; n++, i++) {
			random_num = ct_random_bool(&rng);
			assert_uint8_lt(random_num, 2);
			count[random_num]++;
		}
		sched_yield();
	}

	// 检查随机数的分布是否均匀
	bool is_ok = true;
	for (int i = 0; i < 2; i++) {
		if (count[i] < (int)(test_number * 0.8) || count[i] > (int)(test_number * 1.2)) {
			is_ok = false;
			break;
		}
	}

	assert_true(is_ok);
}

static inline void test_random_uint8(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_t rng;
	ct_random_init(&rng);

	// 定义统计数组,用于记录每个随机数的出现次数
	int count[100] = {0};

	uint8_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint8(&rng, 0, 100);
			assert_uint8_lt(random_num, 100);
			count[random_num]++;
		}
		sched_yield();
	}

	// 检查随机数的分布是否均匀
	bool is_ok = true;
	for (int i = 0; i < 100; i++) {
		if (count[i] < (int)(test_number * 0.8) || count[i] > (int)(test_number * 1.2)) {
			is_ok = false;
			break;
		}
	}

	assert_true(is_ok);
}

static inline void test_random_uint16(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_t rng;
	ct_random_init(&rng);

	// 定义统计数组,用于记录每个随机数的出现次数
	int count[100] = {0};

	uint16_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint16(&rng, 0, 100);
			assert_uint16_lt(random_num, 100);
			count[random_num]++;
		}
	}

	// 检查随机数的分布是否均匀
	bool is_ok = true;
	for (int i = 0; i < 100; i++) {
		if (count[i] < (int)(test_number * 0.8) || count[i] > (int)(test_number * 1.2)) {
			is_ok = false;
			break;
		}
	}

	assert_true(is_ok);
}

static inline void test_random_uint32(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_t rng;
	ct_random_init(&rng);

	// 定义统计数组,用于记录每个随机数的出现次数
	int count[100] = {0};

	uint32_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint32(&rng, 0, 100);
			assert_uint32_lt(random_num, 100);
			count[random_num]++;
		}
		sched_yield();
	}

	// 检查随机数的分布是否均匀
	bool is_ok = true;
	for (int i = 0; i < 100; i++) {
		if (count[i] < (int)(test_number * 0.8) || count[i] > (int)(test_number * 1.2)) {
			is_ok = false;
			break;
		}
	}

	assert_true(is_ok);
}

static inline void test_random_uint64(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_t rng;
	ct_random_init(&rng);

	// 定义统计数组,用于记录每个随机数的出现次数
	uint64_t count[100] = {0};

	uint64_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint64(&rng, 0, 100);
			assert_uint64_lt(random_num, 100);
			count[random_num]++;
		}
		sched_yield();
	}

	// 检查随机数的分布是否均匀
	bool is_ok = true;
	for (int i = 0; i < 100; i++) {
		if (count[i] < (int)(test_number * 0.8) || count[i] > (int)(test_number * 1.2)) {
			is_ok = false;
			break;
		}
	}

	assert_true(is_ok);
}

static inline void test_random_float(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_t rng;
	ct_random_init(&rng);

	// 定义统计数组,用于记录每个随机数落在的区间
	int count[100] = {0};

	int random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = (int)(ct_random_float(&rng, 0.0f, 1.0f) * 100);
			count[random_num]++;
		}
		sched_yield();
	}

	// 检查随机数的分布是否均匀
	bool is_ok = true;
	for (int i = 0; i < 100; i++) {
		if (count[i] < (int)(test_number * 0.8) || count[i] > (int)(test_number * 1.2)) {
			is_ok = false;
			break;
		}
	}

	assert_true(is_ok);
}

static inline void test_random_double(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_t rng;
	ct_random_init(&rng);

	// 定义统计数组,用于记录每个随机数落在的区间
	int count[100] = {0};

	int random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = (int)(ct_random_double(&rng, 0.0, 1.0) * 100);
			count[random_num]++;
		}
		sched_yield();
	}

	// 检查随机数的分布是否均匀
	bool is_ok = true;
	for (int i = 0; i < 100; i++) {
		if (count[i] < (int)(test_number * 0.8) || count[i] > (int)(test_number * 1.2)) {
			is_ok = false;
			break;
		}
	}

	assert_true(is_ok);
}

void test_random_bool_wrapper(void) {
	test_random_bool(10000);
}

void test_random_uint8_wrapper(void) {
	test_random_uint8(10000);
}

void test_random_uint16_wrapper(void) {
	test_random_uint16(10000);
}

void test_random_uint32_wrapper(void) {
	test_random_uint32(10000);
}

void test_random_uint64_wrapper(void) {
	test_random_uint64(10000);
}

void test_random_float_wrapper(void) {
	test_random_float(10000);
}

void test_random_double_wrapper(void) {
	test_random_double(10000);
}

// Boundary tests for signed integers
static inline void test_random_int8_boundary(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	// Test full range
	for (int i = 0; i < 1000; i++) {
		int8_t val = ct_random_int8(&rng, INT8_MIN, INT8_MAX);
		assert_true(val >= INT8_MIN && val <= INT8_MAX);
	}
}

static inline void test_random_int16_boundary(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	// Test full range
	for (int i = 0; i < 1000; i++) {
		int16_t val = ct_random_int16(&rng, INT16_MIN, INT16_MAX);
		assert_true(val >= INT16_MIN && val <= INT16_MAX);
	}
}

static inline void test_random_int32_boundary(void) {
	ct_random_t rng;
	ct_random_init(&rng);

	// Test full range (this previously caused overflow)
	for (int i = 0; i < 1000; i++) {
		int32_t val = ct_random_int32(&rng, INT32_MIN, INT32_MAX);
		assert_true(val >= INT32_MIN && val <= INT32_MAX);
	}
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("random", NULL, NULL)
	CUNIT_TEST("bool", test_random_bool_wrapper)
	CUNIT_TEST("uint8", test_random_uint8_wrapper)
	CUNIT_TEST("uint16", test_random_uint16_wrapper)
	CUNIT_TEST("uint32", test_random_uint32_wrapper)
	CUNIT_TEST("uint64", test_random_uint64_wrapper)
	CUNIT_TEST("float", test_random_float_wrapper)
	CUNIT_TEST("double", test_random_double_wrapper)
	CUNIT_TEST("int8_boundary", test_random_int8_boundary)
	CUNIT_TEST("int16_boundary", test_random_int16_boundary)
	CUNIT_TEST("int32_boundary", test_random_int32_boundary)
	CUNIT_SUITE_END()

	return cunit_run();
}
