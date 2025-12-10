/**
 * @file atomic_test.c
 * @brief 原子操作测试
 */
#include "coter/sync/atomic.h"

#include "cunit.h"

static void test_flag(void) {
	ct_atomic_flag_t flag = CT_ATOMIC_FLAG_INIT;

	// 第一次 test-and-set 应该返回 false (前一个状态)
	assert_false(ct_atomic_flag_test_and_set(&flag));
	// 后续调用应该返回 true
	assert_true(ct_atomic_flag_test_and_set(&flag));
	assert_true(ct_atomic_flag_test_and_set(&flag));

	// 清除标志
	ct_atomic_flag_clear(&flag);

	// test-and-set 应该再次返回 false
	assert_false(ct_atomic_flag_test_and_set(&flag));
	assert_true(ct_atomic_flag_test_and_set(&flag));
}

static void test_load_store(void) {
	ct_atomic_long_t val = CT_ATOMIC_VAR_INIT(42);
	assert_int64_eq(ct_atomic_long_load(&val), 42);

	ct_atomic_long_store(&val, 100);
	assert_int64_eq(ct_atomic_long_load(&val), 100);

	// 测试边界值
	ct_atomic_long_store(&val, LONG_MAX);
	assert_int64_eq(ct_atomic_long_load(&val), LONG_MAX);

	ct_atomic_long_store(&val, LONG_MIN);
	assert_int64_eq(ct_atomic_long_load(&val), LONG_MIN);
}

static void test_add_sub(void) {
	ct_atomic_long_t val = CT_ATOMIC_VAR_INIT(0);
	assert_int64_eq(ct_atomic_long_load(&val), 0);

	ct_atomic_long_add(&val, 10);
	assert_int64_eq(ct_atomic_long_load(&val), 10);

	ct_atomic_long_sub(&val, 5);
	assert_int64_eq(ct_atomic_long_load(&val), 5);

	// 测试负数
	ct_atomic_long_add(&val, -15);
	assert_int64_eq(ct_atomic_long_load(&val), -10);

	ct_atomic_long_sub(&val, -20);
	assert_int64_eq(ct_atomic_long_load(&val), 10);
}

static void test_return_value(void) {
	ct_atomic_long_t val = CT_ATOMIC_VAR_INIT(10);

	// dec 应该返回旧值 10, 新值应该是 9
	assert_int64_eq(ct_atomic_long_sub(&val, 1), 10);
	assert_int64_eq(ct_atomic_long_load(&val), 9);

	// inc 应该返回旧值 9, 新值应该是 10
	assert_int64_eq(ct_atomic_long_add(&val, 1), 9);
	assert_int64_eq(ct_atomic_long_load(&val), 10);

	// sub 应该返回旧值 10, 新值应该是 5
	assert_int64_eq(ct_atomic_long_sub(&val, 5), 10);
	assert_int64_eq(ct_atomic_long_load(&val), 5);

	// add 应该返回旧值 5, 新值应该是 10
	assert_int64_eq(ct_atomic_long_add(&val, 5), 5);
	assert_int64_eq(ct_atomic_long_load(&val), 10);
}

static void test_overflow(void) {
	// 测试溢出 (LONG_MAX + 1 应该回绕到 LONG_MIN)
	{
		ct_atomic_long_t max_val = CT_ATOMIC_VAR_INIT(LONG_MAX);
		ct_atomic_long_add(&max_val, 1);
		assert_int64_eq(ct_atomic_long_load(&max_val), LONG_MIN);
	}

	// 测试下溢 (LONG_MIN - 1 应该回绕到 LONG_MAX)
	{
		ct_atomic_long_t min_val = CT_ATOMIC_VAR_INIT(LONG_MIN);
		ct_atomic_long_sub(&min_val, 1);
		assert_int64_eq(ct_atomic_long_load(&min_val), LONG_MAX);
	}
}

// 测试显式原子类型定义
static void test_explicit_atomic_types(void) {
	// 测试 ct_atomic_int_t
	ct_atomic_int_t atomic_int = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_store(&atomic_int, 123);
	assert_int_eq(ct_atomic_int_load(&atomic_int), 123);

	// 测试 ct_atomic_bool_t
	ct_atomic_bool_t atomic_bool = CT_ATOMIC_VAR_INIT(false);
	ct_atomic_bool_store(&atomic_bool, true);
	assert_true(ct_atomic_bool_load(&atomic_bool));

	// 测试 ct_atomic_uint_t
	ct_atomic_uint_t atomic_uint = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_uint_store(&atomic_uint, 0xDEADBEEF);
	assert_uint_eq(ct_atomic_uint_load(&atomic_uint), 0xDEADBEEF);

	// 测试 ct_atomic_llong_t
	ct_atomic_llong_t atomic_llong = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_llong_store(&atomic_llong, 9999999999LL);
	assert_int64_eq(ct_atomic_llong_load(&atomic_llong), 9999999999LL);
}

static void test_atomic_ptr_basic(void) {
	int             val1 = 10;
	int             val2 = 20;
	ct_atomic_ptr_t ptr  = CT_ATOMIC_VAR_INIT(&val1);

	// Load
	assert_true(ct_atomic_ptr_load(&ptr) == &val1);

	// Store
	ct_atomic_ptr_store(&ptr, &val2);
	assert_true(ct_atomic_ptr_load(&ptr) == &val2);

	// Exchange
	void* old = ct_atomic_ptr_exchange(&ptr, &val1);
	assert_true(old == &val2);
	assert_true(ct_atomic_ptr_load(&ptr) == &val1);
}

static void test_atomic_ptr_compare_exchange(void) {
	int             val1 = 10;
	int             val2 = 20;
	int             val3 = 30;
	ct_atomic_ptr_t ptr  = CT_ATOMIC_VAR_INIT(&val1);

	// Success case
	void* expected = &val1;
	bool  success  = ct_atomic_ptr_compare_exchange(&ptr, &expected, &val2);
	assert_true(success);
	assert_true(ct_atomic_ptr_load(&ptr) == &val2);
	assert_true(expected == &val1);  // Expected should not change on success

	// Failure case
	expected = &val3;  // Wrong expected value
	success  = ct_atomic_ptr_compare_exchange(&ptr, &expected, &val1);
	assert_false(success);
	assert_true(ct_atomic_ptr_load(&ptr) == &val2);  // Should not change
	assert_true(expected == &val2);                  // Expected should be updated to current value
}

static void test_cas_aba_behavior(void) {
	int             val1 = 10;
	int             val2 = 20;
	ct_atomic_ptr_t ptr  = CT_ATOMIC_VAR_INIT(&val1);

	// Thread A reads A
	void* expected = &val1;

	// Thread B changes A -> B
	ct_atomic_ptr_store(&ptr, &val2);

	// Thread B changes B -> A
	ct_atomic_ptr_store(&ptr, &val1);

	// Thread A attempts CAS A -> C
	int  val3    = 30;
	bool success = ct_atomic_ptr_compare_exchange(&ptr, &expected, &val3);

	assert_true(success);
	assert_true(ct_atomic_ptr_load(&ptr) == &val3);
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
	CUNIT_TEST("Ensures explicit atomic types work correctly", test_explicit_atomic_types)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Pointer operations", NULL, NULL);
	CUNIT_TEST("Basic pointer operations", test_atomic_ptr_basic)
	CUNIT_TEST("Pointer compare exchange", test_atomic_ptr_compare_exchange)
	CUNIT_TEST("CAS ABA behavior check", test_cas_aba_behavior)
	CUNIT_SUITE_END()

	return cunit_run();
}
