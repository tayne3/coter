/**
 * @file atomic_test.c
 * @brief 原子操作测试
 */
#include "coter/base/atomic.h"

#include "cunit.h"

#define NUM_THREADS    16
#define NUM_ITERATIONS 100000

// 全局共享计数器
static ct_atomic_t g_shared_counter;

static void* thread_increment_routine(void* arg) {
	ct_unused(arg);
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		CT_ATOMIC_INC(&g_shared_counter);
	}
	return NULL;
}

static void* thread_decrement_routine(void* arg) {
	ct_unused(arg);
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		CT_ATOMIC_DEC(&g_shared_counter);
	}
	return NULL;
}

static void test_flag(void) {
	ct_atomic_flag_t flag = CT_ATOMIC_FLAG_INIT;

	// 第一次 test-and-set 应该返回 false (前一个状态)
	assert_false(ct_atomic_flag_test_and_set(&flag));
	// 后续调用应该返回 true
	assert_true(ct_atomic_flag_test_and_set(&flag));
	assert_true(ct_atomic_flag_test_and_set(&flag));

	// 清除标志
	CT_ATOMIC_FLAG_CLEAR(&flag);

	// test-and-set 应该再次返回 false
	assert_false(ct_atomic_flag_test_and_set(&flag));
	assert_true(ct_atomic_flag_test_and_set(&flag));
}

static void test_load_store(void) {
	ct_atomic_t val = CT_ATOMIC_VAR_INIT(42);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 42);

	CT_ATOMIC_STORE(&val, 100);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 100);

	// 测试边界值
	CT_ATOMIC_STORE(&val, LONG_MAX);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), LONG_MAX);

	CT_ATOMIC_STORE(&val, LONG_MIN);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), LONG_MIN);
}

static void test_add_sub(void) {
	ct_atomic_t val = CT_ATOMIC_VAR_INIT(0);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 0);

	CT_ATOMIC_ADD(&val, 10);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 10);

	CT_ATOMIC_SUB(&val, 5);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 5);

	// 测试负数
	CT_ATOMIC_ADD(&val, -15);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), -10);

	CT_ATOMIC_SUB(&val, -20);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 10);
}

static void test_return_value(void) {
	ct_atomic_t val = CT_ATOMIC_VAR_INIT(10);

	// dec 应该返回旧值 10, 新值应该是 9
	assert_int64_eq(ct_atomic_dec(&val), 10);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 9);

	// inc 应该返回旧值 9, 新值应该是 10
	assert_int64_eq(ct_atomic_inc(&val), 9);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 10);

	// sub 应该返回旧值 10, 新值应该是 5
	assert_int64_eq(ct_atomic_sub(&val, 5), 10);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 5);

	// add 应该返回旧值 5, 新值应该是 10
	assert_int64_eq(ct_atomic_add(&val, 5), 5);
	assert_int64_eq(CT_ATOMIC_LOAD(&val), 10);
}

static void test_overflow(void) {
	// 测试溢出 (LONG_MAX + 1 应该回绕到 LONG_MIN)
	{
		ct_atomic_t max_val = CT_ATOMIC_VAR_INIT(LONG_MAX);
		CT_ATOMIC_ADD(&max_val, 1);
		assert_int64_eq(CT_ATOMIC_LOAD(&max_val), LONG_MIN);
	}

	// 测试下溢 (LONG_MIN - 1 应该回绕到 LONG_MAX)
	{
		ct_atomic_t min_val = CT_ATOMIC_VAR_INIT(LONG_MIN);
		CT_ATOMIC_SUB(&min_val, 1);
		assert_int64_eq(CT_ATOMIC_LOAD(&min_val), LONG_MAX);
	}
}

static void test_concurrent_inc_dec(void) {
	pthread_t threads[NUM_THREADS];

	CT_ATOMIC_STORE(&g_shared_counter, 0);

	// 创建一半线程来增加
	for (int i = 0; i < NUM_THREADS / 2; i++) {
		pthread_create(&threads[i], NULL, thread_increment_routine, NULL);
	}

	// 创建另一半来减少
	for (int i = NUM_THREADS / 2; i < NUM_THREADS; i++) {
		pthread_create(&threads[i], NULL, thread_decrement_routine, NULL);
	}

	// 等待所有线程完成
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	// 最终结果应该为 0 如果所有操作都是原子的
	assert_int64_eq(CT_ATOMIC_LOAD(&g_shared_counter), 0);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("Basic test", NULL, NULL);
	CUNIT_TEST("Ensures atomic flag can be set, tested, and cleared", test_flag)
	CUNIT_TEST("Ensures atomic load and store operate correctly", test_load_store)
	CUNIT_TEST("Ensures atomic add and sub work in a single thread", test_add_sub)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Edge cases", NULL, NULL);
	CUNIT_TEST("Verifies that atomic operations return the previous value", test_return_value)
	CUNIT_TEST("Verifies that atomic operations handle integer overflow", test_overflow)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Ensures concurrent", NULL, NULL);
	CUNIT_TEST("Ensures concurrent increments and decrements are atomic", test_concurrent_inc_dec)
	CUNIT_SUITE_END()

	return cunit_run();
}
