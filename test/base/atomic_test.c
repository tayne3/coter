/**
 * @file atomic_test.c
 * @brief 原子操作相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "base/ct_atomic.h"
#include "ctunit.h"

// 定义线程数量
#define NUM_THREADS (8 << 1)
// 定义每个线程的迭代次数
#define NUM_ITERATIONS 100000

// 测试原子递增和递减的计数器
ct_atomic_t test_counter   = CT_ATOMIC_VAR_INIT(0);
ct_atomic_t shared_counter = CT_ATOMIC_VAR_INIT(0);
ct_atomic_t test_value     = CT_ATOMIC_VAR_INIT(0);

static inline void* increment_thread(void* arg) {
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		CT_ATOMIC_INC(&test_counter);
		CT_ATOMIC_INC(&test_counter);
	}
	return NULL;
	(void)arg;
}

static inline void* decrement_thread(void* arg) {
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		CT_ATOMIC_DEC(&test_counter);
		CT_ATOMIC_DEC(&test_counter);
	}
	return NULL;
	(void)arg;
}

static inline void* atomic_inc_thread(void* arg) {
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		CT_ATOMIC_ADD(&test_value, 1);
		CT_ATOMIC_SUB(&test_value, 3);
		CT_ATOMIC_ADD(&test_value, 5);
	}
	return NULL;
	(void)arg;
}

static inline void* atomic_dec_thread(void* arg) {
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		CT_ATOMIC_SUB(&test_value, 6);
		CT_ATOMIC_ADD(&test_value, 7);
		CT_ATOMIC_SUB(&test_value, 4);
	}
	return NULL;
	(void)arg;
}

// 测试案例: 原子标志
static inline void test_atomic_flag(void) {
	ct_atomic_flag_t flag = CT_ATOMIC_FLAG_INIT;

	ctunit_assert_false(ct_atomic_flag_test_and_set(&flag));
	ctunit_assert_true(ct_atomic_flag_test_and_set(&flag));
	ctunit_assert_true(ct_atomic_flag_test_and_set(&flag));
	ctunit_assert_true(ct_atomic_flag_test_and_set(&flag));

	CT_ATOMIC_FLAG_CLEAR(&flag);
	ctunit_assert_false(ct_atomic_flag_test_and_set(&flag));
	ctunit_assert_true(ct_atomic_flag_test_and_set(&flag));
	ctunit_assert_true(ct_atomic_flag_test_and_set(&flag));
	ctunit_assert_true(ct_atomic_flag_test_and_set(&flag));
}

// 测试案例: 原子递增和递减
static inline void test_atomic_inc_dec(void) {
	bool      is_ok = false;
	pthread_t threads[NUM_THREADS];

	CT_ATOMIC_STORE(&test_counter, 0);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_counter), 0);

	for (int i = 0; i < NUM_THREADS >> 1; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, increment_thread, NULL);
		ctunit_assert_true(is_ok);
	}
	for (int i = NUM_THREADS >> 1; i < NUM_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, decrement_thread, NULL);
		ctunit_assert_true(is_ok);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		CT_ATOMIC_INC(&test_counter);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		ctunit_assert_true(is_ok);
	}

	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_counter), NUM_THREADS);
}

// 测试案例: 原子加法和减法
static inline void test_atomic_add_sub(void) {
	test_value = CT_ATOMIC_VAR_INIT(0);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 0);

	CT_ATOMIC_ADD(&test_value, 10);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 10);

	CT_ATOMIC_SUB(&test_value, 5);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 5);

	// Test with large numbers
	CT_ATOMIC_ADD(&test_value, 1000000);
	CT_ATOMIC_SUB(&test_value, 1000000);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 5);

	// 测试负数
	CT_ATOMIC_ADD(&test_value, -15);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), -10);

	CT_ATOMIC_SUB(&test_value, -20);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 10);
}

// 测试案例: 原子加载和存储操作
static inline void test_atomic_load_store(void) {
	test_value = CT_ATOMIC_VAR_INIT(42);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 42);

	CT_ATOMIC_STORE(&test_value, 100);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 100);

	// 测试边界值
	CT_ATOMIC_STORE(&test_value, LONG_MAX);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), LONG_MAX);

	CT_ATOMIC_STORE(&test_value, LONG_MIN);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), LONG_MIN);
}

// 测试案例: 多线程环境下的原子操作
static inline void test_atomic_operations_multi_threaded(void) {
	bool      is_ok = false;
	pthread_t threads[NUM_THREADS];

	CT_ATOMIC_STORE(&test_value, 0);
	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), 0);

	for (int i = 0; i < NUM_THREADS >> 1; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, atomic_inc_thread, NULL);
		ctunit_assert_true(is_ok);
	}
	for (int i = NUM_THREADS >> 1; i < NUM_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, atomic_dec_thread, NULL);
		ctunit_assert_true(is_ok);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		CT_ATOMIC_ADD(&test_value, 1);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		ctunit_assert_true(is_ok);
	}

	ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&test_value), NUM_THREADS);
}

// 测试案例: 边界条件
static inline void test_boundary_conditions(void) {
	// 测试溢出 (LONG_MAX + 1 = LONG_MIN)
	{
		ct_atomic_t max_value = CT_ATOMIC_VAR_INIT(LONG_MAX);
		CT_ATOMIC_ADD(&max_value, 1);
		ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&max_value), LONG_MIN);
	}

	// 测试下溢 (LONG_MIN - 1 = LONG_MAX)
	{
		ct_atomic_t min_value = CT_ATOMIC_VAR_INIT(LONG_MIN);
		CT_ATOMIC_SUB(&min_value, 1);
		ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&min_value), LONG_MAX);
	}

	// 测试最大值加法 (LONG_MAX + LONG_MAX = 2^63 - 1 + 2^63 - 1 = 2^64 - 2)
	{
		ct_atomic_t max_value = CT_ATOMIC_VAR_INIT(LONG_MAX);
		CT_ATOMIC_ADD(&max_value, LONG_MAX);
		ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&max_value), -2);
	}

	// 测试最小值减法 (LONG_MIN - LONG_MAX = -2^63 - (2^63 - 1) = -2^64 + 1)
	{
		ct_atomic_t min_value = CT_ATOMIC_VAR_INIT(LONG_MIN);
		CT_ATOMIC_SUB(&min_value, LONG_MAX);
		ctunit_assert_int64_equal(CT_ATOMIC_LOAD(&min_value), 1);
	}
}

int main(void) {
	test_atomic_flag();
	ctunit_trace("Finish! test_atomic_flag();\n");

	test_atomic_inc_dec();
	ctunit_trace("Finish! test_atomic_inc_dec();\n");

	test_atomic_add_sub();
	ctunit_trace("Finish! test_atomic_add_sub();\n");

	test_atomic_load_store();
	ctunit_trace("Finish! test_atomic_load_store();\n");

	test_atomic_operations_multi_threaded();
	ctunit_trace("Finish! test_atomic_operations_multi_threaded();\n");

	test_boundary_conditions();
	ctunit_trace("Finish! test_boundary_conditions();\n");

	ctunit_pass();
}
