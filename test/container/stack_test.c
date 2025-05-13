/**
 * @file stack_test.c
 * @brief 栈测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "container/ct_stack.h"
#include "ctunit.h"

static inline int test_stack_init(void) {
	ct_stack_t   stack;
	ct_any_t     buffer[1000];
	const size_t max = ct_arrsize(buffer);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			// 初始化队列
			ct_stack_init(&stack, buffer, i);

			ctunit_assert_int_equal(ct_stack_max(&stack), i);
			ctunit_assert_int_equal(ct_stack_size(&stack), 0);
			ctunit_assert_true(ct_stack_isempty(&stack));
			ctunit_assert_true(!ct_stack_isfull(&stack));
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

			ctunit_assert_true(ct_stack_max(&stack) == max);
			ctunit_assert_true(ct_stack_size(&stack) == i);
			ctunit_assert_true(!ct_stack_isempty(&stack));
			if (i == max) {
				ctunit_assert_true(ct_stack_isfull(&stack));
			} else {
				ctunit_assert_true(!ct_stack_isfull(&stack));
			}
		}
	}

	ct_stack_clear(&stack);
	ctunit_assert_int_equal(ct_stack_max(&stack), max);
	ctunit_assert_int_equal(ct_stack_size(&stack), 0);
	ctunit_assert_true(ct_stack_isempty(&stack));
	ctunit_assert_true(!ct_stack_isfull(&stack));

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

			ctunit_assert_true(ct_any_value_int(item) == (int)(max - i + 1));
			ctunit_assert_true(ct_stack_max(&stack) == max);
			ctunit_assert_true(ct_stack_size(&stack) == max - i);
			ctunit_assert_true(!ct_stack_isfull(&stack));
			if (i == max) {
				ctunit_assert_true(ct_stack_isempty(&stack));
			} else {
				ctunit_assert_true(!ct_stack_isempty(&stack));
			}
		}
	}

	ct_stack_clear(&stack);
	ctunit_assert_int_equal(ct_stack_max(&stack), max);
	ctunit_assert_int_equal(ct_stack_size(&stack), 0);
	ctunit_assert_true(ct_stack_isempty(&stack));
	ctunit_assert_true(!ct_stack_isfull(&stack));

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

			ctunit_assert_int_equal(ct_any_value_int(item), max - i + 1);
			ctunit_assert_int_equal(ct_any_value_int(item_prev), max - i + 1);
			ctunit_assert_int_equal(ct_stack_max(&stack), max);
			ctunit_assert_int_equal(ct_stack_size(&stack), max - i);
			ctunit_assert_true(!ct_stack_isfull(&stack));
			if (i == max) {
				ctunit_assert_true(ct_stack_isempty(&stack));
			} else {
				ctunit_assert_true(!ct_stack_isempty(&stack));
				ct_stack_top(&stack, &item_next);
				ctunit_assert_int_equal(ct_any_value_int(item_next), max - i);
			}
		}
	}

	ct_stack_clear(&stack);
	ctunit_assert_int_equal(ct_stack_max(&stack), max);
	ctunit_assert_int_equal(ct_stack_size(&stack), 0);
	ctunit_assert_true(ct_stack_isempty(&stack));
	ctunit_assert_true(!ct_stack_isfull(&stack));

	return 0;
}

int main(void) {
	test_stack_init();
	ctunit_trace("Finish! test_stack_init();\n");

	test_stack_push();
	ctunit_trace("Finish! test_stack_push();\n");

	test_stack_pop();
	ctunit_trace("Finish! test_stack_pop();\n");

	test_stack_top();
	ctunit_trace("Finish! test_stack_top();\n");

	ctunit_pass();
}
