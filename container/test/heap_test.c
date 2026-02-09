/**
 * @file heap_test.c
 * @brief 堆测试
 */
#include "coter/container/heap.h"

#include "coter/math/rand.h"
#include "cunit.h"

static inline bool ct_heap_sort_func(const ct_any_t* a, const ct_any_t* b) {
	return ct_any_value_int(*a) <= ct_any_value_int(*b);
}

void test_heap_init(void) {
	ct_heap_t    heap;
	ct_any_t     all[1000];
	const size_t max = ct_arrsize(all);

	for (size_t i = 1; i <= max; ++i) {
		// 初始化队列
		ct_heap_init(&heap, all, i, ct_heap_sort_func);

		assert_uint32_eq(ct_heap_max(&heap), i);
		assert_uint32_eq(ct_heap_size(&heap), 0);
		assert_true(ct_heap_isempty(&heap));
		assert_true(!ct_heap_isfull(&heap));
	}

	ct_heap_clear(&heap);
	assert_uint32_eq(ct_heap_max(&heap), max);
	assert_uint32_eq(ct_heap_size(&heap), 0);
	assert_true(ct_heap_isempty(&heap));
	assert_true(!ct_heap_isfull(&heap));
}

void test_heap_insert(void) {
	ct_heap_t    heap;
	ct_any_t     all[10];
	const size_t max = ct_arrsize(all);

	// 初始化队列
	ct_heap_init(&heap, all, max, (ct_heap_sort_t)ct_heap_sort_func);

	{
		int value = 0;
		for (size_t i = 0; i < max; ++i) {
			value = (int)(max - i);
			ct_heap_insert(&heap, CT_ANY_INT(value));

			assert_uint32_eq(ct_heap_max(&heap), max);
			assert_uint32_eq(ct_heap_size(&heap), i + 1);
			assert_true(!ct_heap_isempty(&heap));
			if (i + 1 >= max) {
				assert_true(ct_heap_isfull(&heap));
			} else {
				assert_true(!ct_heap_isfull(&heap));
			}
		}
	}

	ct_heap_clear(&heap);
	assert_uint32_eq(ct_heap_max(&heap), max);
	assert_uint32_eq(ct_heap_size(&heap), 0);
	assert_true(ct_heap_isempty(&heap));
	assert_true(!ct_heap_isfull(&heap));
}

void test_heap_remove(void) {
	ct_heap_t    heap;
	ct_any_t     all[10];
	const size_t max = ct_arrsize(all);

	// 初始化队列
	ct_heap_init(&heap, all, max, (ct_heap_sort_t)ct_heap_sort_func);

	{
		int         it = 0;
		ct_random_t state;
		ct_random_init(&state);

		for (size_t i = 0; i < max; ++i) {
			it = ct_random_int32(&state, -999, 999);
			ct_heap_insert(&heap, CT_ANY_INT(it));
		}
	}

	{
		int prev = 0;
		int next = 0;
		for (size_t i = 0; i < max; ++i) {
			prev = ct_any_value_int(ct_heap_first(&heap));
			ct_heap_remove(&heap);

			assert_uint32_eq(ct_heap_max(&heap), max);
			assert_uint32_eq(ct_heap_size(&heap), max - i - 1);
			assert_true(!ct_heap_isfull(&heap));
			if (i + 1 >= max) {
				assert_true(ct_heap_isempty(&heap));
			} else {
				assert_true(!ct_heap_isempty(&heap));
				next = ct_any_value_int(ct_heap_first(&heap));
				assert_int32_le(prev, next);
			}
		}
	}

	ct_heap_clear(&heap);
	assert_uint32_eq(ct_heap_max(&heap), max);
	assert_uint32_eq(ct_heap_size(&heap), 0);
	assert_true(ct_heap_isempty(&heap));
	assert_true(!ct_heap_isfull(&heap));
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("heap", NULL, NULL)
	CUNIT_TEST("init", test_heap_init)
	CUNIT_TEST("insert", test_heap_insert)
	CUNIT_TEST("remove", test_heap_remove)
	CUNIT_SUITE_END()

	return cunit_run();
}
