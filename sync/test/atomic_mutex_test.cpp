/**
 * @file atomic_mutex_test.cpp
 * @brief Tests for the mutex-based atomic fallback.
 */
#define COTER_TEST_ATOMIC_MUTEX
#include "coter/sync/atomic/atomic_mutex.h"

#include <thread>

#include "coter/sync/mutex.h"
#include "coter/testing/doctest.h"

#if !CT_ATOMIC_USE_MUTEX
static ct_mutex_t s_test_atomic_mutex = CT_MUTEX_INITIALIZER;

extern "C" void atomic_mutex_lock(void) {
    (void)ct_mutex_lock(&s_test_atomic_mutex);
}

extern "C" void atomic_mutex_unlock(void) {
    (void)ct_mutex_unlock(&s_test_atomic_mutex);
}
#endif

#define TEST_BASE_OP(name, type, first, second)                                               \
    do {                                                                                      \
        ct_atomic_##name##_t value = CT_ATOMIC_VAR_INIT((type)(first));                       \
        type                 expected;                                                        \
        type                 old;                                                             \
        REQUIRE(ct_atomic_##name##_load(&value) == (type)(first));                            \
        ct_atomic_##name##_store(&value, (type)(second));                                     \
        REQUIRE(ct_atomic_##name##_load(&value) == (type)(second));                           \
        old = ct_atomic_##name##_exchange(&value, (type)(first));                             \
        REQUIRE(old == (type)(second));                                                       \
        expected = (type)(first);                                                             \
        REQUIRE(ct_atomic_##name##_compare_exchange(&value, &expected, (type)(second)));      \
        REQUIRE(ct_atomic_##name##_load(&value) == (type)(second));                           \
        expected = (type)(first);                                                             \
        REQUIRE_FALSE(ct_atomic_##name##_compare_exchange(&value, &expected, (type)(first))); \
        REQUIRE(expected == (type)(second));                                                  \
    } while (0)

#define TEST_ARITH_OP(name, type)                                     \
    do {                                                              \
        ct_atomic_##name##_t value = CT_ATOMIC_VAR_INIT((type)10);    \
        REQUIRE(ct_atomic_##name##_add(&value, (type)3) == (type)10); \
        REQUIRE(ct_atomic_##name##_load(&value) == (type)13);         \
        REQUIRE(ct_atomic_##name##_sub(&value, (type)3) == (type)13); \
        REQUIRE(ct_atomic_##name##_load(&value) == (type)10);         \
    } while (0)

TEST_SUITE_BEGIN("atomic");

TEST_CASE("mutex fallback: atomic flag") {
    ct_atomic_flag_t flag = CT_ATOMIC_FLAG_INIT;

    REQUIRE_FALSE(ct_atomic_flag_test_and_set(&flag));
    REQUIRE(ct_atomic_flag_test_and_set(&flag));

    ct_atomic_flag_clear(&flag);
    REQUIRE_FALSE(ct_atomic_flag_test_and_set(&flag));
}

TEST_CASE("mutex fallback: load, store, exchange, and compare exchange") {
    TEST_BASE_OP(bool, bool, false, true);
    TEST_BASE_OP(char, char, 'a', 'b');
    TEST_BASE_OP(schar, signed char, -1, 2);
    TEST_BASE_OP(uchar, unsigned char, 1, 2);
    TEST_BASE_OP(short, short, -1, 2);
    TEST_BASE_OP(ushort, unsigned short, 1, 2);
    TEST_BASE_OP(int, int, -1, 2);
    TEST_BASE_OP(uint, unsigned, 1, 2);
    TEST_BASE_OP(long, long, -1, 2);
    TEST_BASE_OP(ulong, unsigned long, 1, 2);
    TEST_BASE_OP(llong, long long, -1, 2);
    TEST_BASE_OP(ullong, unsigned long long, 1, 2);
}

TEST_CASE("mutex fallback: add and sub") {
    TEST_ARITH_OP(char, char);
    TEST_ARITH_OP(schar, signed char);
    TEST_ARITH_OP(uchar, unsigned char);
    TEST_ARITH_OP(short, short);
    TEST_ARITH_OP(ushort, unsigned short);
    TEST_ARITH_OP(int, int);
    TEST_ARITH_OP(uint, unsigned);
    TEST_ARITH_OP(long, long);
    TEST_ARITH_OP(ulong, unsigned long);
    TEST_ARITH_OP(llong, long long);
    TEST_ARITH_OP(ullong, unsigned long long);
}

TEST_CASE("mutex fallback: pointer operations") {
    int             first  = 1;
    int             second = 2;
    ct_atomic_ptr_t value  = CT_ATOMIC_VAR_INIT(&first);

    REQUIRE(ct_atomic_ptr_load(&value) == &first);

    ct_atomic_ptr_store(&value, &second);
    REQUIRE(ct_atomic_ptr_load(&value) == &second);

    REQUIRE(ct_atomic_ptr_exchange(&value, &first) == &second);
    REQUIRE(ct_atomic_ptr_load(&value) == &first);

    void* expected = &first;
    REQUIRE(ct_atomic_ptr_compare_exchange(&value, &expected, &second));
    REQUIRE(ct_atomic_ptr_load(&value) == &second);

    expected = &first;
    REQUIRE_FALSE(ct_atomic_ptr_compare_exchange(&value, &expected, &first));
    REQUIRE(expected == &second);
    REQUIRE(ct_atomic_ptr_load(&value) == &second);
}

TEST_CASE("mutex fallback: concurrent increments") {
    static constexpr int thread_count = 4;
    static constexpr int iterations   = 1000;

    ct_atomic_int_t value = CT_ATOMIC_VAR_INIT(0);
    std::thread     threads[thread_count];

    for (auto& thread : threads) {
        thread = std::thread([&value]() {
            for (int i = 0; i < iterations; ++i) { ct_atomic_int_add(&value, 1); }
        });
    }
    for (auto& thread : threads) { thread.join(); }

    REQUIRE(ct_atomic_int_load(&value) == thread_count * iterations);
}

TEST_SUITE_END();
