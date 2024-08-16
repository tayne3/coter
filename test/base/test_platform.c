/**
 * @file test_platform.c
 * @brief 平台相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "base/ct_platform.h"
#include "ctunit.h"

static inline void test_container_of(void);

int main(void) {
	test_container_of();
	ctunit_trace("Finish! test_container_of();\n");

	ctunit_pass();
}

static inline void test_container_of(void) {
	struct TestStruct {
		int    a;
		double b;
		char   c;
	};

	struct TestStruct  test_instance = {1, 2.0, 'c'};
	struct TestStruct *container_ptr = CONTAINER_OF(&test_instance.c, struct TestStruct, c);

	ctunit_assert_pointer(container_ptr, &test_instance, "original = %p, container = %p\n", (void *)&test_instance,
						  (void *)&container_ptr);
}
