/**
 * @file list_test.c
 * @brief 链表测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "container/ct_list.h"
#include "cunit.h"

// 测试函数: ct_list_foreach_entry
typedef struct my_struct {
	ct_list_buf_t list;
	int           data;
} my_struct_t;

// 测试函数: 链表基本操作
static inline void test_list_basic(void) {
	ct_list_buf_t head;
	ct_list_init(head);
	cunit_assert_true(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 0);

	my_struct_t node1, node2, node3;
	cunit_assert_pointer_equal(ct_list_entry(node1.list, my_struct_t, list), &node1);
	cunit_assert_pointer_equal(ct_list_entry(node2.list, my_struct_t, list), &node2);
	cunit_assert_pointer_equal(ct_list_entry(node3.list, my_struct_t, list), &node3);

	ct_list_append(head, node1.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 1);
	cunit_assert_true(ct_list_first(head) == node1.list);
	cunit_assert_true(ct_list_last(head) == node1.list);

	ct_list_append(head, node2.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);
	cunit_assert_true(ct_list_first(head) == node1.list);
	cunit_assert_true(ct_list_last(head) == node2.list);

	ct_list_append(head, node3.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 3);
	cunit_assert_true(ct_list_first(head) == node1.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node1.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);
	cunit_assert_true(ct_list_first(head) == node2.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node2.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 1);
	cunit_assert_true(ct_list_first(head) == node3.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node3.list);
	cunit_assert_true(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 0);

	ct_list_prepend(head, node3.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 1);
	cunit_assert_true(ct_list_first(head) == node3.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_prepend(head, node2.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);
	cunit_assert_true(ct_list_first(head) == node2.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_prepend(head, node1.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 3);
	cunit_assert_true(ct_list_first(head) == node1.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node1.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);
	cunit_assert_true(ct_list_first(head) == node2.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node2.list);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 1);
	cunit_assert_true(ct_list_first(head) == node3.list);
	cunit_assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node3.list);
	cunit_assert_true(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 0);
}

// 测试函数: 链表操作
static inline void test_list_operations(void) {
	ct_list_buf_t head, node1, node2, node3, node4;

	ct_list_init(head);
	cunit_assert_true(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 0);
	cunit_assert_pointer_equal(ct_list_first(head), head);
	cunit_assert_pointer_equal(ct_list_last(head), head);

	// 测试在空链表上操作
	ct_list_remove(head);
	cunit_assert_true(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 0);
	cunit_assert_pointer_equal(ct_list_first(head), head);
	cunit_assert_pointer_equal(ct_list_last(head), head);

	// 测试 ct_list_append 和 ct_list_before
	ct_list_append(head, node1);
	ct_list_before(node1, node2);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);
	cunit_assert_pointer_equal(ct_list_first(head), node2);
	cunit_assert_pointer_equal(ct_list_last(head), node1);

	// 测试 ct_list_after
	ct_list_after(node1, node3);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 3);
	cunit_assert_pointer_equal(ct_list_first(head), node2);
	cunit_assert_pointer_equal(ct_list_last(head), node3);

	// 测试 ct_list_remove
	ct_list_remove(node1);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);
	cunit_assert_pointer_equal(ct_list_first(head), node2);
	cunit_assert_pointer_equal(ct_list_last(head), node3);

	ct_list_remove(node3);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 1);
	cunit_assert_pointer_equal(ct_list_first(head), node2);
	cunit_assert_pointer_equal(ct_list_last(head), node2);

	ct_list_remove(node2);
	cunit_assert_true(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 0);
	cunit_assert_pointer_equal(ct_list_first(head), head);
	cunit_assert_pointer_equal(ct_list_last(head), head);

	// 测试复杂的插入和删除操作
	ct_list_append(head, node1);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 1);
	ct_list_after(node1, node2);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);
	ct_list_before(node1, node3);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 3);
	ct_list_after(node2, node4);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 4);

	cunit_assert_pointer_equal(ct_list_first(head), node3);
	cunit_assert_pointer_equal(ct_list_last(head), node4);

	ct_list_remove(node1);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 3);
	ct_list_remove(node4);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 2);

	cunit_assert_pointer_equal(ct_list_first(head), node3);
	cunit_assert_pointer_equal(ct_list_last(head), node2);

	ct_list_remove(node2);
	cunit_assert_false(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 1);
	cunit_assert_pointer_equal(ct_list_first(head), node3);
	cunit_assert_pointer_equal(ct_list_last(head), node3);

	ct_list_remove(node3);
	cunit_assert_true(ct_list_isempty(head));
	cunit_assert_uint32_equal(ct_list_size(head), 0);
	cunit_assert_pointer_equal(ct_list_first(head), head);
	cunit_assert_pointer_equal(ct_list_last(head), head);
}

// 测试函数: 链表拼接
static inline void test_list_splice(void) {
	ct_list_buf_t head1, head2, head3;
	ct_list_buf_t node1, node2, node3, node4, node5, node6;
	int           count;

	// 初始化链表
	ct_list_init(head1);
	ct_list_init(head2);
	ct_list_init(head3);

	// 准备测试数据
	ct_list_append(head1, node1);
	ct_list_append(head1, node2);
	ct_list_append(head2, node3);
	ct_list_append(head2, node4);
	ct_list_append(head3, node5);
	ct_list_append(head3, node6);

	// 测试 ct_list_splice_prev
	ct_list_splice_prev(head1, head2);

	cunit_assert_false(ct_list_isempty(head1));
	cunit_assert_true(ct_list_isempty(head2));
	cunit_assert_uint32_equal(ct_list_size(head1), 4);
	cunit_assert_uint32_equal(ct_list_size(head2), 0);
	cunit_assert_pointer_equal(ct_list_first(head1), node3);
	cunit_assert_pointer_equal(ct_list_last(head1), node2);

	count = 0;
	ct_list_foreach (pos, head1) {
		switch (count) {
			case 0: cunit_assert_pointer_equal(pos, node3); break;
			case 1: cunit_assert_pointer_equal(pos, node4); break;
			case 2: cunit_assert_pointer_equal(pos, node1); break;
			case 3: cunit_assert_pointer_equal(pos, node2); break;
		}
		count++;
	}
	cunit_assert_int32_equal(count, 4);

	// 测试 ct_list_splice_next
	ct_list_splice_next(head1, head3);

	cunit_assert_false(ct_list_isempty(head1));
	cunit_assert_true(ct_list_isempty(head3));
	cunit_assert_uint32_equal(ct_list_size(head1), 6);
	cunit_assert_uint32_equal(ct_list_size(head3), 0);
	cunit_assert_pointer_equal(ct_list_first(head1), node3);
	cunit_assert_pointer_equal(ct_list_last(head1), node6);

	count = 0;
	ct_list_foreach (pos, head1) {
		switch (count) {
			case 0: cunit_assert_pointer_equal(pos, node3); break;
			case 1: cunit_assert_pointer_equal(pos, node4); break;
			case 2: cunit_assert_pointer_equal(pos, node1); break;
			case 3: cunit_assert_pointer_equal(pos, node2); break;
			case 4: cunit_assert_pointer_equal(pos, node5); break;
			case 5: cunit_assert_pointer_equal(pos, node6); break;
		}
		count++;
	}
	cunit_assert_int32_equal(count, 6);

	ct_list_remove(node1);
	ct_list_remove(node2);
	ct_list_remove(node3);
	ct_list_remove(node4);
	ct_list_remove(node5);
	ct_list_remove(node6);
	cunit_assert_true(ct_list_isempty(head1));

	// 测试边界情况: 空链表
	ct_list_buf_t empty_head;
	ct_list_init(empty_head);

	ct_list_splice_prev(head1, empty_head);
	cunit_assert_uint32_equal(ct_list_size(head1), 0);
	cunit_assert_uint32_equal(ct_list_size(empty_head), 0);

	ct_list_splice_next(head1, empty_head);
	cunit_assert_uint32_equal(ct_list_size(head1), 0);
	cunit_assert_uint32_equal(ct_list_size(empty_head), 0);
}

// 测试函数: ct_list_foreach
static inline void test_list_foreach(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	ct_list_buf_t node1;
	ct_list_append(head, node1);

	ct_list_buf_t node2;
	ct_list_append(head, node2);

	int count = 0;
	ct_list_foreach (ptr, head) {
		count++;
	}
	cunit_assert_uint32_equal(count, 2);
}

// 测试函数: test_list_foreach_entry
static inline void test_list_foreach_entry(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	my_struct_t node1 = {.data = 1};
	ct_list_append(head, node1.list);
	cunit_assert_pointer_equal(ct_list_first(head), node1.list);
	cunit_assert_pointer_equal(ct_list_last(head), node1.list);

	my_struct_t node2 = {.data = 2};
	ct_list_append(head, node2.list);
	cunit_assert_pointer_equal(ct_list_first(head), node1.list);
	cunit_assert_pointer_equal(ct_list_last(head), node2.list);

	int sum = 0;
	ct_list_foreach_entry (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	cunit_assert_true(sum == 3);
}

// 测试函数: ct_list_foreach_entry_from
static inline void test_list_foreach_entry_from(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	my_struct_t node1 = {.data = 1};
	ct_list_append(head, node1.list);

	my_struct_t node2 = {.data = 2};
	ct_list_append(head, node2.list);

	int          sum = 0;
	my_struct_t *pos = NULL;

	sum = 0;
	pos = &node1;
	ct_list_foreach_entry_from (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	cunit_assert_true(sum == 3);

	sum = 0;
	pos = &node2;
	ct_list_foreach_entry_from (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	cunit_assert_true(sum == 2);

	sum = 0;
	pos = ct_list_entry(head, my_struct_t, list);
	ct_list_foreach_entry_from (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	cunit_assert_true(sum == 0);
}

// 测试函数: ct_list_foreach_entry_safe
static inline void test_list_foreach_entry_safe(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	my_struct_t node1 = {.data = 1};
	ct_list_append(head, node1.list);

	my_struct_t node2 = {.data = 2};
	ct_list_append(head, node2.list);

	int sum = 0;
	ct_list_foreach_entry_safe (pos, head, my_struct_t, list) {
		sum += pos->data;
		ct_list_remove(pos->list);
	}
	cunit_assert_true(sum == 3);
	cunit_assert_true(ct_list_isempty(head));
}

int main(void) {
	test_list_basic();
	cunit_println("Finish! test_list_init();\n");

	test_list_operations();
	cunit_println("Finish! test_list_operations();\n");

	test_list_splice();
	cunit_println("Finish! test_list_splice();\n");

	test_list_foreach();
	cunit_println("Finish! test_list_foreach();\n");

	test_list_foreach_entry();
	cunit_println("Finish! test_list_foreach_entry();\n");

	test_list_foreach_entry_from();
	cunit_println("Finish! test_list_foreach_entry_from();\n");

	test_list_foreach_entry_safe();
	cunit_println("Finish! test_list_foreach_entry_safe();\n");

	cunit_pass();
}
