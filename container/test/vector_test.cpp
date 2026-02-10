#include "coter/container/vector.h"

#include <catch.hpp>

// 声明类型 (IntVec -> IntVec_t)
CT_VEC_DECL(int, IntVec);

// 定义实现
CT_VEC_IMPL(int, IntVec);

TEST_CASE("Vec Init and Destroy", "[vec]") {
	IntVec_t v;
	REQUIRE(IntVec_init(&v, 4) == 0);

	REQUIRE(IntVec_capacity(&v) >= 4);
	REQUIRE(IntVec_size(&v) == 0);
	REQUIRE(IntVec_empty(&v));

	IntVec_destroy(&v);
	REQUIRE(v.ptr == nullptr);

	REQUIRE(IntVec_init(nullptr, 10) == -1);
}

TEST_CASE("Vec Push and Pop", "[vec]") {
	IntVec_t v;
	IntVec_init(&v, 0);

	int val = 10;
	IntVec_push(&v, &val);
	val = 20;
	IntVec_push(&v, &val);
	val = 30;
	IntVec_push(&v, &val);

	REQUIRE(IntVec_size(&v) == 3);
	REQUIRE(*IntVec_at(&v, 0) == 10);
	REQUIRE(*IntVec_at(&v, 1) == 20);
	REQUIRE(*IntVec_at(&v, 2) == 30);

	IntVec_pop(&v);
	REQUIRE(IntVec_size(&v) == 2);
	REQUIRE(*IntVec_back(&v) == 20);

	IntVec_destroy(&v);
}

TEST_CASE("Vec Resize and Reserve", "[vec]") {
	IntVec_t v;
	IntVec_init(&v, 0);

	IntVec_reserve(&v, 100);
	REQUIRE(IntVec_capacity(&v) >= 100);

	IntVec_resize(&v, 5);
	REQUIRE(IntVec_size(&v) == 5);
	REQUIRE(*IntVec_at(&v, 4) == 0);  // zero initialized

	IntVec_destroy(&v);
}
