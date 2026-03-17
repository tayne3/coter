#include "coter/container/heap.h"

#include <catch.hpp>
#include <vector>

typedef struct {
	ct_heap_node_t node;
	int            val;
} my_item_t;

static int my_item_cmp(const ct_heap_node_t* a, const ct_heap_node_t* b) {
	const my_item_t* ia = (const my_item_t*)a;
	const my_item_t* ib = (const my_item_t*)b;
	return ia->val - ib->val;
}

TEST_CASE("Heap Intrusive Basic", "[heap]") {
	ct_heap_t heap;
	ct_heap_init(&heap, my_item_cmp);

	my_item_t items[5];
	int       vals[] = {100, 20, 50, 10, 30};
	for (int i = 0; i < 5; ++i) {
		items[i].val = vals[i];
		ct_heap_insert(&heap, &items[i].node);
	}

	REQUIRE(ct_heap_size(&heap) == 5);
	REQUIRE(((my_item_t*)ct_heap_top(&heap))->val == 10);

	SECTION("Pop items") {
		int expected[] = {10, 20, 30, 50, 100};
		for (int i = 0; i < 5; ++i) {
			ct_heap_node_t* top = ct_heap_pop(&heap);
			REQUIRE(((my_item_t*)top)->val == expected[i]);
		}
		REQUIRE(ct_heap_is_empty(&heap));
	}

	SECTION("Remove arbitrary") {
		/* Remove 20 */
		ct_heap_remove(&heap, &items[1].node);
		REQUIRE(ct_heap_size(&heap) == 4);

		int expected[] = {10, 30, 50, 100};
		for (int i = 0; i < 4; ++i) {
			ct_heap_node_t* top = ct_heap_pop(&heap);
			REQUIRE(((my_item_t*)top)->val == expected[i]);
		}
	}
}

TEST_CASE("Heap Update Priority", "[heap]") {
	ct_heap_t heap;
	ct_heap_init(&heap, my_item_cmp);

	my_item_t items[3];
	items[0].val = 50;
	items[1].val = 100;
	items[2].val = 150;

	for (int i = 0; i < 3; ++i) ct_heap_insert(&heap, &items[i].node);

	REQUIRE(((my_item_t*)ct_heap_top(&heap))->val == 50);

	/* Update 150 to 10 */
	items[2].val = 10;
	ct_heap_update(&heap, &items[2].node);

	REQUIRE(((my_item_t*)ct_heap_top(&heap))->val == 10);
	REQUIRE(ct_heap_pop(&heap) == &items[2].node);
}
