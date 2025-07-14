/**
 * @file stack_test.c
 * @brief 栈测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "container/ct_stack.h"
#include "cunit.h"

static inline int test_stack_init(void) {
	ct_stack_t   stack;
	ct_any_t     buffer[1000];
	const size_t max = ct_arrsize(buffer);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			// 初始化队列
			ct_stack_init(&stack, buffer, i);

			assert_int_eq(ct_stack_max(&stack), i);
			assert_int_eq(ct_stack_size(&stack), 0);
			assert_true(ct_stack_isempty(&stack));
			assert_true(!ct_stack_isfull(&stack));
		}
	}

	return 0;
}

static inline int test_stack_push(void) {
	ct_stack_t   stack;
	ct_any_t     buffer[777];
	const size_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_stack_init(&stack, buffer, max);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			ct_stack_push(&stack, CT_ANY_INT(i));

			assert_true(ct_stack_max(&stack) == max);
			assert_true(ct_stack_size(&stack) == i);
			assert_true(!ct_stack_isempty(&stack));
			if (i == max) {
				assert_true(ct_stack_isfull(&stack));
			} else {
				assert_true(!ct_stack_isfull(&stack));
			}
		}
	}

	ct_stack_clear(&stack);
	assert_int_eq(ct_stack_max(&stack), max);
	assert_int_eq(ct_stack_size(&stack), 0);
	assert_true(ct_stack_isempty(&stack));
	assert_true(!ct_stack_isfull(&stack));

	return 0;
}

static inline int test_stack_pop(void) {
	ct_stack_t   stack;
	ct_any_t     buffer[777];
	const size_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_stack_init(&stack, buffer, max);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			ct_stack_push(&stack, CT_ANY_INT(i));
		}
	}

	{
		size_t   i    = 0;
		ct_any_t item = CT_ANY_INIT_INVALID;
		for (i = 1; i <= max; i++) {
			ct_stack_pop(&stack, &item);

			assert_true(ct_any_value_int(item) == (int)(max - i + 1));
			assert_true(ct_stack_max(&stack) == max);
			assert_true(ct_stack_size(&stack) == max - i);
			assert_true(!ct_stack_isfull(&stack));
			if (i == max) {
				assert_true(ct_stack_isempty(&stack));
			} else {
				assert_true(!ct_stack_isempty(&stack));
			}
		}
	}

	ct_stack_clear(&stack);
	assert_int_eq(ct_stack_max(&stack), max);
	assert_int_eq(ct_stack_size(&stack), 0);
	assert_true(ct_stack_isempty(&stack));
	assert_true(!ct_stack_isfull(&stack));

	return 0;
}

static inline int test_stack_top(void) {
	ct_stack_t   stack;
	ct_any_t     buffer[777];
	const size_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_stack_init(&stack, buffer, max);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			ct_stack_push(&stack, CT_ANY_INT(i));
		}
	}

	{
		size_t   i         = 0;
		ct_any_t item_prev = CT_ANY_INIT_INVALID;
		ct_any_t item      = CT_ANY_INIT_INVALID;
		ct_any_t item_next = CT_ANY_INIT_INVALID;

		for (i = 1; i <= max; i++) {
			ct_stack_top(&stack, &item_prev);
			ct_stack_pop(&stack, &item);

			assert_int_eq(ct_any_value_int(item), max - i + 1);
			assert_int_eq(ct_any_value_int(item_prev), max - i + 1);
			assert_int_eq(ct_stack_max(&stack), max);
			assert_int_eq(ct_stack_size(&stack), max - i);
			assert_true(!ct_stack_isfull(&stack));
			if (i == max) {
				assert_true(ct_stack_isempty(&stack));
			} else {
				assert_true(!ct_stack_isempty(&stack));
				ct_stack_top(&stack, &item_next);
				assert_int_eq(ct_any_value_int(item_next), max - i);
			}
		}
	}

	ct_stack_clear(&stack);
	assert_int_eq(ct_stack_max(&stack), max);
	assert_int_eq(ct_stack_size(&stack), 0);
	assert_true(ct_stack_isempty(&stack));
	assert_true(!ct_stack_isfull(&stack));

	return 0;
}

int main(void) {
	test_stack_init();
	cunit_println("Finish! test_stack_init();");

	test_stack_push();
	cunit_println("Finish! test_stack_push();");

	test_stack_pop();
	cunit_println("Finish! test_stack_pop();");

	test_stack_top();
	cunit_println("Finish! test_stack_top();");

	cunit_pass();
}
