/**
 * @file vector_define_test.c
 * @brief 动态数组宏定义测试
 */
#include "coter/container/vector_define.h"

#include "cunit.h"

CT_VECTOR_DECLARE(int, i32)
CT_VECTOR_DEFINE(int, i32)

// Test with double
CT_VECTOR_DECLARE(double, f64)
CT_VECTOR_DEFINE(double, f64)

// Test with a struct
typedef struct my_struct {
	int  id;
	char name[20];
} my_struct_t;

CT_VECTOR_DECLARE(my_struct_t, my_struct)
CT_VECTOR_DEFINE(my_struct_t, my_struct)

#define TEST_VECTOR_SIZE 100

static void test_vector_define_init(void) {
	ct_vector_i32_t vec[1];

	// Test initialization with 0 capacity - note: define version requires > 0 capacity
	ct_vector_i32_init(vec, 1);
	assert_uint32_eq(ct_vector_i32_size(vec), 0);
	assert_uint32_eq(ct_vector_i32_capacity(vec), 1);
	assert_true(ct_vector_i32_empty(vec));
	ct_vector_i32_destroy(vec);

	// Test initialization with non-zero capacity
	ct_vector_i32_init(vec, 10);
	assert_uint32_eq(ct_vector_i32_size(vec), 0);
	assert_uint32_ge(ct_vector_i32_capacity(vec), 10);  // capacity can be greater or equal
	assert_true(ct_vector_i32_empty(vec));
	ct_vector_i32_destroy(vec);
}

static void test_vector_define_push_pop(void) {
	ct_vector_i32_t vec[1];
	ct_vector_i32_init(vec, 1);

	// Push elements
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		assert_true(ct_vector_i32_push(vec, i));
		assert_uint32_eq(ct_vector_i32_size(vec), i + 1);
		const int* back = ct_vector_i32_back(vec);
		assert_not_null(back);
		assert_int32_eq(*back, i);
	}
	assert_false(ct_vector_i32_empty(vec));
	assert_uint32_eq(ct_vector_i32_size(vec), TEST_VECTOR_SIZE);

	// Check elements
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		const int* val = ct_vector_i32_value(vec, i);
		assert_not_null(val);
		assert_int32_eq(*val, i);
	}

	// Pop elements
	for (int i = TEST_VECTOR_SIZE - 1; i >= 0; --i) {
		const int* back = ct_vector_i32_back(vec);
		assert_not_null(back);
		assert_int32_eq(*back, i);
		assert_uint32_eq(ct_vector_i32_size(vec), i + 1);
		ct_vector_i32_pop(vec);
	}
	assert_uint32_eq(ct_vector_i32_size(vec), 0);
	assert_true(ct_vector_i32_empty(vec));

	ct_vector_i32_destroy(vec);
}

static void test_vector_define_access(void) {
	ct_vector_i32_t vec[1];
	ct_vector_i32_init(vec, 1);

	// Test on empty vector
	assert_null(ct_vector_i32_at(vec, 0));
	assert_null(ct_vector_i32_value(vec, 0));
	assert_null(ct_vector_i32_front(vec));
	assert_null(ct_vector_i32_back(vec));

	// Push elements
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		ct_vector_i32_push(vec, i);
	}

	// Test front and back
	const int* front = ct_vector_i32_front(vec);
	const int* back  = ct_vector_i32_back(vec);
	assert_not_null(front);
	assert_not_null(back);
	assert_int32_eq(*front, 0);
	assert_int32_eq(*back, TEST_VECTOR_SIZE - 1);

	// Test at and value
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		const int* val    = ct_vector_i32_value(vec, i);
		int*       at_val = ct_vector_i32_at(vec, i);
		assert_not_null(val);
		assert_not_null(at_val);
		assert_int32_eq(*val, i);
		assert_int32_eq(*at_val, i);

		// Modify through at
		*at_val = i * 2;
	}

	// Verify modification
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		const int* val = ct_vector_i32_value(vec, i);
		assert_not_null(val);
		assert_int32_eq(*val, i * 2);
	}

	// Test out-of-bounds access
	assert_null(ct_vector_i32_at(vec, TEST_VECTOR_SIZE));
	assert_null(ct_vector_i32_value(vec, TEST_VECTOR_SIZE));

	ct_vector_i32_destroy(vec);
}

static void test_vector_define_insert(void) {
	ct_vector_i32_t vec[1];
	ct_vector_i32_init(vec, 1);

	// Insert into empty vector
	int val = 100;
	assert_true(ct_vector_i32_insert(vec, 0, val));
	assert_uint32_eq(ct_vector_i32_size(vec), 1);
	assert_int32_eq(*ct_vector_i32_at(vec, 0), 100);

	// Insert at the end
	val = 200;
	assert_true(ct_vector_i32_insert(vec, 1, val));
	assert_uint32_eq(ct_vector_i32_size(vec), 2);
	assert_int32_eq(*ct_vector_i32_at(vec, 0), 100);
	assert_int32_eq(*ct_vector_i32_at(vec, 1), 200);

	// Insert at the beginning
	val = 0;
	assert_true(ct_vector_i32_insert(vec, 0, val));
	assert_uint32_eq(ct_vector_i32_size(vec), 3);
	assert_int32_eq(*ct_vector_i32_at(vec, 0), 0);
	assert_int32_eq(*ct_vector_i32_at(vec, 1), 100);
	assert_int32_eq(*ct_vector_i32_at(vec, 2), 200);

	// Insert in the middle
	val = 50;
	assert_true(ct_vector_i32_insert(vec, 1, val));
	assert_uint32_eq(ct_vector_i32_size(vec), 4);
	assert_int32_eq(*ct_vector_i32_at(vec, 0), 0);
	assert_int32_eq(*ct_vector_i32_at(vec, 1), 50);
	assert_int32_eq(*ct_vector_i32_at(vec, 2), 100);
	assert_int32_eq(*ct_vector_i32_at(vec, 3), 200);

	// Insert out of bounds (should fail or be ignored)
	val = 999;
	assert_false(ct_vector_i32_insert(vec, 5, val));  // position > size
	assert_uint32_eq(ct_vector_i32_size(vec), 4);

	ct_vector_i32_destroy(vec);
}

static void test_vector_define_erase(void) {
	ct_vector_i32_t vec[1];
	ct_vector_i32_init(vec, 1);

	for (int i = 0; i < 10; ++i) {
		ct_vector_i32_push(vec, i);
	}
	// vec is {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
	assert_uint32_eq(ct_vector_i32_size(vec), 10);

	// Erase from middle
	ct_vector_i32_erase(vec, 5);  // erase 5
	assert_uint32_eq(ct_vector_i32_size(vec), 9);
	assert_int32_eq(*ct_vector_i32_value(vec, 4), 4);
	assert_int32_eq(*ct_vector_i32_value(vec, 5), 6);

	// Erase from beginning
	ct_vector_i32_erase(vec, 0);  // erase 0
	assert_uint32_eq(ct_vector_i32_size(vec), 8);
	assert_int32_eq(*ct_vector_i32_front(vec), 1);

	// Erase from end
	ct_vector_i32_erase(vec, 7);  // erase 9, which is now at index 7
	assert_uint32_eq(ct_vector_i32_size(vec), 7);
	assert_int32_eq(*ct_vector_i32_back(vec), 8);

	// Erase until empty
	while (!ct_vector_i32_empty(vec)) {
		ct_vector_i32_erase(vec, 0);
	}
	assert_uint32_eq(ct_vector_i32_size(vec), 0);

	// Erase from empty is a no-op, should return false
	assert_false(ct_vector_i32_erase(vec, 0));
	assert_true(ct_vector_i32_empty(vec));

	ct_vector_i32_destroy(vec);
}

static void test_vector_define_capacity_management(void) {
	ct_vector_i32_t vec[1];
	ct_vector_i32_init(vec, 10);
	assert_uint32_eq(ct_vector_i32_size(vec), 0);
	assert_uint32_ge(ct_vector_i32_capacity(vec), 10);
	size_t old_cap = ct_vector_i32_capacity(vec);

	// Reserve less or equal, should not change capacity
	assert_true(ct_vector_i32_reserve(vec, old_cap));
	assert_uint32_eq(ct_vector_i32_capacity(vec), old_cap);
	assert_true(ct_vector_i32_reserve(vec, 5));
	assert_uint32_eq(ct_vector_i32_capacity(vec), old_cap);

	// Reserve more, should increase capacity
	assert_true(ct_vector_i32_reserve(vec, 100));
	assert_uint32_ge(ct_vector_i32_capacity(vec), 100);

	// Fill with some data
	for (int i = 0; i < 50; ++i) {
		ct_vector_i32_push(vec, i);
	}
	assert_uint32_eq(ct_vector_i32_size(vec), 50);

	// Resize smaller
	assert_true(ct_vector_i32_resize(vec, 20));
	assert_uint32_eq(ct_vector_i32_size(vec), 20);
	for (int i = 0; i < 20; ++i) {
		assert_int32_eq(*ct_vector_i32_value(vec, i), i);
	}

	// Resize larger
	size_t cap_before_resize = ct_vector_i32_capacity(vec);
	assert_true(ct_vector_i32_resize(vec, 60));
	assert_uint32_eq(ct_vector_i32_size(vec), 60);
	assert_uint32_ge(ct_vector_i32_capacity(vec), cap_before_resize);
	for (int i = 0; i < 20; ++i) {
		assert_int32_eq(*ct_vector_i32_value(vec, i), i);
	}

	// Shrink
	ct_vector_i32_t vec2[1];
	ct_vector_i32_init(vec2, 100);
	for (int i = 0; i < 10; ++i) {
		ct_vector_i32_push(vec2, i);
	}
	assert_uint32_ge(ct_vector_i32_capacity(vec2), 100);
	assert_true(ct_vector_i32_shrink(vec2));
	assert_uint32_eq(ct_vector_i32_capacity(vec2), 10);
	assert_uint32_eq(ct_vector_i32_size(vec2), 10);
	for (int i = 0; i < 10; ++i) {
		assert_int32_eq(*ct_vector_i32_value(vec2, i), i);
	}

	// Clear
	size_t cap_before_clear = ct_vector_i32_capacity(vec);
	ct_vector_i32_clear(vec);
	assert_uint32_eq(ct_vector_i32_size(vec), 0);
	assert_true(ct_vector_i32_empty(vec));
	assert_uint32_eq(ct_vector_i32_capacity(vec), cap_before_clear);  // capacity should not change

	ct_vector_i32_destroy(vec);
	ct_vector_i32_destroy(vec2);
}

static void test_vector_define_double(void) {
	ct_vector_f64_t vec[1];
	ct_vector_f64_init(vec, 1);

	// Push elements
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		assert_true(ct_vector_f64_push(vec, i * 1.5));
		assert_uint32_eq(ct_vector_f64_size(vec), i + 1);
		const double* back = ct_vector_f64_back(vec);
		assert_not_null(back);
		assert_true(fabs(*back - (i * 1.5)) < 1e-9);
	}
	assert_false(ct_vector_f64_empty(vec));
	assert_uint32_eq(ct_vector_f64_size(vec), TEST_VECTOR_SIZE);

	// Check elements
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		const double* val = ct_vector_f64_value(vec, i);
		assert_not_null(val);
		assert_true(fabs(*val - (i * 1.5)) < 1e-9);
	}

	// Pop elements
	for (int i = TEST_VECTOR_SIZE - 1; i >= 0; --i) {
		const double* back = ct_vector_f64_back(vec);
		assert_not_null(back);
		assert_true(fabs(*back - (i * 1.5)) < 1e-9);
		assert_uint32_eq(ct_vector_f64_size(vec), i + 1);
		ct_vector_f64_pop(vec);
	}
	assert_uint32_eq(ct_vector_f64_size(vec), 0);
	assert_true(ct_vector_f64_empty(vec));

	ct_vector_f64_destroy(vec);
}

static void test_vector_define_struct(void) {
	ct_vector_my_struct_t vec[1];
	ct_vector_my_struct_init(vec, 1);

	// Push elements
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		my_struct_t s;
		s.id = i;
		snprintf(s.name, sizeof(s.name), "item %d", i);
		assert_true(ct_vector_my_struct_push(vec, s));
	}
	assert_uint32_eq(ct_vector_my_struct_size(vec), TEST_VECTOR_SIZE);

	// Check elements
	for (int i = 0; i < TEST_VECTOR_SIZE; ++i) {
		const my_struct_t* s = ct_vector_my_struct_value(vec, i);
		assert_not_null(s);
		assert_int32_eq(s->id, i);
		char expected_name[20];
		snprintf(expected_name, sizeof(expected_name), "item %d", i);
		assert_true(strcmp(s->name, expected_name) == 0);
	}

	// Pop elements
	for (int i = TEST_VECTOR_SIZE - 1; i >= 0; --i) {
		ct_vector_my_struct_pop(vec);
	}
	assert_true(ct_vector_my_struct_empty(vec));

	ct_vector_my_struct_destroy(vec);
}

int main(void) {
	test_vector_define_init();
	cunit_println("Finish! test_vector_define_init();");

	test_vector_define_push_pop();
	cunit_println("Finish! test_vector_define_push_pop();");

	test_vector_define_access();
	cunit_println("Finish! test_vector_define_access();");

	test_vector_define_insert();
	cunit_println("Finish! test_vector_define_insert();");

	test_vector_define_erase();
	cunit_println("Finish! test_vector_define_erase();");

	test_vector_define_capacity_management();
	cunit_println("Finish! test_vector_define_capacity_management();");

	test_vector_define_double();
	cunit_println("Finish! test_vector_define_double();");

	test_vector_define_struct();
	cunit_println("Finish! test_vector_define_struct();");

	cunit_pass();
}
