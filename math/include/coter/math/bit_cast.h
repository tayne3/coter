#ifndef COTER_MATH_BIT_CAST_H
#define COTER_MATH_BIT_CAST_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert a 32-bit float to its bit representation
 * @param f The float value to convert
 * @return The bits of the float as uint32_t
 */
static inline uint32_t ct_bits_from_float32(float f) {
	union {
		float    f;
		uint32_t u;
	} c;
	c.f = f;
	return c.u;
}

/**
 * @brief Convert bits to a 32-bit float
 * @param u The bits to interpret as a float
 * @return The resulting float value
 */
static inline float ct_bits_to_float32(uint32_t u) {
	union {
		float    f;
		uint32_t u;
	} c;
	c.u = u;
	return c.f;
}

/**
 * @brief Convert a 64-bit double to its bit representation
 * @param f The double value to convert
 * @return The bits of the double as uint64_t
 */
static inline uint64_t ct_bits_from_float64(double f) {
	union {
		double   f;
		uint64_t u;
	} c;
	c.f = f;
	return c.u;
}

/**
 * @brief Convert bits to a 64-bit double
 * @param u The bits to interpret as a double
 * @return The resulting double value
 */
static inline double ct_bits_to_float64(uint64_t u) {
	union {
		double   f;
		uint64_t u;
	} c;
	c.u = u;
	return c.f;
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_MATH_BIT_CAST_H
