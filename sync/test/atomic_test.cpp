/**
 * @file atomic_test.cpp
 * @brief 原子操作测试
 */
#include "coter/sync/atomic.h"

#include <climits>

#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("atomic");

TEST_CASE("atomic flag can be set, tested, and cleared") {
    ct_atomic_flag_t flag = CT_ATOMIC_FLAG_INIT;

    REQUIRE(!ct_atomic_flag_test_and_set(&flag));
    REQUIRE(ct_atomic_flag_test_and_set(&flag));
    REQUIRE(ct_atomic_flag_test_and_set(&flag));

    ct_atomic_flag_clear(&flag);

    REQUIRE(!ct_atomic_flag_test_and_set(&flag));
    REQUIRE(ct_atomic_flag_test_and_set(&flag));
}

TEST_CASE("atomic load and store operate correctly") {
    ct_atomic_long_t val = CT_ATOMIC_VAR_INIT(42);
    REQUIRE(ct_atomic_long_load(&val) == 42);

    ct_atomic_long_store(&val, 100);
    REQUIRE(ct_atomic_long_load(&val) == 100);

    ct_atomic_long_store(&val, LONG_MAX);
    REQUIRE(ct_atomic_long_load(&val) == LONG_MAX);

    ct_atomic_long_store(&val, LONG_MIN);
    REQUIRE(ct_atomic_long_load(&val) == LONG_MIN);
}

TEST_CASE("atomic add and sub work in a single thread") {
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

TEST_CASE("atomic operations return the previous value") {
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

TEST_CASE("atomic operations handle integer overflow") {
    ct_atomic_long_t max_val = CT_ATOMIC_VAR_INIT(LONG_MAX);
    ct_atomic_long_add(&max_val, 1);
    REQUIRE(ct_atomic_long_load(&max_val) == LONG_MIN);

    ct_atomic_long_t min_val = CT_ATOMIC_VAR_INIT(LONG_MIN);
    ct_atomic_long_sub(&min_val, 1);
    REQUIRE(ct_atomic_long_load(&min_val) == LONG_MAX);
}

TEST_CASE("atomic scalar types load and store correctly") {
    SUBCASE("boolean") {
        ct_atomic_bool_t value = CT_ATOMIC_VAR_INIT(false);
        ct_atomic_bool_store(&value, true);
        REQUIRE(ct_atomic_bool_load(&value));
    }

    SUBCASE("narrow signed integers") {
        ct_atomic_char_t  character        = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_schar_t signed_character = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_short_t short_integer    = CT_ATOMIC_VAR_INIT(0);

        ct_atomic_char_store(&character, 'x');
        ct_atomic_schar_store(&signed_character, -42);
        ct_atomic_short_store(&short_integer, -1234);

        REQUIRE(ct_atomic_char_load(&character) == 'x');
        REQUIRE(ct_atomic_schar_load(&signed_character) == -42);
        REQUIRE(ct_atomic_short_load(&short_integer) == -1234);
    }

    SUBCASE("unsigned integers") {
        ct_atomic_uchar_t  unsigned_character = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_ushort_t unsigned_short     = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_uint_t   unsigned_integer   = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_ulong_t  unsigned_long      = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_ullong_t unsigned_long_long = CT_ATOMIC_VAR_INIT(0);

        ct_atomic_uchar_store(&unsigned_character, 200U);
        ct_atomic_ushort_store(&unsigned_short, 60000U);
        ct_atomic_uint_store(&unsigned_integer, 0xDEADBEEFU);
        ct_atomic_ulong_store(&unsigned_long, 123456789UL);
        ct_atomic_ullong_store(&unsigned_long_long, 9999999999ULL);

        REQUIRE(ct_atomic_uchar_load(&unsigned_character) == 200U);
        REQUIRE(ct_atomic_ushort_load(&unsigned_short) == 60000U);
        REQUIRE(ct_atomic_uint_load(&unsigned_integer) == 0xDEADBEEFU);
        REQUIRE(ct_atomic_ulong_load(&unsigned_long) == 123456789UL);
        REQUIRE(ct_atomic_ullong_load(&unsigned_long_long) == 9999999999ULL);
    }

    SUBCASE("wide signed integers") {
        ct_atomic_int_t   integer           = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_long_t  long_integer      = CT_ATOMIC_VAR_INIT(0);
        ct_atomic_llong_t long_long_integer = CT_ATOMIC_VAR_INIT(0);

        ct_atomic_int_store(&integer, 123);
        ct_atomic_long_store(&long_integer, -123456789L);
        ct_atomic_llong_store(&long_long_integer, 9999999999LL);

        REQUIRE(ct_atomic_int_load(&integer) == 123);
        REQUIRE(ct_atomic_long_load(&long_integer) == -123456789L);
        REQUIRE(ct_atomic_llong_load(&long_long_integer) == 9999999999LL);
    }
}

TEST_CASE("atomic pointer load, store, and exchange") {
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

TEST_CASE("atomic pointer compare exchange succeeds and fails correctly") {
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

TEST_CASE("CAS allows ABA value reuse") {
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

TEST_CASE("atomic exchange returns the previous scalar value") {
    SUBCASE("boolean") {
        ct_atomic_bool_t value = CT_ATOMIC_VAR_INIT(false);
        REQUIRE_FALSE(ct_atomic_bool_exchange(&value, true));
        REQUIRE(ct_atomic_bool_load(&value));
    }

    SUBCASE("signed integers") {
        ct_atomic_int_t   integer           = CT_ATOMIC_VAR_INIT(10);
        ct_atomic_long_t  long_integer      = CT_ATOMIC_VAR_INIT(100L);
        ct_atomic_llong_t long_long_integer = CT_ATOMIC_VAR_INIT(1000LL);

        REQUIRE(ct_atomic_int_exchange(&integer, 20) == 10);
        REQUIRE(ct_atomic_long_exchange(&long_integer, 200L) == 100L);
        REQUIRE(ct_atomic_llong_exchange(&long_long_integer, 2000LL) == 1000LL);
        REQUIRE(ct_atomic_int_load(&integer) == 20);
        REQUIRE(ct_atomic_long_load(&long_integer) == 200L);
        REQUIRE(ct_atomic_llong_load(&long_long_integer) == 2000LL);
    }

    SUBCASE("unsigned integers") {
        ct_atomic_uint_t   integer           = CT_ATOMIC_VAR_INIT(10U);
        ct_atomic_ulong_t  long_integer      = CT_ATOMIC_VAR_INIT(100UL);
        ct_atomic_ullong_t long_long_integer = CT_ATOMIC_VAR_INIT(1000ULL);

        REQUIRE(ct_atomic_uint_exchange(&integer, 20U) == 10U);
        REQUIRE(ct_atomic_ulong_exchange(&long_integer, 200UL) == 100UL);
        REQUIRE(ct_atomic_ullong_exchange(&long_long_integer, 2000ULL) == 1000ULL);
        REQUIRE(ct_atomic_uint_load(&integer) == 20U);
        REQUIRE(ct_atomic_ulong_load(&long_integer) == 200UL);
        REQUIRE(ct_atomic_ullong_load(&long_long_integer) == 2000ULL);
    }
}

TEST_CASE("atomic compare exchange updates expected on failure") {
    SUBCASE("boolean") {
        ct_atomic_bool_t value    = CT_ATOMIC_VAR_INIT(false);
        bool             expected = false;
        REQUIRE(ct_atomic_bool_compare_exchange(&value, &expected, true));
        REQUIRE(ct_atomic_bool_load(&value));

        expected = false;
        REQUIRE_FALSE(ct_atomic_bool_compare_exchange(&value, &expected, false));
        REQUIRE(expected);
        REQUIRE(ct_atomic_bool_load(&value));
    }

    SUBCASE("signed integers") {
        ct_atomic_int_t   integer            = CT_ATOMIC_VAR_INIT(10);
        ct_atomic_long_t  long_integer       = CT_ATOMIC_VAR_INIT(100L);
        ct_atomic_llong_t long_long_integer  = CT_ATOMIC_VAR_INIT(1000LL);
        int               expected_integer   = 10;
        long              expected_long      = 100L;
        long long         expected_long_long = 1000LL;

        REQUIRE(ct_atomic_int_compare_exchange(&integer, &expected_integer, 20));
        REQUIRE(ct_atomic_long_compare_exchange(&long_integer, &expected_long, 200L));
        REQUIRE(ct_atomic_llong_compare_exchange(&long_long_integer, &expected_long_long, 2000LL));

        expected_integer   = 10;
        expected_long      = 100L;
        expected_long_long = 1000LL;
        REQUIRE_FALSE(ct_atomic_int_compare_exchange(&integer, &expected_integer, 30));
        REQUIRE_FALSE(ct_atomic_long_compare_exchange(&long_integer, &expected_long, 300L));
        REQUIRE_FALSE(ct_atomic_llong_compare_exchange(&long_long_integer, &expected_long_long, 3000LL));
        REQUIRE(expected_integer == 20);
        REQUIRE(expected_long == 200L);
        REQUIRE(expected_long_long == 2000LL);
        REQUIRE(ct_atomic_int_load(&integer) == 20);
        REQUIRE(ct_atomic_long_load(&long_integer) == 200L);
        REQUIRE(ct_atomic_llong_load(&long_long_integer) == 2000LL);
    }

    SUBCASE("unsigned integers") {
        ct_atomic_uint_t   integer            = CT_ATOMIC_VAR_INIT(10U);
        ct_atomic_ulong_t  long_integer       = CT_ATOMIC_VAR_INIT(100UL);
        ct_atomic_ullong_t long_long_integer  = CT_ATOMIC_VAR_INIT(1000ULL);
        unsigned           expected_integer   = 10U;
        unsigned long      expected_long      = 100UL;
        unsigned long long expected_long_long = 1000ULL;

        REQUIRE(ct_atomic_uint_compare_exchange(&integer, &expected_integer, 20U));
        REQUIRE(ct_atomic_ulong_compare_exchange(&long_integer, &expected_long, 200UL));
        REQUIRE(ct_atomic_ullong_compare_exchange(&long_long_integer, &expected_long_long, 2000ULL));

        expected_integer   = 10U;
        expected_long      = 100UL;
        expected_long_long = 1000ULL;
        REQUIRE_FALSE(ct_atomic_uint_compare_exchange(&integer, &expected_integer, 30U));
        REQUIRE_FALSE(ct_atomic_ulong_compare_exchange(&long_integer, &expected_long, 300UL));
        REQUIRE_FALSE(ct_atomic_ullong_compare_exchange(&long_long_integer, &expected_long_long, 3000ULL));
        REQUIRE(expected_integer == 20U);
        REQUIRE(expected_long == 200UL);
        REQUIRE(expected_long_long == 2000ULL);
        REQUIRE(ct_atomic_uint_load(&integer) == 20U);
        REQUIRE(ct_atomic_ulong_load(&long_integer) == 200UL);
        REQUIRE(ct_atomic_ullong_load(&long_long_integer) == 2000ULL);
    }
}

TEST_SUITE_END();
