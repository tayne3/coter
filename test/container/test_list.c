/**
 * @file test_list.c
 * @brief 链表测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "container/ct_list.h"
#include "ctunit.h"

// 测试函数：ct_list_foreach_entry
typedef struct {
	ct_list_buf_t list;
	int           data;
} my_struct_t;

// 测试函数：ct_list_init
static inline int test_list_init(void);
// 测试函数：ct_list_isempty
static inline int test_list_isempty(void);
// 测试函数：ct_list_append
static inline int test_list_append(void);
// 测试函数：ct_list_prepend
static inline int test_list_prepend(void);
// 测试函数：ct_list_before
static inline int test_list_before(void);
// 测试函数：ct_list_after
static inline int test_list_after(void);
// 测试函数：ct_list_remove
static inline int test_list_remove(void);
// 测试函数：ct_list_splice_prev
static inline int test_list_splice_prev(void);
// 测试函数：ct_list_foreach
static inline int test_list_foreach(void);
static inline int test_list_foreach_entry(void);
// 测试函数：ct_list_foreach_entry_from
static inline int test_list_foreach_entry_from(void);
// 测试函数：ct_list_foreach_entry_safe
static inline int test_list_foreach_entry_safe(void);

int main(void) {
	test_list_init();
	ctunit_trace("Finish! test_list_init();\n");

	test_list_isempty();
	ctunit_trace("Finish! test_list_isempty();\n");

	test_list_append();
	ctunit_trace("Finish! test_list_append();\n");

	test_list_prepend();
	ctunit_trace("Finish! test_list_prepend();\n");

	test_list_before();
	ctunit_trace("Finish! test_list_before();\n");

	test_list_after();
	ctunit_trace("Finish! test_list_after();\n");

	test_list_remove();
	ctunit_trace("Finish! test_list_remove();\n");

	test_list_splice_prev();
	ctunit_trace("Finish! test_list_splice_prev();\n");

	test_list_foreach();
	ctunit_trace("Finish! test_list_foreach();\n");

	test_list_foreach_entry();
	ctunit_trace("Finish! test_list_foreach_entry();\n");

	test_list_foreach_entry_from();
	ctunit_trace("Finish! test_list_foreach_entry_from();\n");

	test_list_foreach_entry_safe();
	ctunit_trace("Finish! test_list_foreach_entry_safe();\n");

	ctunit_pass();
}

// 测试函数：ct_list_init
static inline int test_list_init(void) {
	ct_list_buf_t head;
	ct_list_init(head);
	ctunit_assert_true(ct_list_isempty(head));

	return 0;
}

// 测试函数：ct_list_isempty
static inline int test_list_isempty(void) {
	ct_list_buf_t head;
	ct_list_init(head);
	ctunit_assert_true(ct_list_isempty(head));

	ct_list_buf_t node;
	ct_list_append(head, node);
	ctunit_assert_true(!ct_list_isempty(head));

	return 0;
}

// 测试函数：ct_list_append
static inline int test_list_append(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	ct_list_buf_t node1;
	ct_list_append(head, node1);
	ctunit_assert_true(ct_list_first(head) == node1);
	ctunit_assert_true(ct_list_last(head) == node1);

	ct_list_buf_t node2;
	ct_list_append(head, node2);
	ctunit_assert_true(ct_list_first(head) == node1);
	ctunit_assert_true(ct_list_last(head) == node2);

	return 0;
}

// 测试函数：ct_list_prepend
static inline int test_list_prepend(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	ct_list_buf_t node1;
	ct_list_prepend(head, node1);
	ctunit_assert_true(ct_list_first(head) == node1);
	ctunit_assert_true(ct_list_last(head) == node1);

	ct_list_buf_t node2;
	ct_list_prepend(head, node2);
	ctunit_assert_true(ct_list_first(head) == node2);
	ctunit_assert_true(ct_list_last(head) == node1);

	return 0;
}

// 测试函数：ct_list_before
static inline int test_list_before(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	ct_list_buf_t node1;
	ct_list_append(head, node1);

	ct_list_buf_t node2;
	ct_list_before(node1, node2);

	ctunit_assert_pointer(ct_list_first(head), node2);
	ctunit_assert_pointer(ct_list_last(head), node1);

	return 0;
}

// 测试函数：ct_list_after
static inline int test_list_after(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	ct_list_buf_t node1;
	ct_list_append(head, node1);

	ct_list_buf_t node2;
	ct_list_after(node1, node2);

	ctunit_assert_pointer(ct_list_first(head), node1);
	ctunit_assert_pointer(ct_list_last(head), node2);

	return 0;
}

// 测试函数：ct_list_remove
static inline int test_list_remove(void) {
	ct_list_buf_t head;
	ct_list_init(head);
	ctunit_assert_true(ct_list_isempty(head));

	my_struct_t node1 = {.data = 1};
	ct_list_append(head, node1.list);
	ctunit_assert_true(!ct_list_isempty(head));

	my_struct_t node2 = {.data = 2};
	ct_list_append(head, node2.list);
	ctunit_assert_true(!ct_list_isempty(head));

	ct_list_remove(node1.list);
	ctunit_assert_true(!ct_list_isempty(head));

	ctunit_assert_pointer(ct_list_first(head), node2.list);
	ctunit_assert_pointer(ct_list_last(head), node2.list);

	ct_list_remove(node2.list);
	ctunit_assert_true(ct_list_isempty(head));

	ct_list_append(head, node1.list);
	ctunit_assert_true(!ct_list_isempty(head));
	ct_list_append(head, node2.list);
	ctunit_assert_true(!ct_list_isempty(head));

	ct_list_remove(node1.list);
	ctunit_assert_true(!ct_list_isempty(head));
	ct_list_remove(node2.list);
	ctunit_assert_true(ct_list_isempty(head));

	return 0;
}

// 测试函数：ct_list_splice_prev
static inline int test_list_splice_prev(void) {
	ct_list_buf_t head1;
	ct_list_init(head1);

	ct_list_buf_t node1;
	ct_list_append(head1, node1);

	ct_list_buf_t head2;
	ct_list_init(head2);

	ct_list_buf_t node2;
	ct_list_append(head2, node2);

	ct_list_splice_prev(head1, head2);

	ctunit_assert_pointer(ct_list_first(head1), node2);
	ctunit_assert_pointer(ct_list_last(head1), node1);

	return 0;
}

// 测试函数：ct_list_foreach
static inline int test_list_foreach(void) {
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
	ctunit_assert_true(count == 2);

	return 0;
}

static inline int test_list_foreach_entry(void) {
	ct_list_buf_t head;
	ct_list_init(head);

	my_struct_t node1 = {.data = 1};
	ct_list_append(head, node1.list);

	my_struct_t node2 = {.data = 2};
	ct_list_append(head, node2.list);

	int sum = 0;
	ct_list_foreach_entry (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	ctunit_assert_true(sum == 3);
	return 0;
}

// 测试函数：ct_list_foreach_entry_from
static inline int test_list_foreach_entry_from(void) {
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
	ctunit_assert_true(sum == 3);

	sum = 0;
	pos = &node2;
	ct_list_foreach_entry_from (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	ctunit_assert_true(sum == 2);

	sum = 0;
	pos = ct_list_entry(head, my_struct_t, list);
	ct_list_foreach_entry_from (pos, head, my_struct_t, list) {
		sum += pos->data;
	}
	ctunit_assert_true(sum == 0);

	return 0;
}

// 测试函数：ct_list_foreach_entry_safe
static inline int test_list_foreach_entry_safe(void) {
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
	ctunit_assert_true(sum == 3);
	ctunit_assert_true(ct_list_isempty(head));

	return 0;
}
