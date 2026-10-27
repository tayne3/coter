/**
 * @file vector_test.cpp
 * @brief Type-safe dynamic array tests.
 */
#include "coter/container/vector.h"

#include "coter/testing/doctest.h"

// Declare the generated type (IntVec -> IntVec_t).
CT_VEC_DECL(int, IntVec);

// Define the generated implementation.
CT_VEC_IMPL(int, IntVec);

TEST_CASE("vector init and destroy" * doctest::test_suite("vector")) {
    IntVec_t v;
    REQUIRE(IntVec_init(&v, 4) == 0);

    REQUIRE(IntVec_capacity(&v) >= 4);
    REQUIRE(IntVec_size(&v) == 0);
    REQUIRE(IntVec_empty(&v));

    IntVec_destroy(&v);
    REQUIRE(v.ptr_ == nullptr);

    REQUIRE(IntVec_init(nullptr, 10) == -1);
}

TEST_CASE("vector push and pop" * doctest::test_suite("vector")) {
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

TEST_CASE("vector resize and reserve" * doctest::test_suite("vector")) {
    IntVec_t v;
    IntVec_init(&v, 0);

    IntVec_reserve(&v, 100);
    REQUIRE(IntVec_capacity(&v) >= 100);

    IntVec_resize(&v, 5);
    REQUIRE(IntVec_size(&v) == 5);
    REQUIRE(*IntVec_at(&v, 4) == 0);  // zero initialized

    IntVec_destroy(&v);
}
