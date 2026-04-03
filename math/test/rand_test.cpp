#include "coter/math/rand.h"

#include <catch.hpp>
#include <cstring>

TEST_CASE("rand_null_safety", "[rand]") {
    REQUIRE(ct_random_u64(nullptr) == 0);
    REQUIRE(ct_random_i64(nullptr) == 0);
    REQUIRE(ct_random_u64_range(nullptr, 10, 20) == 0);
    REQUIRE(ct_random_i64_range(nullptr, -10, 20) == 0);
    REQUIRE(ct_random_f64(nullptr) == 0.0);

    uint8_t buffer[16] = {0};
    ct_random_seed(nullptr, 123);
    ct_random_init(nullptr);
    ct_random_bytes(nullptr, buffer, sizeof(buffer));
    ct_random_bytes(nullptr, nullptr, sizeof(buffer));
}

TEST_CASE("rand_seed_is_reproducible", "[rand]") {
    ct_random_t lhs;
    ct_random_t rhs;

    ct_random_seed(&lhs, 123456789ull);
    ct_random_seed(&rhs, 123456789ull);

    for (int i = 0; i < 64; ++i) { REQUIRE(ct_random_u64(&lhs) == ct_random_u64(&rhs)); }
}

TEST_CASE("rand_i64_is_reproducible", "[rand]") {
    ct_random_t lhs;
    ct_random_t rhs;

    ct_random_seed(&lhs, 42);
    ct_random_seed(&rhs, 42);

    for (int i = 0; i < 64; ++i) { REQUIRE(ct_random_i64(&lhs) == ct_random_i64(&rhs)); }
}

TEST_CASE("rand_edge_seeds_are_reproducible", "[rand]") {
    ct_random_t zero_a;
    ct_random_t zero_b;
    ct_random_t max_a;
    ct_random_t max_b;

    ct_random_seed(&zero_a, 0);
    ct_random_seed(&zero_b, 0);
    ct_random_seed(&max_a, UINT64_MAX);
    ct_random_seed(&max_b, UINT64_MAX);

    for (int i = 0; i < 16; ++i) {
        REQUIRE(ct_random_u64(&zero_a) == ct_random_u64(&zero_b));
        REQUIRE(ct_random_u64(&max_a) == ct_random_u64(&max_b));
    }
}

TEST_CASE("rand_different_seeds_diverge", "[rand]") {
    ct_random_t lhs;
    ct_random_t rhs;

    ct_random_seed(&lhs, 1);
    ct_random_seed(&rhs, 2);

    bool all_same = true;
    for (int i = 0; i < 8; ++i) {
        if (ct_random_u64(&lhs) != ct_random_u64(&rhs)) {
            all_same = false;
            break;
        }
    }
    REQUIRE_FALSE(all_same);
}

TEST_CASE("rand_auto_init_changes_state", "[rand]") {
    ct_random_t rng;
    ct_random_init(&rng);

    const uint64_t a = ct_random_u64(&rng);
    const uint64_t b = ct_random_u64(&rng);

    REQUIRE(a != b);
}

TEST_CASE("rand_u64_range_uses_half_open_interval", "[rand]") {
    ct_random_t rng;
    ct_random_seed(&rng, 1);

    for (int i = 0; i < 10000; ++i) {
        const uint64_t value = ct_random_u64_range(&rng, 10, 20);
        REQUIRE(value >= 10);
        REQUIRE(value < 20);
    }

    REQUIRE(ct_random_u64_range(&rng, 42, 42) == 42);
    REQUIRE(ct_random_u64_range(&rng, 50, 42) == 50);
    REQUIRE(ct_random_u64_range(&rng, UINT64_MAX - 5, UINT64_MAX) >= UINT64_MAX - 5);
    REQUIRE(ct_random_u64_range(&rng, UINT64_MAX - 5, UINT64_MAX) < UINT64_MAX);
}

TEST_CASE("rand_i64_range_uses_half_open_interval", "[rand]") {
    ct_random_t rng;
    ct_random_seed(&rng, 2);

    for (int i = 0; i < 10000; ++i) {
        const int64_t value = ct_random_i64_range(&rng, -50, 50);
        REQUIRE(value >= -50);
        REQUIRE(value < 50);
    }

    REQUIRE(ct_random_i64_range(&rng, -7, -7) == -7);
    REQUIRE(ct_random_i64_range(&rng, 50, -50) == 50);
}

TEST_CASE("rand_i64_range_handles_extreme_bounds", "[rand]") {
    ct_random_t rng;
    ct_random_seed(&rng, 4);

    for (int i = 0; i < 10000; ++i) {
        const int64_t low  = ct_random_i64_range(&rng, INT64_MIN, INT64_MIN + 32);
        const int64_t high = ct_random_i64_range(&rng, INT64_MAX - 32, INT64_MAX);

        REQUIRE(low >= INT64_MIN);
        REQUIRE(low < INT64_MIN + 32);
        REQUIRE(high >= INT64_MAX - 32);
        REQUIRE(high < INT64_MAX);
    }
}

TEST_CASE("rand_f64_is_unit_interval", "[rand]") {
    ct_random_t rng;
    ct_random_seed(&rng, 3);

    for (int i = 0; i < 10000; ++i) {
        const double value = ct_random_f64(&rng);
        REQUIRE(value >= 0.0);
        REQUIRE(value < 1.0);
    }
}

TEST_CASE("rand_bytes_are_reproducible", "[rand]") {
    ct_random_t lhs;
    ct_random_t rhs;
    uint8_t     a[33] = {0};
    uint8_t     b[33] = {0};

    ct_random_seed(&lhs, 0xABCDEFu);
    ct_random_seed(&rhs, 0xABCDEFu);

    ct_random_bytes(&lhs, a, sizeof(a));
    ct_random_bytes(&rhs, b, sizeof(b));

    REQUIRE(std::memcmp(a, b, sizeof(a)) == 0);
}

TEST_CASE("rand_bytes_writes_data", "[rand]") {
    ct_random_t rng;
    uint8_t     buffer[31] = {0};

    ct_random_seed(&rng, 99);
    ct_random_bytes(&rng, buffer, sizeof(buffer));

    bool any_non_zero = false;
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        if (buffer[i] != 0) {
            any_non_zero = true;
            break;
        }
    }
    REQUIRE(any_non_zero);
}

TEST_CASE("rand_bytes_zero_length_is_noop", "[rand]") {
    ct_random_t rng;
    uint8_t     buffer[4] = {1, 2, 3, 4};

    ct_random_seed(&rng, 123);
    ct_random_bytes(&rng, buffer, 0);

    REQUIRE(buffer[0] == 1);
    REQUIRE(buffer[1] == 2);
    REQUIRE(buffer[2] == 3);
    REQUIRE(buffer[3] == 4);
}
