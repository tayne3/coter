/**
 * @file bit_cash.h
 * @brief 位操作类型
 */
#ifndef COTER_MATH_BIT_CASH_H
#define COTER_MATH_BIT_CASH_H

#include "coter/core/platform.h"

#ifndef __cplusplus

/**
 * @brief Convert a 32-bit float to its bit representation
 * @param f The float value to convert
 * @return The bits of the float as uint32_t
 */
static inline uint32_t ct_float32_to_bits(float f) {
	union {
		float    f;
		uint32_t u;
	} c = {.f = f};
	return c.u;
}

/**
 * @brief Convert bits to a 32-bit float
 * @param u The bits to interpret as a float
 * @return The resulting float value
 */
static inline float ct_float32_from_bits(uint32_t u) {
	union {
		float    f;
		uint32_t u;
	} c = {.u = u};
	return c.f;
}

/**
 * @brief Convert a 64-bit double to its bit representation
 * @param f The double value to convert
 * @return The bits of the double as uint64_t
 */
static inline uint64_t ct_float64_to_bits(double f) {
	union {
		double   f;
		uint64_t u;
	} c = {.f = f};
	return c.u;
}

/**
 * @brief Convert bits to a 64-bit double
 * @param u The bits to interpret as a double
 * @return The resulting double value
 */
static inline double ct_float64_from_bits(uint64_t u) {
	union {
		double   f;
		uint64_t u;
	} c = {.u = u};
	return c.f;
}

#elif __cplusplus >= 201103L

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace coter {

#if __cplusplus >= 202002L
using std::bit_cast;
#else
// clang-format off
template <typename To, typename From>
static inline
typename std::enable_if<
	sizeof(To) == sizeof(From) && 
	std::is_trivially_copyable<From>::value &&
	std::is_trivially_copyable<To>::value,
	To>::type
bit_cast(const From& src) noexcept {
	static_assert(sizeof(To) == sizeof(From), "bit_cast requires source and destination to have the same size");
	static_assert(std::is_trivially_copyable<From>::value, "bit_cast requires From to be trivially copyable");
	static_assert(std::is_trivially_copyable<To>::value, "bit_cast requires To to be trivially copyable");

	typename std::aligned_storage<sizeof(To), alignof(To)>::type storage;
	std::memcpy(&storage, &src, sizeof(To));
	return *reinterpret_cast<To*>(&storage);
}
// clang-format on
#endif

/**
 * @brief Convert floating-point value to its 32-bit representation
 * @tparam T Any floating-point type (float, double, long double)
 * @param f The floating-point value to convert
 * @return Bit pattern as uint32_t
 */
template <typename T>
static inline typename std::enable_if<std::is_floating_point<T>::value, uint32_t>::type float32_to_bits(T f) noexcept {
	return bit_cast<uint32_t>(static_cast<float>(f));
}

/**
 * @brief Convert 32-bit pattern to float
 * @tparam T Any integral type
 * @param bits The bit pattern to interpret as float
 * @return Resulting float value
 */
template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value, float>::type float32_from_bits(T bits) noexcept {
	return bit_cast<float>(static_cast<uint32_t>(bits));
}

/**
 * @brief Convert floating-point value to its 64-bit representation
 * @tparam T Any floating-point type (float, double, long double)
 * @param f The floating-point value to convert
 * @return Bit pattern as uint64_t
 */
template <typename T>
static inline typename std::enable_if<std::is_floating_point<T>::value, uint64_t>::type float64_to_bits(T f) noexcept {
	return bit_cast<uint64_t>(static_cast<double>(f));
}

/**
 * @brief Convert 64-bit pattern to double
 * @tparam T Any integral type
 * @param bits The bit pattern to interpret as double
 * @return Resulting double value
 */
template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value, double>::type float64_from_bits(T bits) noexcept {
	return bit_cast<double>(static_cast<uint64_t>(bits));
}

}  // namespace coter

#endif

#endif  // COTER_MATH_BIT_CASH_H
