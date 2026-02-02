#ifndef COTER_MATH_BCD_H
#define COTER_MATH_BCD_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline uint8_t ct_bcd_from_u8(uint8_t decimal) {
	uint8_t bcd = 0;

	if (!decimal) {
		return bcd;
	}
	bcd |= decimal % 10U;
	decimal /= 10U;

	if (!decimal) {
		return bcd;
	}
	bcd |= (decimal % 10U) << 4U;

	return bcd;
}

static inline uint8_t ct_bcd_to_u8(uint8_t bcd) {
	uint8_t decimal = 0;

	if (!bcd) {
		return decimal;
	}
	decimal += bcd & 0x0F;
	bcd >>= 4U;

	if (!bcd) {
		return decimal;
	}
	decimal += (bcd & 0x0F) * 10U;

	return decimal;
}

static inline uint16_t ct_bcd_from_u16(uint16_t decimal) {
	uint16_t bcd = 0;

	if (!decimal) {
		return bcd;
	}
	bcd |= decimal % 10U;
	decimal /= 10U;
	if (!decimal) {
		return bcd;
	}
	bcd |= (decimal % 10U) << 4U;
	decimal /= 10U;
	if (!decimal) {
		return bcd;
	}
	bcd |= (decimal % 10U) << 8U;
	decimal /= 10U;
	if (!decimal) {
		return bcd;
	}
	bcd |= (decimal % 10U) << 12U;

	return bcd;
}

static inline uint16_t ct_bcd_to_u16(uint16_t bcd) {
	uint16_t decimal = 0;

	if (!bcd) {
		return decimal;
	}
	decimal += bcd & 0x0F;
	bcd >>= 4U;
	if (!bcd) {
		return decimal;
	}
	decimal += (bcd & 0x0F) * 10U;
	bcd >>= 4U;
	if (!bcd) {
		return decimal;
	}
	decimal += (bcd & 0x0F) * 100U;
	bcd >>= 4U;
	if (!bcd) {
		return decimal;
	}
	decimal += (bcd & 0x0F) * 1000U;

	return decimal;
}

static inline uint32_t ct_bcd_from_u32(uint32_t decimal) {
	uint32_t bcd   = 0;
	uint32_t shift = 0;

	for (; decimal > 0;) {
		bcd |= (decimal % 10) << shift;
		decimal /= 10;
		shift += 4;
	}

	return bcd;
}

static inline uint32_t ct_bcd_to_u32(uint32_t bcd) {
	uint32_t decimal    = 0;
	uint32_t multiplier = 1;

	for (; bcd > 0;) {
		decimal += (bcd & 0x0F) * multiplier;
		multiplier *= 10;
		bcd >>= 4;
	}

	return decimal;
}

static inline uint64_t ct_bcd_from_u64(uint64_t decimal) {
	uint64_t bcd   = 0;
	int      shift = 0;

	for (; decimal > 0;) {
		bcd |= (decimal % 10) << shift;
		decimal /= 10;
		shift += 4;
	}

	return bcd;
}

static inline uint64_t ct_bcd_to_u64(uint64_t bcd) {
	uint64_t decimal    = 0;
	uint64_t multiplier = 1;

	for (; bcd > 0;) {
		decimal += (bcd & 0x0F) * multiplier;
		multiplier *= 10;
		bcd >>= 4;
	}

	return decimal;
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_MATH_BCD_H
