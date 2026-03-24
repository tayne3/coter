/**
 * @file random.c
 * @brief 随机数实现
 */
#include "coter/math/rand.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 左移操作
 * @param x 待左移的数
 * @param k 左移的位数
 * @return uint64_t 左移后的结果
 */
static uint64_t ct_random_rotl(uint64_t x, int k);

/**
 * @brief 生成下一个随机数
 * @param self 随机数句柄
 * @return uint64_t 生成的随机数
 */
static uint64_t ct_random_xoroshiro_next(ct_random_t* self);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_random_init(ct_random_t* self) {
    if (!self) { return; }
    const time_t t = time(NULL);
    srand((unsigned)t);
    self->_s[0] = t;
    self->_s[1] = rand();
}

bool ct_random_bool(ct_random_t* self) {
    if (!self) { return false; }
    return ct_random_uint8(self, 0, 2) != 0;
}

uint8_t ct_random_uint8(ct_random_t* self, uint8_t min, uint8_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }

    uint8_t  range = max - min;
    uint16_t limit = 0x100u - (0x100u % range); /* 256 - (256 % range) */

    uint8_t sample;
    do { sample = (uint8_t)ct_random_xoroshiro_next(self); } while ((uint16_t)sample >= limit);

    return min + (sample % range);
}

int8_t ct_random_int8(ct_random_t* self, int8_t min, int8_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }
    const uint8_t umin   = (uint8_t)(min + 128);
    const uint8_t umax   = (uint8_t)(max + 128);
    const uint8_t result = ct_random_uint8(self, umin, umax);
    return (int8_t)(result - 128);
}

uint16_t ct_random_uint16(ct_random_t* self, uint16_t min, uint16_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }
    const uint16_t range = max - min;
    const uint32_t limit = 0x10000u - (0x10000u % range); /* 65536 - (65536 % range) */
    uint16_t       sample;
    do { sample = (uint16_t)ct_random_xoroshiro_next(self); } while ((uint32_t)sample >= limit);
    return min + (sample % range);
}

int16_t ct_random_int16(ct_random_t* self, int16_t min, int16_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }
    const uint16_t umin   = (uint16_t)(min + 32768);
    const uint16_t umax   = (uint16_t)(max + 32768);
    const uint16_t result = ct_random_uint16(self, umin, umax);
    return (int16_t)(result - 32768);
}

uint32_t ct_random_uint32(ct_random_t* self, uint32_t min, uint32_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }
    const uint32_t range = max - min;
    const uint64_t limit = 0x100000000ull - (0x100000000ull % range); /* 2^32 - (2^32 % range) */
    uint32_t       sample;
    do { sample = (uint32_t)ct_random_xoroshiro_next(self); } while ((uint64_t)sample >= limit);
    return min + (sample % range);
}

int32_t ct_random_int32(ct_random_t* self, int32_t min, int32_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }
    const uint32_t umin   = (uint32_t)(min + 2147483648u);
    const uint32_t umax   = (uint32_t)(max + 2147483648u);
    const uint32_t result = ct_random_uint32(self, umin, umax);
    return (int32_t)(result - 2147483648u);
}

uint64_t ct_random_uint64(ct_random_t* self, uint64_t min, uint64_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }
    const uint64_t range     = max - min;
    const uint64_t threshold = (UINT64_MAX - range + 1) % range;
    uint64_t       sample;
    do { sample = ct_random_xoroshiro_next(self); } while (sample < threshold);
    return min + (sample % range);
}

int64_t ct_random_int64(ct_random_t* self, int64_t min, int64_t max) {
    if (!self) { return 0; }
    if (max <= min) { return min; }
    const uint64_t umin   = (uint64_t)(min + 9223372036854775808ull);
    const uint64_t umax   = (uint64_t)(max + 9223372036854775808ull);
    const uint64_t result = ct_random_uint64(self, umin, umax);
    return (int64_t)(result - 9223372036854775808ull);
}

float ct_random_float(ct_random_t* self, float min, float max) {
    if (!self) { return 0; }
    return (ct_random_xoroshiro_next(self) >> 40) * (1.0f / (float)(1u << 24)) * (max - min) + min;
}

double ct_random_double(ct_random_t* self, double min, double max) {
    if (!self) { return 0; }
    return (ct_random_xoroshiro_next(self) >> 11) * (1.0 / (1ULL << 53)) * (max - min) + min;
}

void ct_random_string(ct_random_t* self, char* str, size_t length) {
    if (!self || !str) { return; }
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < length; ++i) { str[i] = charset[ct_random_uint8(self, 0, sizeof(charset) - 1)]; }
    str[length] = '\0';  // 添加 null 终止符
}

// -------------------------[STATIC DEFINITION]-------------------------

static uint64_t ct_random_rotl(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t ct_random_xoroshiro_next(ct_random_t* self) {
    if (!self) { return 0; }
    uint64_t       s0     = self->_s[0];
    uint64_t       s1     = self->_s[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    self->_s[0] = ct_random_rotl(s0, 55) ^ s1 ^ (s1 << 14);
    self->_s[1] = ct_random_rotl(s1, 36);

    return result;
}
