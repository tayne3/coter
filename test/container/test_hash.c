/**
 * @file test_hash.c
 * @brief 哈希表测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "base/ct_platform.h"
#include "common/ct_any.h"
#include "container/ct_hash.h"
#include "ctunit.h"
#include "sched.h"

#define TEST_NUMBER 10000

static char test_keys[TEST_NUMBER][64];
static char test_values[TEST_NUMBER][64];

static inline void test_hash_basic(void);

int main(void) {
	test_hash_basic();
	ctunit_trace("Finish! test_hash_basic();\n");

	ctunit_pass();
}

static inline void test_hash_basic(void) {
	ct_hash_buf_t chash;
	ct_hash_init_s(chash, 1, true, ct_any_methods_default);

	ctunit_assert_uint32(ct_hash_size(chash), 0, CTUnit_Equal);
	ct_hash_clear(chash);
	ctunit_assert_uint32(ct_hash_size(chash), 0, CTUnit_Equal);

	ctunit_assert_false(ct_hash_insert(chash, "", ct_any_null));
	ctunit_assert_false(ct_hash_remove(chash, ""));
	ctunit_assert_false(ct_hash_value_r(chash, "", NULL));

	for (register int i = 0; i < TEST_NUMBER; i++) {
		ct_snprintf(test_keys[i], sizeof(test_keys[0]), "Key%03d", i + 1);
		ct_snprintf(test_values[i], sizeof(test_keys[0]), "Value%03d", i + 1);
		sched_yield();
	}

	const size_t size = ct_arrsize(test_keys);
	ct_any_t     value;
	const char  *key;

	for (register size_t i = 0; i < size; i++) {
		key   = test_keys[i];
		value = CT_ANY_STRING(test_values[i]);

		ctunit_assert_false(ct_hash_contains(chash, key));
		ctunit_assert_uint32(ct_hash_size(chash), i, CTUnit_Equal);

		ctunit_assert_true(ct_hash_insert(chash, key, value));

		ctunit_assert_true(ct_hash_contains(chash, key));
		ctunit_assert_uint32(ct_hash_size(chash), i + 1, CTUnit_Equal);

		sched_yield();
	}
	for (register size_t i = 0; i < size; i++) {
		key = test_keys[i];

		ctunit_assert_true(ct_hash_contains(chash, key));
		ctunit_assert_uint32(ct_hash_size(chash), size - i, CTUnit_Equal);

		ctunit_assert_true(ct_hash_remove(chash, key));

		ctunit_assert_false(ct_hash_contains(chash, key));
		ctunit_assert_uint32(ct_hash_size(chash), size - i - 1, CTUnit_Equal);

		ctunit_assert_false(ct_hash_remove(chash, key));

		ctunit_assert_false(ct_hash_contains(chash, key));
		ctunit_assert_uint32(ct_hash_size(chash), size - i - 1, CTUnit_Equal);
		sched_yield();
	}

	ctunit_assert_uint32(ct_hash_size(chash), 0, CTUnit_Equal);
	ctunit_assert_true(ct_hash_isempty(chash));

	for (register size_t i = 0; i < size; i++) {
		key   = test_keys[i];
		value = CT_ANY_STRING(test_values[i]);

		ctunit_assert_true(ct_hash_insert(chash, key, value));

		value = ct_hash_value(chash, key);
		ctunit_assert_string(ct_any_value_string(value), test_values[i]);

		value = ct_hash_value_s(chash, key, ct_any_null);
		ctunit_assert_string(ct_any_value_string(value), test_values[i]);

		ctunit_assert_true(ct_hash_value_r(chash, key, &value));
		ctunit_assert_string(ct_any_value_string(value), test_values[i]);
		sched_yield();
	}

	ctunit_assert_uint32(ct_hash_size(chash), size, CTUnit_Equal);
	ctunit_assert_false(ct_hash_isempty(chash));

	ct_hash_clear(chash);
	ctunit_assert_uint32(ct_hash_size(chash), 0, CTUnit_Equal);
	ctunit_assert_true(ct_hash_isempty(chash));

	for (register size_t i = 0; i < size; i++) {
		key   = test_keys[i];
		value = CT_ANY_STRING(test_values[i]);

		ctunit_assert_uint32(ct_hash_size(chash), 0, CTUnit_Equal);
		ctunit_assert_false(ct_hash_contains(chash, key));
		ctunit_assert_true(ct_hash_isempty(chash));

		ctunit_assert_true(ct_hash_insert(chash, key, value));

		ctunit_assert_uint32(ct_hash_size(chash), 1, CTUnit_Equal);
		ctunit_assert_true(ct_hash_contains(chash, key));
		ctunit_assert_false(ct_hash_isempty(chash));

		ctunit_assert_true(ct_hash_remove(chash, key));

		ctunit_assert_uint32(ct_hash_size(chash), 0, CTUnit_Equal);
		ctunit_assert_false(ct_hash_contains(chash, key));
		ctunit_assert_true(ct_hash_isempty(chash));
		sched_yield();
	}

	ct_hash_destroy(chash);
}
