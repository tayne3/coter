/**
 * @file platform_test.c
 * @brief 平台相关测试
 */
#include "coter/base/platform.h"
#include "cunit.h"

static inline void test_offset_of(void) {
	struct TestStruct {
		char   a;
		double b;
		int    c;
		char   d[4];
		float  e;
	};

	// 计算对齐字节数
	size_t alignment = OFFSET_OF(struct TestStruct, b);

	// 测试案例1：第一个成员偏移量
	size_t offset_a = OFFSET_OF(struct TestStruct, a);
	assert_uint32_eq(offset_a, 0, "Case 1: OFFSET_OF(struct TestStruct, a) = %zu\n", offset_a);

	// 测试案例2：对齐后的成员偏移量
	size_t offset_b = OFFSET_OF(struct TestStruct, b);
	assert_uint32_eq(offset_b, alignment, "Case 2: OFFSET_OF(struct TestStruct, b) = %zu\n", offset_b);

	// 测试案例3：紧跟大型成员后的成员偏移量
	size_t offset_c = OFFSET_OF(struct TestStruct, c);
	assert_uint32_eq(offset_c, offset_b + sizeof(double), "Case 3: OFFSET_OF(struct TestStruct, c) = %zu\n",
							   offset_c);

	// 测试案例4：数组成员的偏移量
	size_t offset_d = OFFSET_OF(struct TestStruct, d);
	assert_uint32_eq(offset_d, offset_c + sizeof(int), "Case 4: OFFSET_OF(struct TestStruct, d) = %zu\n",
							   offset_d);

	// 测试案例5：最后一个成员的偏移量
	size_t offset_e = OFFSET_OF(struct TestStruct, e);
	assert_uint32_eq(offset_e, (offset_d + sizeof(char[4]) + alignment - 1) & ~(alignment - 1),
							   "Case 5: OFFSET_OF(struct TestStruct, e) = %zu\n", offset_e);

	// 测试案例6：验证结构体大小
	size_t struct_size   = sizeof(struct TestStruct);
	size_t expected_size = (offset_e + sizeof(float) + alignment - 1) & ~(alignment - 1);
	assert_uint32_eq(struct_size, expected_size, "Case 6: sizeof(struct TestStruct) = %zu\n", struct_size);

	// 测试案例7：数组元素的偏移量
	size_t offset_d2 = OFFSET_OF(struct TestStruct, d[2]);
	assert_uint32_eq(offset_d2, offset_d + 2, "Case 7: OFFSET_OF(struct TestStruct, d[2]) = %zu\n",
							   offset_d2);
}

static inline void test_container_of(void) {
	struct TestStruct {
		double a;
		int    b;
		char   c[4];
		float  d;
	};

	// 测试案例1：基本功能测试
	struct TestStruct  test_instance1 = {1.0, 2, {'a', 'b', 'c', 'd'}, 3.14f};
	struct TestStruct *container_ptr1 = CONTAINER_OF(&test_instance1.c, struct TestStruct, c);
	assert_ptr_eq(container_ptr1, &test_instance1, "Case 1: original = %p, container = %p\n",
								(void *)&test_instance1, (void *)container_ptr1);

	// 测试案例2：使用不同成员进行测试
	struct TestStruct *container_ptr2 = CONTAINER_OF(&test_instance1.b, struct TestStruct, b);
	assert_ptr_eq(container_ptr2, &test_instance1, "Case 2: original = %p, container = %p\n",
								(void *)&test_instance1, (void *)container_ptr2);

	// 测试案例3：使用数组中的元素
	struct TestStruct *container_ptr3 = CONTAINER_OF(&test_instance1.c[2], struct TestStruct, c[2]);
	assert_ptr_eq(container_ptr3, &test_instance1, "Case 3: original = %p, container = %p\n",
								(void *)&test_instance1, (void *)container_ptr3);

	// 测试案例4：验证成员值的正确性
	assert_double_eq(container_ptr1->a, 1.0, "Case 4: a = %f\n", container_ptr1->a);
	assert_int32_eq(container_ptr1->b, 2, "Case 4: b = %d\n", container_ptr1->b);
	assert_char(container_ptr1->c[0], 'a', "Case 4: c[0] = %c\n", container_ptr1->c[0]);
	assert_float_eq(container_ptr1->d, 3.14f, "Case 4: d = %f\n", container_ptr1->d);

	// 测试案例5：使用结构体数组
	struct TestStruct  test_array[3]  = {{1.1, 11, {'w', 'x', 'y', 'z'}, 1.1f},
										 {2.2, 22, {'a', 'b', 'c', 'd'}, 2.2f},
										 {3.3, 33, {'m', 'n', 'o', 'p'}, 3.3f}};
	struct TestStruct *container_ptr5 = CONTAINER_OF(&test_array[1].c, struct TestStruct, c);
	assert_ptr_eq(container_ptr5, &test_array[1], "Case 5: original = %p, container = %p\n",
								(void *)&test_array[1], (void *)container_ptr5);
}

int main(void) {
	test_offset_of();
	cunit_println("Finish! test_offset_of();");

	test_container_of();
	cunit_println("Finish! test_container_of();");

	cunit_pass();
}
