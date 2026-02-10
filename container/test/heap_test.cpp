#include "coter/container/heap.h"

#include <catch.hpp>

#include "coter/math/rand.h"

static inline ct_any_t any_int(int x) {
	ct_any_t v;
	v._d[0].i = x;
	v.type    = CTAny_TypeInt;
	return v;
}

static inline bool ct_heap_sort_func(const ct_any_t* a, const ct_any_t* b) {
	return ct_any_value_int(*a) <= ct_any_value_int(*b);
}

TEST_CASE("heap_init", "[heap]") {
	ct_heap_t    heap;
	ct_any_t     all[1000];
	const size_t max = sizeof(all) / sizeof(all[0]);
	for (size_t i = 1; i <= max; ++i) {
		ct_heap_init(&heap, all, i, ct_heap_sort_func);
		REQUIRE(ct_heap_max(&heap) == i);
		REQUIRE(ct_heap_size(&heap) == 0);
		REQUIRE(ct_heap_isempty(&heap));
		REQUIRE(!ct_heap_isfull(&heap));
	}
	ct_heap_clear(&heap);
	REQUIRE(ct_heap_max(&heap) == max);
	REQUIRE(ct_heap_size(&heap) == 0);
	REQUIRE(ct_heap_isempty(&heap));
	REQUIRE(!ct_heap_isfull(&heap));
}

TEST_CASE("heap_insert", "[heap]") {
	ct_heap_t    heap;
	ct_any_t     all[10];
	const size_t max = sizeof(all) / sizeof(all[0]);
	ct_heap_init(&heap, all, max, (ct_heap_sort_t)ct_heap_sort_func);
	int value = 0;
	for (size_t i = 0; i < max; ++i) {
		value = (int)(max - i);
		ct_heap_insert(&heap, any_int(value));
		REQUIRE(ct_heap_max(&heap) == max);
		REQUIRE(ct_heap_size(&heap) == i + 1);
		REQUIRE(!ct_heap_isempty(&heap));
		if (i + 1 >= max) {
			REQUIRE(ct_heap_isfull(&heap));
		} else {
			REQUIRE(!ct_heap_isfull(&heap));
		}
	}
	ct_heap_clear(&heap);
	REQUIRE(ct_heap_max(&heap) == max);
	REQUIRE(ct_heap_size(&heap) == 0);
	REQUIRE(ct_heap_isempty(&heap));
	REQUIRE(!ct_heap_isfull(&heap));
}

TEST_CASE("heap_remove", "[heap]") {
	ct_heap_t    heap;
	ct_any_t     all[10];
	const size_t max = sizeof(all) / sizeof(all[0]);
	ct_heap_init(&heap, all, max, (ct_heap_sort_t)ct_heap_sort_func);
	int         it = 0;
	ct_random_t state;
	ct_random_init(&state);
	for (size_t i = 0; i < max; ++i) {
		it = ct_random_int32(&state, -999, 999);
		ct_heap_insert(&heap, any_int(it));
	}
	int prev = 0;
	int next = 0;
	for (size_t i = 0; i < max; ++i) {
		ct_any_t first = heap._all[0];
		prev           = first._d[0].i;
		ct_heap_remove(&heap);
		REQUIRE(ct_heap_max(&heap) == max);
		REQUIRE(ct_heap_size(&heap) == max - i - 1);
		REQUIRE(!ct_heap_isfull(&heap));
		if (i + 1 >= max) {
			REQUIRE(ct_heap_isempty(&heap));
		} else {
			REQUIRE(!ct_heap_isempty(&heap));
			first = heap._all[0];
			next  = first._d[0].i;
			REQUIRE(prev <= next);
		}
	}
	ct_heap_clear(&heap);
	REQUIRE(ct_heap_max(&heap) == max);
	REQUIRE(ct_heap_size(&heap) == 0);
	REQUIRE(ct_heap_isempty(&heap));
	REQUIRE(!ct_heap_isfull(&heap));
}
