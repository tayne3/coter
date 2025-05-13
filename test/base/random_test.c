/**
 * @file random_test.c
 * @brief 随机数测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "base/ct_platform.h"
#include "base/ct_random.h"
#include "ctunit.h"

static inline void test_random_bool(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_buf_t state;
	ct_random_init(state);

	// 定义统计数组，用于记录每个随机数的出现次数
	int count[2] = {0};

	bool random_num = 0;
	for (register uint64_t i = 0; i < 2 * test_number;) {
		for (uint64_t n = 0; n < test_number; n++, i++) {
			random_num = ct_random_bool(state);
			ctunit_assert_uint8_less(random_num, 2);
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

	ctunit_assert_true(is_ok);
}

static inline void test_random_uint8(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_buf_t state;
	ct_random_init(state);

	// 定义统计数组,用于记录每个随机数的出现次数
	int count[100] = {0};

	uint8_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint8(state, 0, 100);
			ctunit_assert_uint8_less(random_num, 100);
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

	ctunit_assert_true(is_ok);
}

static inline void test_random_uint16(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_buf_t state;
	ct_random_init(state);

	// 定义统计数组,用于记录每个随机数的出现次数
	int count[100] = {0};

	uint16_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint16(state, 0, 100);
			ctunit_assert_uint16_less(random_num, 100);
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

	ctunit_assert_true(is_ok);
}

static inline void test_random_uint32(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_buf_t state;
	ct_random_init(state);

	// 定义统计数组,用于记录每个随机数的出现次数
	int count[100] = {0};

	uint32_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint32(state, 0, 100);
			ctunit_assert_uint32_less(random_num, 100);
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

	ctunit_assert_true(is_ok);
}

static inline void test_random_uint64(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_buf_t state;
	ct_random_init(state);

	// 定义统计数组,用于记录每个随机数的出现次数
	uint64_t count[100] = {0};

	uint64_t random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = ct_random_uint64(state, 0, 100);
			ctunit_assert_uint64_less(random_num, 100);
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

	ctunit_assert_true(is_ok);
}

static inline void test_random_float(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_buf_t state;
	ct_random_init(state);

	// 定义统计数组,用于记录每个随机数落在的区间
	int count[100] = {0};

	int random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = (int)(ct_random_float(state, 0.0f, 1.0f) * 100);
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

	ctunit_assert_true(is_ok);
}

static inline void test_random_double(uint64_t test_number) {
	// 初始化随机数生成器
	ct_random_buf_t state;
	ct_random_init(state);

	// 定义统计数组,用于记录每个随机数落在的区间
	int count[100] = {0};

	int random_num = 0;
	for (register uint64_t i = 0; i < 100 * test_number;) {
		for (uint64_t n = 0; n < 50 * test_number; n++, i++) {
			random_num = (int)(ct_random_double(state, 0.0, 1.0) * 100);
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

	ctunit_assert_true(is_ok);
}

int main(void) {
	test_random_bool(10000);
	ctunit_trace("Finish! test_random_bool(10000);\n");

	test_random_uint8(10000);
	ctunit_trace("Finish! test_random_uint8(10000);\n");

	test_random_uint16(10000);
	ctunit_trace("Finish! test_random_uint16(10000);\n");

	test_random_uint32(10000);
	ctunit_trace("Finish! test_random_uint32(10000);\n");

	test_random_uint64(10000);
	ctunit_trace("Finish! test_random_uint64(10000);\n");

	test_random_float(10000);
	ctunit_trace("Finish! test_random_float(10000);\n");

	test_random_double(10000);
	ctunit_trace("Finish! test_random_double(10000);\n");

	ctunit_pass();
}
