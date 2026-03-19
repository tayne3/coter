/**
 * @file atomic_test.cpp
 * @brief 原子操作测试
 */
#include "coter/sync/atomic.h"

#include <catch.hpp>

TEST_CASE("atomic flag can be set, tested, and cleared", "[atomic]") {
    ct_atomic_flag_t flag = CT_ATOMIC_FLAG_INIT;

    REQUIRE(!ct_atomic_flag_test_and_set(&flag));
    REQUIRE(ct_atomic_flag_test_and_set(&flag));
    REQUIRE(ct_atomic_flag_test_and_set(&flag));

    ct_atomic_flag_clear(&flag);

    REQUIRE(!ct_atomic_flag_test_and_set(&flag));
    REQUIRE(ct_atomic_flag_test_and_set(&flag));
}

TEST_CASE("atomic load and store operate correctly", "[atomic]") {
    ct_atomic_long_t val = CT_ATOMIC_VAR_INIT(42);
    REQUIRE(ct_atomic_long_load(&val) == 42);

    ct_atomic_long_store(&val, 100);
    REQUIRE(ct_atomic_long_load(&val) == 100);

    ct_atomic_long_store(&val, LONG_MAX);
    REQUIRE(ct_atomic_long_load(&val) == LONG_MAX);

    ct_atomic_long_store(&val, LONG_MIN);
    REQUIRE(ct_atomic_long_load(&val) == LONG_MIN);
}

TEST_CASE("atomic add and sub work in a single thread", "[atomic]") {
    ct_atomic_long_t val = CT_ATOMIC_VAR_INIT(0);
    REQUIRE(ct_atomic_long_load(&val) == 0);

    ct_atomic_long_add(&val, 10);
    REQUIRE(ct_atomic_long_load(&val) == 10);

    ct_atomic_long_sub(&val, 5);
    REQUIRE(ct_atomic_long_load(&val) == 5);

    ct_atomic_long_add(&val, -15);
    REQUIRE(ct_atomic_long_load(&val) == -10);

    ct_atomic_long_sub(&val, -20);
    REQUIRE(ct_atomic_long_load(&val) == 10);
}

TEST_CASE("atomic operations return the previous value", "[atomic]") {
    ct_atomic_long_t val = CT_ATOMIC_VAR_INIT(10);

    REQUIRE(ct_atomic_long_sub(&val, 1) == 10);
    REQUIRE(ct_atomic_long_load(&val) == 9);

    REQUIRE(ct_atomic_long_add(&val, 1) == 9);
    REQUIRE(ct_atomic_long_load(&val) == 10);

    REQUIRE(ct_atomic_long_sub(&val, 5) == 10);
    REQUIRE(ct_atomic_long_load(&val) == 5);

    REQUIRE(ct_atomic_long_add(&val, 5) == 5);
    REQUIRE(ct_atomic_long_load(&val) == 10);
}

TEST_CASE("atomic operations handle integer overflow", "[atomic]") {
    ct_atomic_long_t max_val = CT_ATOMIC_VAR_INIT(LONG_MAX);
    ct_atomic_long_add(&max_val, 1);
    REQUIRE(ct_atomic_long_load(&max_val) == LONG_MIN);

    ct_atomic_long_t min_val = CT_ATOMIC_VAR_INIT(LONG_MIN);
    ct_atomic_long_sub(&min_val, 1);
    REQUIRE(ct_atomic_long_load(&min_val) == LONG_MAX);
}

TEST_CASE("explicit atomic types work correctly", "[atomic]") {
    ct_atomic_int_t atomic_int = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_store(&atomic_int, 123);
    REQUIRE(ct_atomic_int_load(&atomic_int) == 123);

    ct_atomic_bool_t atomic_bool = CT_ATOMIC_VAR_INIT(false);
    ct_atomic_bool_store(&atomic_bool, true);
    REQUIRE(ct_atomic_bool_load(&atomic_bool));

    ct_atomic_uint_t atomic_uint = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_uint_store(&atomic_uint, 0xDEADBEEF);
    REQUIRE(ct_atomic_uint_load(&atomic_uint) == 0xDEADBEEF);

    ct_atomic_llong_t atomic_llong = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_llong_store(&atomic_llong, 9999999999LL);
    REQUIRE(ct_atomic_llong_load(&atomic_llong) == 9999999999LL);
}

TEST_CASE("atomic pointer load, store, and exchange", "[atomic]") {
    int             val1 = 10;
    int             val2 = 20;
    ct_atomic_ptr_t ptr  = CT_ATOMIC_VAR_INIT(&val1);

    REQUIRE(ct_atomic_ptr_load(&ptr) == &val1);

    ct_atomic_ptr_store(&ptr, &val2);
    REQUIRE(ct_atomic_ptr_load(&ptr) == &val2);

    void* old = ct_atomic_ptr_exchange(&ptr, &val1);
    REQUIRE(old == &val2);
    REQUIRE(ct_atomic_ptr_load(&ptr) == &val1);
}

TEST_CASE("atomic pointer compare exchange succeeds and fails correctly", "[atomic]") {
    int             val1 = 10;
    int             val2 = 20;
    int             val3 = 30;
    ct_atomic_ptr_t ptr  = CT_ATOMIC_VAR_INIT(&val1);

    void* expected = &val1;
    bool  success  = ct_atomic_ptr_compare_exchange(&ptr, &expected, &val2);
    REQUIRE(success);
    REQUIRE(ct_atomic_ptr_load(&ptr) == &val2);
    REQUIRE(expected == &val1);

    expected = &val3;
    success  = ct_atomic_ptr_compare_exchange(&ptr, &expected, &val1);
    REQUIRE(!success);
    REQUIRE(ct_atomic_ptr_load(&ptr) == &val2);
    REQUIRE(expected == &val2);
}

TEST_CASE("CAS allows ABA value reuse", "[atomic]") {
    int             val1 = 10;
    int             val2 = 20;
    ct_atomic_ptr_t ptr  = CT_ATOMIC_VAR_INIT(&val1);

    void* expected = &val1;

    ct_atomic_ptr_store(&ptr, &val2);
    ct_atomic_ptr_store(&ptr, &val1);

    int  val3    = 30;
    bool success = ct_atomic_ptr_compare_exchange(&ptr, &expected, &val3);

    REQUIRE(success);
    REQUIRE(ct_atomic_ptr_load(&ptr) == &val3);
}
