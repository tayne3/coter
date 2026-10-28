/**
 * @file list_test.cpp
 * @brief Intrusive linked list tests.
 */
#include "coter/container/list.h"

#include <cstring>

#include "coter/testing/doctest.h"

typedef struct my_struct {
    ct_list_t list_[1];
    int       data_;
} my_struct_t;

TEST_CASE("list_basic" * doctest::test_suite("list")) {
    ct_list_t head[1];
    ct_list_init(head);
    REQUIRE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 0);
    my_struct_t node1, node2, node3;
    REQUIRE(ct_list_entry(node1.list_, my_struct_t, list_) == &node1);
    REQUIRE(ct_list_entry(node2.list_, my_struct_t, list_) == &node2);
    REQUIRE(ct_list_entry(node3.list_, my_struct_t, list_) == &node3);
    ct_list_append(head, node1.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 1);
    REQUIRE(ct_list_first(head) == node1.list_);
    REQUIRE(ct_list_last(head) == node1.list_);
    ct_list_append(head, node2.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    REQUIRE(ct_list_first(head) == node1.list_);
    REQUIRE(ct_list_last(head) == node2.list_);
    ct_list_append(head, node3.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 3);
    REQUIRE(ct_list_first(head) == node1.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_remove(node1.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    REQUIRE(ct_list_first(head) == node2.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_remove(node2.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 1);
    REQUIRE(ct_list_first(head) == node3.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_remove(node3.list_);
    REQUIRE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 0);
    ct_list_prepend(head, node3.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 1);
    REQUIRE(ct_list_first(head) == node3.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_prepend(head, node2.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    REQUIRE(ct_list_first(head) == node2.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_prepend(head, node1.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 3);
    REQUIRE(ct_list_first(head) == node1.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_remove(node1.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    REQUIRE(ct_list_first(head) == node2.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_remove(node2.list_);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 1);
    REQUIRE(ct_list_first(head) == node3.list_);
    REQUIRE(ct_list_last(head) == node3.list_);
    ct_list_remove(node3.list_);
    REQUIRE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 0);
}

TEST_CASE("list_operations" * doctest::test_suite("list")) {
    ct_list_t head[1], node1[1], node2[1], node3[1], node4[1];
    ct_list_init(head);
    REQUIRE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 0);
    REQUIRE(ct_list_first(head) == head);
    REQUIRE(ct_list_last(head) == head);
    ct_list_remove(head);
    REQUIRE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 0);
    REQUIRE(ct_list_first(head) == head);
    REQUIRE(ct_list_last(head) == head);
    ct_list_append(head, node1);
    ct_list_before(node1, node2);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    REQUIRE(ct_list_first(head) == node2);
    REQUIRE(ct_list_last(head) == node1);
    ct_list_after(node1, node3);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 3);
    REQUIRE(ct_list_first(head) == node2);
    REQUIRE(ct_list_last(head) == node3);
    ct_list_remove(node1);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    REQUIRE(ct_list_first(head) == node2);
    REQUIRE(ct_list_last(head) == node3);
    ct_list_remove(node3);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 1);
    REQUIRE(ct_list_first(head) == node2);
    REQUIRE(ct_list_last(head) == node2);
    ct_list_remove(node2);
    REQUIRE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 0);
    REQUIRE(ct_list_first(head) == head);
    REQUIRE(ct_list_last(head) == head);
    ct_list_append(head, node1);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 1);
    ct_list_after(node1, node2);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    ct_list_before(node1, node3);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 3);
    ct_list_after(node2, node4);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 4);
    REQUIRE(ct_list_first(head) == node3);
    REQUIRE(ct_list_last(head) == node4);
    ct_list_remove(node1);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 3);
    ct_list_remove(node4);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 2);
    REQUIRE(ct_list_first(head) == node3);
    REQUIRE(ct_list_last(head) == node2);
    ct_list_remove(node2);
    REQUIRE_FALSE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 1);
    REQUIRE(ct_list_first(head) == node3);
    REQUIRE(ct_list_last(head) == node3);
    ct_list_remove(node3);
    REQUIRE(ct_list_is_empty(head));
    REQUIRE(ct_list_size(head) == 0);
    REQUIRE(ct_list_first(head) == head);
    REQUIRE(ct_list_last(head) == head);
}

TEST_CASE("list_splice" * doctest::test_suite("list")) {
    ct_list_t head1[1], head2[1], head3[1];
    ct_list_t node1[1], node2[1], node3[1], node4[1], node5[1], node6[1];
    int       count;
    ct_list_init(head1);
    ct_list_init(head2);
    ct_list_init(head3);
    ct_list_append(head1, node1);
    ct_list_append(head1, node2);
    ct_list_append(head2, node3);
    ct_list_append(head2, node4);
    ct_list_append(head3, node5);
    ct_list_append(head3, node6);
    ct_list_splice_prev(head1, head2);
    REQUIRE_FALSE(ct_list_is_empty(head1));
    REQUIRE(ct_list_is_empty(head2));
    REQUIRE(ct_list_size(head1) == 4);
    REQUIRE(ct_list_size(head2) == 0);
    REQUIRE(ct_list_first(head1) == node3);
    REQUIRE(ct_list_last(head1) == node2);
    count = 0;
    ct_list_foreach(pos, head1) {
        switch (count) {
            case 0: REQUIRE(pos == node3); break;
            case 1: REQUIRE(pos == node4); break;
            case 2: REQUIRE(pos == node1); break;
            case 3: REQUIRE(pos == node2); break;
        }
        ++count;
    }
    REQUIRE(count == 4);
    ct_list_splice_next(head1, head3);
    REQUIRE_FALSE(ct_list_is_empty(head1));
    REQUIRE(ct_list_is_empty(head3));
    REQUIRE(ct_list_size(head1) == 6);
    REQUIRE(ct_list_size(head3) == 0);
    REQUIRE(ct_list_first(head1) == node3);
    REQUIRE(ct_list_last(head1) == node6);
    count = 0;
    ct_list_foreach(pos, head1) {
        switch (count) {
            case 0: REQUIRE(pos == node3); break;
            case 1: REQUIRE(pos == node4); break;
            case 2: REQUIRE(pos == node1); break;
            case 3: REQUIRE(pos == node2); break;
            case 4: REQUIRE(pos == node5); break;
            case 5: REQUIRE(pos == node6); break;
        }
        ++count;
    }
    REQUIRE(count == 6);
    ct_list_remove(node1);
    ct_list_remove(node2);
    ct_list_remove(node3);
    ct_list_remove(node4);
    ct_list_remove(node5);
    ct_list_remove(node6);
    REQUIRE(ct_list_is_empty(head1));
    ct_list_t empty_head[1];
    ct_list_init(empty_head);
    ct_list_splice_prev(head1, empty_head);
    REQUIRE(ct_list_size(head1) == 0);
    REQUIRE(ct_list_size(empty_head) == 0);
    ct_list_splice_next(head1, empty_head);
    REQUIRE(ct_list_size(head1) == 0);
    REQUIRE(ct_list_size(empty_head) == 0);
}

TEST_CASE("list_foreach" * doctest::test_suite("list")) {
    ct_list_t head[1];
    ct_list_init(head);
    ct_list_t node1[1];
    ct_list_append(head, node1);
    ct_list_t node2[1];
    ct_list_append(head, node2);
    int count = 0;
    ct_list_foreach(ptr, head) {
        ++count;
    }
    REQUIRE(count == 2);
}

TEST_CASE("list_foreach_entry" * doctest::test_suite("list")) {
    ct_list_t head[1];
    ct_list_init(head);
    my_struct_t node1;
    node1.data_ = 1;
    ct_list_append(head, node1.list_);
    REQUIRE(ct_list_first(head) == node1.list_);
    REQUIRE(ct_list_last(head) == node1.list_);
    my_struct_t node2;
    node2.data_ = 2;
    ct_list_append(head, node2.list_);
    REQUIRE(ct_list_first(head) == node1.list_);
    REQUIRE(ct_list_last(head) == node2.list_);
    int sum = 0;
    ct_list_foreach_entry(pos, head, my_struct_t, list_) {
        sum += pos->data_;
    }
    REQUIRE(sum == 3);
}

TEST_CASE("list_foreach_entry_from" * doctest::test_suite("list")) {
    ct_list_t head[1];
    ct_list_init(head);
    my_struct_t node1;
    node1.data_ = 1;
    ct_list_append(head, node1.list_);
    my_struct_t node2;
    node2.data_ = 2;
    ct_list_append(head, node2.list_);
    int          sum = 0;
    my_struct_t* pos = nullptr;
    sum              = 0;
    pos              = &node1;
    ct_list_foreach_entry_from(pos, head, my_struct_t, list_) {
        sum += pos->data_;
    }
    REQUIRE(sum == 3);
    sum = 0;
    pos = &node2;
    ct_list_foreach_entry_from(pos, head, my_struct_t, list_) {
        sum += pos->data_;
    }
    REQUIRE(sum == 2);
    sum = 0;
    pos = ct_list_entry(head, my_struct_t, list_);
    ct_list_foreach_entry_from(pos, head, my_struct_t, list_) {
        sum += pos->data_;
    }
    REQUIRE(sum == 0);
}

TEST_CASE("list_foreach_entry_safe" * doctest::test_suite("list")) {
    ct_list_t head[1];
    ct_list_init(head);
    my_struct_t node1;
    node1.data_ = 1;
    ct_list_append(head, node1.list_);
    my_struct_t node2;
    node2.data_ = 2;
    ct_list_append(head, node2.list_);
    int sum = 0;
    ct_list_foreach_entry_safe(pos, head, my_struct_t, list_) {
        sum += pos->data_;
        ct_list_remove(pos->list_);
    }
    REQUIRE(sum == 3);
    REQUIRE(ct_list_is_empty(head));
}
