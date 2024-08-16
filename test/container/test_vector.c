/**
 * @file test_vector.c
 * @brief 向量测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.15
 */
#include "common/ct_random.h"
#include "container/ct_vector.h"
#include "ctunit.h"

static inline void test_vector_resize(void);
static inline void test_vector_get(void);
static inline void test_vector_at(void);

int main(void) {
	test_vector_resize();
	ctunit_trace("Finish! test_vector_resize();\n");

	test_vector_get();
	ctunit_trace("Finish! test_vector_get();\n");

	test_vector_at();
	ctunit_trace("Finish! test_vector_at();\n");

	ctunit_pass();
}

static inline void test_vector_resize(void) {
	ct_vector_buf_t vec;
	ct_vector_init(vec, sizeof(int), 10);
	ctunit_assert_uint32(ct_vector_size(vec), 10, CTUnit_Equal);

	ct_vector_resize(vec, 10);
	ctunit_assert_uint32(ct_vector_size(vec), 10, CTUnit_Equal);
	ct_vector_resize(vec, 50);
	ctunit_assert_uint32(ct_vector_size(vec), 50, CTUnit_Equal);
	ct_vector_resize(vec, 90);
	ctunit_assert_uint32(ct_vector_size(vec), 90, CTUnit_Equal);
	ct_vector_resize(vec, 50);
	ctunit_assert_uint32(ct_vector_size(vec), 50, CTUnit_Equal);
	ct_vector_resize(vec, 10);
	ctunit_assert_uint32(ct_vector_size(vec), 10, CTUnit_Equal);

	ct_vector_destroy(vec);
}

static inline void test_vector_get(void) {
	ct_vector_buf_t vec;
	ct_vector_init(vec, sizeof(int), 1);
	ctunit_assert_uint32(ct_vector_size(vec), 1, CTUnit_Equal);

	for (size_t i = 0; i < 100; i++) {
		ct_vector_insert(vec, i, &i);
	}

	{
		const int *it = ct_nullptr;
		for (size_t i = 0; i < 100; i++) {
			it = ct_vector_value(vec, i);
			ctunit_assert_int32(*it, i, CTUnit_Equal);
		}
	}

	{
		const int *it = ct_nullptr;
		ct_vector_resize(vec, 50);
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_value(vec, i);
			ctunit_assert_int32(*it, i, CTUnit_Equal);
		}
	}

	{
		const int *it = ct_nullptr;
		ct_vector_resize(vec, 10);
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_value(vec, i);
			ctunit_assert_int32(*it, i, CTUnit_Equal);
		}
	}

	ct_vector_destroy(vec);
}

static inline void test_vector_at(void) {
	ct_vector_buf_t vec;
	ct_vector_init(vec, sizeof(int), 1);
	ctunit_assert_uint32(ct_vector_size(vec), 1, CTUnit_Equal);

	for (size_t i = 0; i < 100; i++) {
		ct_vector_insert(vec, i, &i);
	}

	{
		int *it = ct_nullptr;
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_at(vec, i);
			ctunit_assert_int32(*it, i, CTUnit_Equal);
		}
	}

	{
		int *it = ct_nullptr;
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it  = ct_vector_at(vec, i);
			*it = i + 1;
		}
	}

	{
		const int *it = ct_nullptr;
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_value(vec, i);
			ctunit_assert_int32(*it, i + 1, CTUnit_Equal);
		}
	}

	ct_vector_destroy(vec);
}
