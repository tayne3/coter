/**
 * @file heap_test.c
 * @brief 堆测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "base/ct_random.h"
#include "container/ct_heap.h"
#include "cunit.h"

static inline bool ct_heap_sort_func(const ct_any_t* a, const ct_any_t* b) {
	return ct_any_value_int(*a) <= ct_any_value_int(*b);
}

static inline int test_heap_init(void) {
	ct_heap_t    heap;
	ct_any_t     all[1000];
	const size_t max = ct_arrsize(all);

	for (size_t i = 1; i <= max; i++) {
		// 初始化队列
		ct_heap_init(&heap, all, i, ct_heap_sort_func);

		cunit_assert_uint32_equal(ct_heap_max(&heap), i);
		cunit_assert_uint32_equal(ct_heap_size(&heap), 0);
		cunit_assert_true(ct_heap_isempty(&heap));
		cunit_assert_true(!ct_heap_isfull(&heap));
	}

	ct_heap_clear(&heap);
	cunit_assert_uint32_equal(ct_heap_max(&heap), max);
	cunit_assert_uint32_equal(ct_heap_size(&heap), 0);
	cunit_assert_true(ct_heap_isempty(&heap));
	cunit_assert_true(!ct_heap_isfull(&heap));

	return 0;
}

static inline int test_heap_insert(void) {
	ct_heap_t    heap;
	ct_any_t     all[10];
	const size_t max = ct_arrsize(all);

	// 初始化队列
	ct_heap_init(&heap, all, max, (ct_heap_sort_t)ct_heap_sort_func);

	{
		int value = 0;
		for (size_t i = 0; i < max; i++) {
			value = (int)(max - i);
			ct_heap_insert(&heap, CT_ANY_INT(value));

			cunit_assert_uint32_equal(ct_heap_max(&heap), max);
			cunit_assert_uint32_equal(ct_heap_size(&heap), i + 1);
			cunit_assert_true(!ct_heap_isempty(&heap));
			if (i + 1 >= max) {
				cunit_assert_true(ct_heap_isfull(&heap));
			} else {
				cunit_assert_true(!ct_heap_isfull(&heap));
			}
		}
	}

	ct_heap_clear(&heap);
	cunit_assert_uint32_equal(ct_heap_max(&heap), max);
	cunit_assert_uint32_equal(ct_heap_size(&heap), 0);
	cunit_assert_true(ct_heap_isempty(&heap));
	cunit_assert_true(!ct_heap_isfull(&heap));

	return 0;
}

static inline int test_heap_remove(void) {
	ct_heap_t    heap;
	ct_any_t     all[10];
	const size_t max = ct_arrsize(all);

	// 初始化队列
	ct_heap_init(&heap, all, max, (ct_heap_sort_t)ct_heap_sort_func);

	{
		int         it = 0;
		ct_random_t state;
		ct_random_init(&state);

		for (size_t i = 0; i < max; i++) {
			it = ct_random_int32(&state, -999, 999);
			ct_heap_insert(&heap, CT_ANY_INT(it));
		}
	}

	{
		int prev = 0;
		int next = 0;
		for (size_t i = 0; i < max; i++) {
			prev = ct_any_value_int(ct_heap_first(&heap));
			ct_heap_remove(&heap);

			cunit_assert_uint32_equal(ct_heap_max(&heap), max);
			cunit_assert_uint32_equal(ct_heap_size(&heap), max - i - 1);
			cunit_assert_true(!ct_heap_isfull(&heap));
			if (i + 1 >= max) {
				cunit_assert_true(ct_heap_isempty(&heap));
			} else {
				cunit_assert_true(!ct_heap_isempty(&heap));
				next = ct_any_value_int(ct_heap_first(&heap));
				cunit_assert_int32_less_equal(prev, next);
			}
		}
	}

	ct_heap_clear(&heap);
	cunit_assert_uint32_equal(ct_heap_max(&heap), max);
	cunit_assert_uint32_equal(ct_heap_size(&heap), 0);
	cunit_assert_true(ct_heap_isempty(&heap));
	cunit_assert_true(!ct_heap_isfull(&heap));

	return 0;
}

int main(void) {
	test_heap_init();
	cunit_println("Finish! test_heap_init();\n");

	test_heap_insert();
	cunit_println("Finish! test_heap_insert();\n");

	test_heap_remove();
	cunit_println("Finish! test_heap_remove();\n");

	cunit_pass();
}
