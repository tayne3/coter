/**
 * @file vector_test.c
 * @brief 向量测试
 */
#include "coter/base/random.h"
#include "coter/container/vector.h"
#include "cunit.h"

static inline void test_vector_resize(void) {
	ct_vector_buf_t vec;
	ct_vector_init(vec, sizeof(int), 10);
	assert_uint32_eq(ct_vector_size(vec), 10);

	ct_vector_resize(vec, 10);
	assert_uint32_eq(ct_vector_size(vec), 10);
	ct_vector_resize(vec, 50);
	assert_uint32_eq(ct_vector_size(vec), 50);
	ct_vector_resize(vec, 90);
	assert_uint32_eq(ct_vector_size(vec), 90);
	ct_vector_resize(vec, 50);
	assert_uint32_eq(ct_vector_size(vec), 50);
	ct_vector_resize(vec, 10);
	assert_uint32_eq(ct_vector_size(vec), 10);

	ct_vector_destroy(vec);
}

static inline void test_vector_get(void) {
	ct_vector_buf_t vec;
	ct_vector_init(vec, sizeof(int), 1);
	assert_uint32_eq(ct_vector_size(vec), 1);

	for (int i = 0; i < 100; i++) {
		ct_vector_insert(vec, i, &i);
	}

	{
		const int *it = NULL;
		for (int i = 0; i < 100; i++) {
			it = ct_vector_value(vec, i);
			assert_int32_eq(*it, i);
		}
	}

	{
		const int *it = NULL;
		ct_vector_resize(vec, 50);
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_value(vec, i);
			assert_int32_eq(*it, i);
		}
	}

	{
		const int *it = NULL;
		ct_vector_resize(vec, 10);
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_value(vec, i);
			assert_int32_eq(*it, i);
		}
	}

	ct_vector_destroy(vec);
}

static inline void test_vector_at(void) {
	ct_vector_buf_t vec;
	ct_vector_init(vec, sizeof(int), 1);
	assert_uint32_eq(ct_vector_size(vec), 1);

	for (int i = 0; i < 100; i++) {
		ct_vector_insert(vec, i, &i);
	}

	{
		int *it = NULL;
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_at(vec, i);
			assert_int32_eq(*it, i);
		}
	}

	{
		int *it = NULL;
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it  = ct_vector_at(vec, i);
			*it = i + 1;
		}
	}

	{
		const int *it = NULL;
		for (size_t i = 0; i < ct_vector_size(vec); i++) {
			it = ct_vector_value(vec, i);
			assert_int32_eq(*it, i + 1);
		}
	}

	ct_vector_destroy(vec);
}

int main(void) {
	test_vector_resize();
	cunit_println("Finish! test_vector_resize();");

	test_vector_get();
	cunit_println("Finish! test_vector_get();");

	test_vector_at();
	cunit_println("Finish! test_vector_at();");

	cunit_pass();
}
