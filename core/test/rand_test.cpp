#include "coter/core/rand.h"

#include <cstring>

#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("rand");

TEST_CASE("rand_null_safety") {
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

    ct_random_t rng;
    ct_random_seed(&rng, 1);
    ct_random_bytes(&rng, nullptr, sizeof(buffer));
}

TEST_CASE("rand_known_answer") {
    // Seed expansion: two SplitMix64 outputs. Expected state and outputs were
    // generated with an independent xoroshiro128++ 1.0 reference implementation.
    const uint64_t expected[] = {
        0x2ABFEA6A9AF18078ULL, 0x2132616B35565259ULL, 0x850378C3B2E42309ULL, 0x326DF705CC29056DULL,
        0x7D556729FDD0E190ULL, 0xBD0B25417FDA149CULL, 0x4D328DD0ACB646CCULL, 0x31ED08428BACB3D7ULL,
    };

    ct_random_t rng;
    ct_random_seed(&rng, 0x0123456789ABCDEFULL);

    REQUIRE(rng._s[0] == 0x157A3807A48FAA9DULL);
    REQUIRE(rng._s[1] == 0xD573529B34A1D093ULL);

    for (const uint64_t value : expected) { REQUIRE(ct_random_u64(&rng) == value); }
}

TEST_CASE("rand_seed_is_reproducible") {
    ct_random_t lhs;
    ct_random_t rhs;

    ct_random_seed(&lhs, 123456789ull);
    ct_random_seed(&rhs, 123456789ull);

    for (int i = 0; i < 64; ++i) { REQUIRE(ct_random_u64(&lhs) == ct_random_u64(&rhs)); }
}

TEST_CASE("rand_reseed_resets_sequence") {
    ct_random_t rng;
    uint64_t    first[8] = {0};

    ct_random_seed(&rng, 0x123456789ABCDEF0ULL);
    for (uint64_t& value : first) { value = ct_random_u64(&rng); }

    ct_random_seed(&rng, 0x123456789ABCDEF0ULL);
    for (uint64_t value : first) { REQUIRE(ct_random_u64(&rng) == value); }
}

TEST_CASE("rand_i64_matches_u64_bit_pattern") {
    ct_random_t unsigned_rng;
    ct_random_t signed_rng;

    ct_random_seed(&unsigned_rng, 123);
    ct_random_seed(&signed_rng, 123);

    for (int i = 0; i < 1024; ++i) {
        const uint64_t raw          = ct_random_u64(&unsigned_rng);
        const int64_t  signed_value = ct_random_i64(&signed_rng);

        REQUIRE(static_cast<uint64_t>(signed_value) == raw);
    }
}

TEST_CASE("rand_edge_seeds_are_reproducible") {
    ct_random_t zero_a;
    ct_random_t zero_b;
    ct_random_t max_a;
    ct_random_t max_b;

    ct_random_seed(&zero_a, 0);
    ct_random_seed(&zero_b, 0);
    ct_random_seed(&max_a, UINT64_MAX);
    ct_random_seed(&max_b, UINT64_MAX);

    REQUIRE((zero_a._s[0] | zero_a._s[1]) != 0);
    REQUIRE((max_a._s[0] | max_a._s[1]) != 0);

    for (int i = 0; i < 16; ++i) {
        REQUIRE(ct_random_u64(&zero_a) == ct_random_u64(&zero_b));
        REQUIRE(ct_random_u64(&max_a) == ct_random_u64(&max_b));
    }
}

TEST_CASE("rand_different_seeds_diverge") {
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

TEST_CASE("rand_auto_init_changes_state") {
    // Smoke test only: automatic seeding is best-effort and not guaranteed unique.
    ct_random_t rng;
    ct_random_init(&rng);

    const uint64_t a = ct_random_u64(&rng);
    const uint64_t b = ct_random_u64(&rng);

    REQUIRE(a != b);
}

TEST_CASE("rand_auto_init_produces_valid_state") {
    // Automatic seeding is best-effort, but a seeded state must never be all-zero.
    ct_random_t rng;

    for (int i = 0; i < 100; ++i) {
        ct_random_init(&rng);
        REQUIRE((rng._s[0] | rng._s[1]) != 0);
        (void)ct_random_u64(&rng);
    }
}

TEST_CASE("rand_u64_range_uses_half_open_interval") {
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

TEST_CASE("rand_i64_range_uses_half_open_interval") {
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

TEST_CASE("rand_i64_range_handles_extreme_bounds") {
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

TEST_CASE("rand_range_handles_full_span_and_singletons") {
    ct_random_t rng;
    ct_random_seed(&rng, 5);

    for (int i = 0; i < 10000; ++i) {
        REQUIRE(ct_random_u64_range(&rng, 0, UINT64_MAX) < UINT64_MAX);

        const int64_t value = ct_random_i64_range(&rng, INT64_MIN, INT64_MAX);
        REQUIRE(value >= INT64_MIN);
        REQUIRE(value < INT64_MAX);
    }

    REQUIRE(ct_random_u64_range(&rng, 7, 8) == 7);
    REQUIRE(ct_random_i64_range(&rng, -7, -6) == -7);
    REQUIRE(ct_random_i64_range(&rng, INT64_MIN, INT64_MIN + 1) == INT64_MIN);
    REQUIRE(ct_random_i64_range(&rng, INT64_MAX - 1, INT64_MAX) == INT64_MAX - 1);
}

TEST_CASE("rand_f64_is_unit_interval") {
    ct_random_t rng;
    ct_random_seed(&rng, 3);

    for (int i = 0; i < 10000; ++i) {
        const double value = ct_random_f64(&rng);
        REQUIRE(value >= 0.0);
        REQUIRE(value < 1.0);
    }
}

TEST_CASE("rand_bytes_are_reproducible") {
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

TEST_CASE("rand_bytes_writes_data") {
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

TEST_CASE("rand_bytes_zero_length_is_noop") {
    ct_random_t rng;
    uint8_t     buffer[4] = {1, 2, 3, 4};

    ct_random_seed(&rng, 123);
    const uint64_t state0 = rng._s[0];
    const uint64_t state1 = rng._s[1];

    ct_random_bytes(&rng, buffer, 0);

    REQUIRE(rng._s[0] == state0);
    REQUIRE(rng._s[1] == state1);
    REQUIRE(buffer[0] == 1);
    REQUIRE(buffer[1] == 2);
    REQUIRE(buffer[2] == 3);
    REQUIRE(buffer[3] == 4);
}

TEST_SUITE_END();
