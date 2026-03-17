#include "coter/container/pqueue.h"

#include <algorithm>
#include <catch.hpp>
#include <vector>

static int int_cmp(const void* a, const void* b) {
	int va = *(const int*)a;
	int vb = *(const int*)b;
	return va - vb;
}

TEST_CASE("PQueue Basic Operations", "[pqueue]") {
	ct_pqueue_t pq;
	ct_pqueue_init(&pq, int, int_cmp);

	SECTION("Push and Pop order") {
		int vals[] = {50, 10, 30, 20, 40};
		for (int v : vals) { REQUIRE(ct_pqueue_push(&pq, &v)); }

		REQUIRE(ct_pqueue_size(&pq) == 5);

		int out;
		REQUIRE(ct_pqueue_pop(&pq, &out));
		REQUIRE(out == 10);
		REQUIRE(ct_pqueue_pop(&pq, &out));
		REQUIRE(out == 20);
		REQUIRE(ct_pqueue_pop(&pq, &out));
		REQUIRE(out == 30);
		REQUIRE(ct_pqueue_pop(&pq, &out));
		REQUIRE(out == 40);
		REQUIRE(ct_pqueue_pop(&pq, &out));
		REQUIRE(out == 50);

		REQUIRE(ct_pqueue_is_empty(&pq));
	}

	SECTION("Top check") {
		int v = 100;
		ct_pqueue_push(&pq, &v);
		v = 50;
		ct_pqueue_push(&pq, &v);

		REQUIRE(*(int*)ct_pqueue_top(&pq) == 50);
	}

	ct_pqueue_destroy(&pq);
}

TEST_CASE("PQueue Static Mode", "[pqueue]") {
	ct_pqueue_t pq;
	int         buffer[10];
	ct_pqueue_init_s(&pq, int, buffer, 10, int_cmp);

	SECTION("Static push limit") {
		for (int i = 0; i < 10; ++i) {
			int v = 10 - i;
			REQUIRE(ct_pqueue_push(&pq, &v));
		}

		int v = 11;
		REQUIRE_FALSE(ct_pqueue_push(&pq, &v));
		REQUIRE(ct_pqueue_size(&pq) == 10);
	}

	ct_pqueue_destroy(&pq);
}

TEST_CASE("PQueue Alignment and Large Elements", "[pqueue]") {
	struct large {
		char data[500];
	};
	ct_pqueue_t pq;
	/* This should pass the 512 byte check */
	ct_pqueue_init(&pq, struct large, (ct_compare_cb)strcmp);
	REQUIRE(ct_pqueue_item_size(&pq) == 500);
	ct_pqueue_destroy(&pq);
	/* The following would fail compilation if uncommented:
	struct too_large { char data[600]; };
	ct_pqueue_init(&pq, struct too_large, (ct_compare_cb)strcmp);
	*/
}
