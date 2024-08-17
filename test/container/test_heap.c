/**
 * @file test_heap.c
 * @brief 堆测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "common/ct_random.h"
#include "container/ct_heap.h"
#include "ctunit.h"

static inline bool ct_heap_sort_func(const ct_any_buf_t a, const ct_any_buf_t b);
static inline int  test_heap_init(void);
static inline int  test_heap_insert(void);
static inline int  test_heap_remove(void);

int main(void) {
	test_heap_init();
	ctunit_trace("Finish! test_heap_init();\n");

	test_heap_insert();
	ctunit_trace("Finish! test_heap_insert();\n");

	test_heap_remove();
	ctunit_trace("Finish! test_heap_remove();\n");

	ctunit_pass();
}

static inline bool ct_heap_sort_func(const ct_any_buf_t a, const ct_any_buf_t b) {
	return a->d->i <= b->d->i;
}

static inline int test_heap_init(void) {
	ct_heap_t    heap;
	ct_any_t     all[1000];
	const size_t max = ct_arrsize(all);

	for (size_t i = 1; i <= max; i++) {
		// 初始化队列
		ct_heap_init(&heap, all, i, ct_heap_sort_func);

		ctunit_assert_int32(ct_heap_max(&heap), i, CTUnit_Equal);
		ctunit_assert_int32(ct_heap_size(&heap), 0, CTUnit_Equal);
		ctunit_assert_true(ct_heap_isempty(&heap));
		ctunit_assert_true(!ct_heap_isfull(&heap));
	}

	ct_heap_clear(&heap);
	ctunit_assert_int32(ct_heap_max(&heap), max, CTUnit_Equal);
	ctunit_assert_int32(ct_heap_size(&heap), 0, CTUnit_Equal);
	ctunit_assert_true(ct_heap_isempty(&heap));
	ctunit_assert_true(!ct_heap_isfull(&heap));

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
			value = max - i;
			ct_heap_insert(&heap, CT_ANY_INT(value));

			ctunit_assert_int32(ct_heap_max(&heap), max, CTUnit_Equal);
			ctunit_assert_int32(ct_heap_size(&heap), i + 1, CTUnit_Equal);
			ctunit_assert_true(!ct_heap_isempty(&heap));
			if (i + 1 >= max) {
				ctunit_assert_true(ct_heap_isfull(&heap));
			} else {
				ctunit_assert_true(!ct_heap_isfull(&heap));
			}
		}
	}

	ct_heap_clear(&heap);
	ctunit_assert_int32(ct_heap_max(&heap), max, CTUnit_Equal);
	ctunit_assert_int32(ct_heap_size(&heap), 0, CTUnit_Equal);
	ctunit_assert_true(ct_heap_isempty(&heap));
	ctunit_assert_true(!ct_heap_isfull(&heap));

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
			prev = ct_heap_first(&heap).d->i;
			ct_heap_remove(&heap);

			ctunit_assert_int32(ct_heap_max(&heap), max, CTUnit_Equal);
			ctunit_assert_int32(ct_heap_size(&heap), max - i - 1, CTUnit_Equal);
			ctunit_assert_true(!ct_heap_isfull(&heap));
			if (i + 1 >= max) {
				ctunit_assert_true(ct_heap_isempty(&heap));
			} else {
				ctunit_assert_true(!ct_heap_isempty(&heap));
				next = ct_heap_first(&heap).d->i;
				ctunit_assert_int32(prev, next, CTUnit_LessEqual);
			}
		}
	}

	ct_heap_clear(&heap);
	ctunit_assert_int32(ct_heap_max(&heap), max, CTUnit_Equal);
	ctunit_assert_int32(ct_heap_size(&heap), 0, CTUnit_Equal);
	ctunit_assert_true(ct_heap_isempty(&heap));
	ctunit_assert_true(!ct_heap_isfull(&heap));

	return 0;
}
