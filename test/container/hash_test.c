/**
 * @file hash_test.c
 * @brief 哈希表测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "base/ct_any.h"
#include "base/ct_platform.h"
#include "container/ct_hash.h"
#include "cunit.h"
#include "sched.h"

#define TEST_NUMBER 10000

static char test_keys[TEST_NUMBER][64];
static char test_values[TEST_NUMBER][64];

static inline void test_hash_basic(void) {
	ct_hash_buf_t chash;
	ct_hash_init_s(chash, 1, true, ct_any_methods_default);

	assert_uint32_eq(ct_hash_size(chash), 0);
	ct_hash_clear(chash);
	assert_uint32_eq(ct_hash_size(chash), 0);

	assert_false(ct_hash_insert(chash, "", ct_any_null));
	assert_false(ct_hash_remove(chash, ""));
	assert_false(ct_hash_value_r(chash, "", NULL));

	for (register int i = 0; i < TEST_NUMBER; i++) {
		ct_snprintf_s(test_keys[i], sizeof(test_keys[0]), "Key%03d", i + 1);
		ct_snprintf_s(test_values[i], sizeof(test_keys[0]), "Value%03d", i + 1);
		sched_yield();
	}

	const size_t size = ct_arrsize(test_keys);
	ct_any_t     value;
	const char  *key;

	for (register size_t i = 0; i < size; i++) {
		key   = test_keys[i];
		value = CT_ANY_STRING(test_values[i]);

		assert_false(ct_hash_contains(chash, key));
		assert_uint32_eq(ct_hash_size(chash), i);

		assert_true(ct_hash_insert(chash, key, value));

		assert_true(ct_hash_contains(chash, key));
		assert_uint32_eq(ct_hash_size(chash), i + 1);

		sched_yield();
	}
	for (register size_t i = 0; i < size; i++) {
		key = test_keys[i];

		assert_true(ct_hash_contains(chash, key));
		assert_uint32_eq(ct_hash_size(chash), size - i);

		assert_true(ct_hash_remove(chash, key));

		assert_false(ct_hash_contains(chash, key));
		assert_uint32_eq(ct_hash_size(chash), size - i - 1);

		assert_false(ct_hash_remove(chash, key));

		assert_false(ct_hash_contains(chash, key));
		assert_uint32_eq(ct_hash_size(chash), size - i - 1);
		sched_yield();
	}

	assert_uint32_eq(ct_hash_size(chash), 0);
	assert_true(ct_hash_isempty(chash));

	for (register size_t i = 0; i < size; i++) {
		key   = test_keys[i];
		value = CT_ANY_STRING(test_values[i]);

		assert_true(ct_hash_insert(chash, key, value));

		value = ct_hash_value(chash, key);
		assert_str_eq(ct_any_value_string(value), test_values[i]);

		value = ct_hash_value_s(chash, key, ct_any_null);
		assert_str_eq(ct_any_value_string(value), test_values[i]);

		assert_true(ct_hash_value_r(chash, key, &value));
		assert_str_eq(ct_any_value_string(value), test_values[i]);
		sched_yield();
	}

	assert_uint32_eq(ct_hash_size(chash), size);
	assert_false(ct_hash_isempty(chash));

	ct_hash_clear(chash);
	assert_uint32_eq(ct_hash_size(chash), 0);
	assert_true(ct_hash_isempty(chash));

	for (register size_t i = 0; i < size; i++) {
		key   = test_keys[i];
		value = CT_ANY_STRING(test_values[i]);

		assert_uint32_eq(ct_hash_size(chash), 0);
		assert_false(ct_hash_contains(chash, key));
		assert_true(ct_hash_isempty(chash));

		assert_true(ct_hash_insert(chash, key, value));

		assert_uint32_eq(ct_hash_size(chash), 1);
		assert_true(ct_hash_contains(chash, key));
		assert_false(ct_hash_isempty(chash));

		assert_true(ct_hash_remove(chash, key));

		assert_uint32_eq(ct_hash_size(chash), 0);
		assert_false(ct_hash_contains(chash, key));
		assert_true(ct_hash_isempty(chash));
		sched_yield();
	}

	ct_hash_destroy(chash);
}

int main(void) {
	test_hash_basic();
	cunit_println("Finish! test_hash_basic();");

	cunit_pass();
}
