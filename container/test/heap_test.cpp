/**
 * @file heap_test.cpp
 * @brief Pairing heap tests.
 */
#include "coter/container/heap.h"

#include <vector>

#include "coter/testing/doctest.h"


typedef struct {
    ct_heap_node_t node_;
    int            val_;
} my_item_t;

static int heap_item_cmp(const ct_heap_node_t* a, const ct_heap_node_t* b) {
    const my_item_t* ia = (const my_item_t*)a;
    const my_item_t* ib = (const my_item_t*)b;
    return ia->val_ - ib->val_;
}

TEST_CASE("heap intrusive basic" * doctest::test_suite("heap")) {
    ct_heap_t heap;
    ct_heap_init(&heap, heap_item_cmp);

    my_item_t items[5];
    int       vals[] = {100, 20, 50, 10, 30};
    for (int i = 0; i < 5; ++i) {
        items[i].val_ = vals[i];
        ct_heap_insert(&heap, &items[i].node_);
    }

    REQUIRE(ct_heap_size(&heap) == 5);
    REQUIRE(((my_item_t*)ct_heap_top(&heap))->val_ == 10);

    SUBCASE("Pop items") {
        int expected[] = {10, 20, 30, 50, 100};
        for (int i = 0; i < 5; ++i) {
            ct_heap_node_t* top = ct_heap_pop(&heap);
            REQUIRE(((my_item_t*)top)->val_ == expected[i]);
        }
        REQUIRE(ct_heap_is_empty(&heap));
    }

    SUBCASE("Remove arbitrary") {
        /* Remove 20 */
        ct_heap_remove(&heap, &items[1].node_);
        REQUIRE(ct_heap_size(&heap) == 4);

        int expected[] = {10, 30, 50, 100};
        for (int i = 0; i < 4; ++i) {
            ct_heap_node_t* top = ct_heap_pop(&heap);
            REQUIRE(((my_item_t*)top)->val_ == expected[i]);
        }
    }
}

TEST_CASE("heap update priority" * doctest::test_suite("heap")) {
    ct_heap_t heap;
    ct_heap_init(&heap, heap_item_cmp);

    my_item_t items[3];
    items[0].val_ = 50;
    items[1].val_ = 100;
    items[2].val_ = 150;

    for (int i = 0; i < 3; ++i) ct_heap_insert(&heap, &items[i].node_);

    REQUIRE(((my_item_t*)ct_heap_top(&heap))->val_ == 50);

    /* Update 150 to 10 */
    items[2].val_ = 10;
    ct_heap_update(&heap, &items[2].node_);

    REQUIRE(((my_item_t*)ct_heap_top(&heap))->val_ == 10);
    REQUIRE(ct_heap_pop(&heap) == &items[2].node_);
}

TEST_CASE("heap clear" * doctest::test_suite("heap")) {
    ct_heap_t heap;
    ct_heap_init(&heap, heap_item_cmp);

    my_item_t items[3];
    for (int i = 0; i < 3; ++i) {
        items[i].val_ = i;
        ct_heap_insert(&heap, &items[i].node_);
    }

    REQUIRE(ct_heap_size(&heap) == 3);
    ct_heap_clear(&heap);
    REQUIRE(ct_heap_size(&heap) == 0);
    REQUIRE(ct_heap_top(&heap) == nullptr);
}

TEST_CASE("heap move" * doctest::test_suite("heap")) {
    my_item_t items_a[3], items_b[3];
    for (int i = 0; i < 3; ++i) {
        items_a[i].val_ = (i + 1) * 10;      // 10, 20, 30
        items_b[i].val_ = (i + 1) * 10 + 5;  // 15, 25, 35
    }

    SUBCASE("Move to empty heap") {
        ct_heap_t heap_a, heap_b;
        ct_heap_init(&heap_a, heap_item_cmp);
        ct_heap_init(&heap_b, heap_item_cmp);

        for (int i = 0; i < 3; ++i) ct_heap_insert(&heap_b, &items_b[i].node_);

        ct_heap_move(&heap_a, &heap_b);

        REQUIRE(ct_heap_size(&heap_a) == 3);
        REQUIRE(ct_heap_size(&heap_b) == 0);
        REQUIRE(((my_item_t*)ct_heap_top(&heap_a))->val_ == 15);
    }

    SUBCASE("Merge two non-empty heaps") {
        ct_heap_t heap_a, heap_b;
        ct_heap_init(&heap_a, heap_item_cmp);
        ct_heap_init(&heap_b, heap_item_cmp);

        for (int i = 0; i < 3; ++i) {
            ct_heap_insert(&heap_a, &items_a[i].node_);
            ct_heap_insert(&heap_b, &items_b[i].node_);
        }

        ct_heap_move(&heap_a, &heap_b);

        REQUIRE(ct_heap_size(&heap_a) == 6);
        REQUIRE(ct_heap_size(&heap_b) == 0);

        int expected[] = {10, 15, 20, 25, 30, 35};
        for (int i = 0; i < 6; ++i) {
            ct_heap_node_t* top = ct_heap_pop(&heap_a);
            REQUIRE(((my_item_t*)top)->val_ == expected[i]);
        }
    }

    SUBCASE("Move from empty heap") {
        ct_heap_t heap_a, heap_b;
        ct_heap_init(&heap_a, heap_item_cmp);
        ct_heap_init(&heap_b, heap_item_cmp);

        for (int i = 0; i < 3; ++i) ct_heap_insert(&heap_a, &items_a[i].node_);

        ct_heap_move(&heap_a, &heap_b);

        REQUIRE(ct_heap_size(&heap_a) == 3);
        REQUIRE(ct_heap_size(&heap_b) == 0);
        REQUIRE(((my_item_t*)ct_heap_top(&heap_a))->val_ == 10);
    }
}
