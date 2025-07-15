/**
 * @file list_test.c
 * @brief 链表测试
 */
#include "coter/container/list.h"
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
	assert_true(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 0);

	my_struct_t node1, node2, node3;
	assert_ptr_eq(ct_list_entry(node1.list, my_struct_t, list), &node1);
	assert_ptr_eq(ct_list_entry(node2.list, my_struct_t, list), &node2);
	assert_ptr_eq(ct_list_entry(node3.list, my_struct_t, list), &node3);

	ct_list_append(head, node1.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 1);
	assert_true(ct_list_first(head) == node1.list);
	assert_true(ct_list_last(head) == node1.list);

	ct_list_append(head, node2.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);
	assert_true(ct_list_first(head) == node1.list);
	assert_true(ct_list_last(head) == node2.list);

	ct_list_append(head, node3.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 3);
	assert_true(ct_list_first(head) == node1.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node1.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);
	assert_true(ct_list_first(head) == node2.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node2.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 1);
	assert_true(ct_list_first(head) == node3.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node3.list);
	assert_true(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 0);

	ct_list_prepend(head, node3.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 1);
	assert_true(ct_list_first(head) == node3.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_prepend(head, node2.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);
	assert_true(ct_list_first(head) == node2.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_prepend(head, node1.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 3);
	assert_true(ct_list_first(head) == node1.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node1.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);
	assert_true(ct_list_first(head) == node2.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node2.list);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 1);
	assert_true(ct_list_first(head) == node3.list);
	assert_true(ct_list_last(head) == node3.list);

	ct_list_remove(node3.list);
	assert_true(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 0);
}

// 测试函数: 链表操作
static inline void test_list_operations(void) {
	ct_list_buf_t head, node1, node2, node3, node4;

	ct_list_init(head);
	assert_true(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 0);
	assert_ptr_eq(ct_list_first(head), head);
	assert_ptr_eq(ct_list_last(head), head);

	// 测试在空链表上操作
	ct_list_remove(head);
	assert_true(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 0);
	assert_ptr_eq(ct_list_first(head), head);
	assert_ptr_eq(ct_list_last(head), head);

	// 测试 ct_list_append 和 ct_list_before
	ct_list_append(head, node1);
	ct_list_before(node1, node2);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);
	assert_ptr_eq(ct_list_first(head), node2);
	assert_ptr_eq(ct_list_last(head), node1);

	// 测试 ct_list_after
	ct_list_after(node1, node3);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 3);
	assert_ptr_eq(ct_list_first(head), node2);
	assert_ptr_eq(ct_list_last(head), node3);

	// 测试 ct_list_remove
	ct_list_remove(node1);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);
	assert_ptr_eq(ct_list_first(head), node2);
	assert_ptr_eq(ct_list_last(head), node3);

	ct_list_remove(node3);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 1);
	assert_ptr_eq(ct_list_first(head), node2);
	assert_ptr_eq(ct_list_last(head), node2);

	ct_list_remove(node2);
	assert_true(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 0);
	assert_ptr_eq(ct_list_first(head), head);
	assert_ptr_eq(ct_list_last(head), head);

	// 测试复杂的插入和删除操作
	ct_list_append(head, node1);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 1);
	ct_list_after(node1, node2);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);
	ct_list_before(node1, node3);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 3);
	ct_list_after(node2, node4);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 4);

	assert_ptr_eq(ct_list_first(head), node3);
	assert_ptr_eq(ct_list_last(head), node4);

	ct_list_remove(node1);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 3);
	ct_list_remove(node4);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 2);

	assert_ptr_eq(ct_list_first(head), node3);
	assert_ptr_eq(ct_list_last(head), node2);

	ct_list_remove(node2);
	assert_false(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 1);
	assert_ptr_eq(ct_list_first(head), node3);
	assert_ptr_eq(ct_list_last(head), node3);

	ct_list_remove(node3);
	assert_true(ct_list_isempty(head));
	assert_uint32_eq(ct_list_size(head), 0);
	assert_ptr_eq(ct_list_first(head), head);
	assert_ptr_eq(ct_list_last(head), head);
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

	assert_false(ct_list_isempty(head1));
	assert_true(ct_list_isempty(head2));
	assert_uint32_eq(ct_list_size(head1), 4);
	assert_uint32_eq(ct_list_size(head2), 0);
	assert_ptr_eq(ct_list_first(head1), node3);
	assert_ptr_eq(ct_list_last(head1), node2);

	count = 0;
	ct_list_foreach (pos, head1) {
		switch (count) {
			case 0: assert_ptr_eq(pos, node3); break;
			case 1: assert_ptr_eq(pos, node4); break;
			case 2: assert_ptr_eq(pos, node1); break;
			case 3: assert_ptr_eq(pos, node2); break;
		}
		count++;
	}
	assert_int32_eq(count, 4);

	// 测试 ct_list_splice_next
	ct_list_splice_next(head1, head3);

	assert_false(ct_list_isempty(head1));
	assert_true(ct_list_isempty(head3));
	assert_uint32_eq(ct_list_size(head1), 6);
	assert_uint32_eq(ct_list_size(head3), 0);
	assert_ptr_eq(ct_list_first(head1), node3);
	assert_ptr_eq(ct_list_last(head1), node6);

	count = 0;
	ct_list_foreach (pos, head1) {
		switch (count) {
			case 0: assert_ptr_eq(pos, node3); break;
			case 1: assert_ptr_eq(pos, node4); break;
			case 2: assert_ptr_eq(pos, node1); break;
			case 3: assert_ptr_eq(pos, node2); break;
			case 4: assert_ptr_eq(pos, node5); break;
			case 5: assert_ptr_eq(pos, node6); break;
		}
		count++;
	}
	assert_int32_eq(count, 6);

	ct_list_remove(node1);
	ct_list_remove(node2);
	ct_list_remove(node3);
	ct_list_remove(node4);
	ct_list_remove(node5);
	ct_list_remove(node6);
	assert_true(ct_list_isempty(head1));

	// 测试边界情况: 空链表
	ct_list_buf_t empty_head;
	ct_list_init(empty_head);

	ct_list_splice_prev(head1, empty_head);
	assert_uint32_eq(ct_list_size(head1), 0);
	assert_uint32_eq(ct_list_size(empty_head), 0);

	ct_list_splice_next(head1, empty_head);
	assert_uint32_eq(ct_list_size(head1), 0);
	assert_uint32_eq(ct_list_size(empty_head), 0);
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
	assert_uint32_eq(count, 2);
}

// 测试函数: test_list_foreach_entry
static inline void test_list_foreach_entry(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	my_struct_t node1 = {.data = 1};
	ct_list_append(head, node1.list);
	assert_ptr_eq(ct_list_first(head), node1.list);
	assert_ptr_eq(ct_list_last(head), node1.list);

	my_struct_t node2 = {.data = 2};
	ct_list_append(head, node2.list);
	assert_ptr_eq(ct_list_first(head), node1.list);
	assert_ptr_eq(ct_list_last(head), node2.list);

	int sum = 0;
	ct_list_foreach_entry (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	assert_true(sum == 3);
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
	assert_true(sum == 3);

	sum = 0;
	pos = &node2;
	ct_list_foreach_entry_from (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	assert_true(sum == 2);

	sum = 0;
	pos = ct_list_entry(head, my_struct_t, list);
	ct_list_foreach_entry_from (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	assert_true(sum == 0);
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
	assert_true(sum == 3);
	assert_true(ct_list_isempty(head));
}

int main(void) {
	test_list_basic();
	cunit_println("Finish! test_list_init();");

	test_list_operations();
	cunit_println("Finish! test_list_operations();");

	test_list_splice();
	cunit_println("Finish! test_list_splice();");

	test_list_foreach();
	cunit_println("Finish! test_list_foreach();");

	test_list_foreach_entry();
	cunit_println("Finish! test_list_foreach_entry();");

	test_list_foreach_entry_from();
	cunit_println("Finish! test_list_foreach_entry_from();");

	test_list_foreach_entry_safe();
	cunit_println("Finish! test_list_foreach_entry_safe();");

	cunit_pass();
}
