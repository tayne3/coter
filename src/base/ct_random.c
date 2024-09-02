/**
 * @file ct_random.c
 * @brief 随机数实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_random.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 左移操作
 * @param x 待左移的数
 * @param k 左移的位数
 * @return uint64_t 左移后的结果
 */
static inline uint64_t ct_random_rotl(uint64_t x, int k);

/**
 * @brief 生成下一个随机数
 * @param self 随机数句柄
 * @return uint64_t 生成的随机数
 */
static inline uint64_t ct_random_xoroshiro_next(ct_random_buf_t self);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_random_init(ct_random_buf_t self) {
	assert(self);
	const time_t t = time(NULL);
	srand((unsigned)t);
	self->_s[0] = t;
	self->_s[1] = rand();
}

bool ct_random_bool(ct_random_buf_t self) {
	assert(self);
	return ct_random_uint8(self, 0, 2);
}

uint8_t ct_random_uint8(ct_random_buf_t self, uint8_t min, uint8_t max) {
	assert(self);
	union {
		uint8_t  u8[8];
		uint64_t u64;
	} data = {.u64 = ct_random_xoroshiro_next(self) % (max - min) + min};
	return data.u8[0];
}

int8_t ct_random_int8(ct_random_buf_t self, int8_t min, int8_t max) {
	assert(self);
	union {
		int8_t   i8[8];
		uint64_t u64;
	} data = {.u64 = ct_random_xoroshiro_next(self) % (max - min) + min};
	return data.i8[0];
}

uint16_t ct_random_uint16(ct_random_buf_t self, uint16_t min, uint16_t max) {
	assert(self);
	union {
		uint16_t u16[4];
		uint64_t u64;
	} data = {.u64 = ct_random_xoroshiro_next(self) % (max - min) + min};
	return data.u16[0];
}

int16_t ct_random_int16(ct_random_buf_t self, int16_t min, int16_t max) {
	assert(self);
	union {
		uint16_t i16[4];
		uint64_t u64;
	} data = {.u64 = ct_random_xoroshiro_next(self) % (max - min) + min};
	return data.i16[0];
}

uint32_t ct_random_uint32(ct_random_buf_t self, uint32_t min, uint32_t max) {
	assert(self);
	union {
		uint32_t u32[2];
		uint64_t u64;
	} data = {.u64 = ct_random_xoroshiro_next(self) % (max - min) + min};
	return data.u32[0];
}

int32_t ct_random_int32(ct_random_buf_t self, int32_t min, int32_t max) {
	assert(self);
	union {
		int32_t  i32[2];
		uint64_t u64;
	} data = {.u64 = ct_random_xoroshiro_next(self) % (max - min) + min};
	return data.i32[0];
}

uint64_t ct_random_uint64(ct_random_buf_t self, uint64_t min, uint64_t max) {
	assert(self);
	return (ct_random_xoroshiro_next(self) % (max - min)) + min;
}

int64_t ct_random_int64(ct_random_buf_t self, int64_t min, int64_t max) {
	assert(self);
	return ((int64_t)ct_random_xoroshiro_next(self) % (max - min)) + min;
}

float ct_random_float(ct_random_buf_t self, float min, float max) {
	assert(self);
	const float value = (float)ct_random_uint32(self, 0, UINT16_MAX) / (float)UINT16_MAX;
	return min + value * (max - min);
}

double ct_random_double(ct_random_buf_t self, double min, double max) {
	assert(self);
	const double value = (double)ct_random_xoroshiro_next(self) / (double)UINT64_MAX;
	return min + value * (max - min);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline uint64_t ct_random_rotl(uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static inline uint64_t ct_random_xoroshiro_next(ct_random_buf_t self) {
	assert(self);
	uint64_t       s0     = self->_s[0];
	uint64_t       s1     = self->_s[1];
	const uint64_t result = s0 + s1;

	s1 ^= s0;
	self->_s[0] = ct_random_rotl(s0, 55) ^ s1 ^ (s1 << 14);
	self->_s[1] = ct_random_rotl(s1, 36);

	return result;
}
