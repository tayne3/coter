/**
 * @file rand.c
 * @brief random number generator implementation.
 */
#include "coter/math/rand.h"

#include <string.h>

// -------------------------[STATIC DECLARATION]-------------------------

static uint64_t ct_random_rotl(uint64_t x, int k);
static uint64_t ct_random_splitmix64(uint64_t* state);
static uint64_t ct_random_seed_material(void);
static uint64_t ct_random_mix64(uint64_t value);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_random_init(ct_random_t* self) {
    if (!self) { return; }
    ct_random_seed(self, ct_random_seed_material());
}

void ct_random_seed(ct_random_t* self, uint64_t seed) {
    if (!self) { return; }

    self->_s[0] = ct_random_splitmix64(&seed);
    self->_s[1] = ct_random_splitmix64(&seed);

    if (self->_s[0] == 0 && self->_s[1] == 0) { self->_s[0] = 0x9E3779B97F4A7C15ULL; }
}

uint64_t ct_random_u64(ct_random_t* self) {
    if (!self) { return 0; }

    const uint64_t s0     = self->_s[0];
    uint64_t       s1     = self->_s[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    self->_s[0] = ct_random_rotl(s0, 55) ^ s1 ^ (s1 << 14);
    self->_s[1] = ct_random_rotl(s1, 36);

    return result;
}

int64_t ct_random_i64(ct_random_t* self) {
    return (int64_t)ct_random_u64(self);
}

uint64_t ct_random_u64_range(ct_random_t* self, uint64_t min, uint64_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }

    const uint64_t range     = max - min;
    const uint64_t threshold = (uint64_t)(0 - range) % range;

    for (;;) {
        const uint64_t sample = ct_random_u64(self);
        if (sample >= threshold) { return min + (sample % range); }
    }
}

int64_t ct_random_i64_range(ct_random_t* self, int64_t min, int64_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }

    const uint64_t offset = 0x8000000000000000ull;
    const uint64_t umin   = ((uint64_t)min) ^ offset;
    const uint64_t umax   = ((uint64_t)max) ^ offset;
    const uint64_t value  = ct_random_u64_range(self, umin, umax);

    return (int64_t)(value ^ offset);
}

double ct_random_f64(ct_random_t* self) {
    if (!self) { return 0.0; }
    return (ct_random_u64(self) >> 11) * (1.0 / (1ULL << 53));
}

void ct_random_bytes(ct_random_t* self, void* buffer, size_t size) {
    if (!self || !buffer) { return; }

    uint8_t* out = (uint8_t*)buffer;
    size_t   i   = 0;
    while (i + sizeof(uint64_t) <= size) {
        const uint64_t value = ct_random_u64(self);
        memcpy(out + i, &value, sizeof(value));
        i += sizeof(value);
    }

    if (i < size) {
        const uint64_t value = ct_random_u64(self);
        memcpy(out + i, &value, size - i);
    }
}

// -------------------------[STATIC DEFINITION]-------------------------

static uint64_t ct_random_rotl(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t ct_random_splitmix64(uint64_t* state) {
    uint64_t z = (*state += 0x9E3779B97F4A7C15ull);
    z          = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z          = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

static uint64_t ct_random_seed_material(void) {
    uint64_t seed = (uint64_t)ct_time(NULL);
    seed ^= ((uint64_t)ct_current_microsecond() << 7);
    seed ^= ((uint64_t)ct_gettimeofday_ms() << 17);
    seed ^= ((uint64_t)ct_getuptime_ms() << 29);
    seed ^= ((uint64_t)ct_gethrtime_us() << 41);
    seed ^= ((uint64_t)ct_getpid() << 53);
    seed ^= (uint64_t)(uintptr_t)&seed;
    seed ^= ct_random_rotl(seed, 17);
    return ct_random_mix64(seed);
}

static uint64_t ct_random_mix64(uint64_t value) {
    value ^= value >> 30;
    value *= 0xBF58476D1CE4E5B9ull;
    value ^= value >> 27;
    value *= 0x94D049BB133111EBull;
    value ^= value >> 31;
    return value;
}
