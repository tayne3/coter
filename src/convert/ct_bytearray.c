/**
 * @file ct_bytearray.c
 * @brief 字节数组相关操作
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_bytearray.h"

#include <assert.h>
#include <string.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_bytearray]"

// -------------------------[GLOBAL DEFINITION]-------------------------

int ct_bytearray_to_hex(const char* array, int size, uint8_t* hex, int max)
{
	assert(array);
	assert(hex);

	int i   = 0;
	int idx = 0;
	for (; idx < max && i < size;) {
		// 跳过空格
		if (array[i] == ' ') {
			i++;
		} else {
			hex[idx++] = (CT_UINT8_FROM_ASCII(array[i]) << 4) + (CT_UINT8_FROM_ASCII(array[i + 1]) & 0x0F);
			i += 2;
		}
	}
	return i >= size ? idx : -1;
}

int ct_bytearray_from_hex(char* array, int max, const uint8_t* hex, int size)
{
	assert(array);
	assert(hex);

	int i   = 0;
	int idx = 0;
	for (; idx < size && i < max;) {
		array[i++] = CT_UINT8_TO_ASCII(hex[idx] >> 4);
		array[i++] = CT_UINT8_TO_ASCII(hex[idx] & 0x0F);
		if (i >= max) {
			break;
		}
		if (++idx < size) {
			// array[i++] = ' ';
		} else {
			array[i] = '\0';
			break;
		}
	}
	return idx == size ? i : i > size ? i - 1 : -1;
}

bool ct_bytearray_copy(uint8_t* target, const uint8_t* source, size_t size)
{
	assert(target);
	assert(source);

	return memcpy(target, source, size) == target;
}

bool ct_bytearray_copy_invert(uint8_t* target, const uint8_t* source, size_t size)
{
	assert(target);
	assert(source);

	for (size_t idx1 = 0, idx2 = size - 1; idx1 < size; idx1++, idx2--) {
		target[idx1] = source[idx2];
	}
	return true;
}

bool ct_bytearray_copys(uint8_t* target, const uint8_t* source, size_t size, ct_endian_t endian)
{
	assert(target);
	assert(source);

	if (endian == CTEndian_System) {
		return ct_bytearray_copy(target, source, size);
	} else {
		return ct_bytearray_copy_invert(target, source, size);
	}
}

int ct_bytearray_compare(const uint8_t* l, const uint8_t* r, size_t size)
{
	assert(l);
	assert(r);

	uint8_t lc, rc;

	for (; size--;) {
		if ((lc = *l++) != (rc = *r++)) {
			return (lc > rc) - (lc < rc);
		}
	}

	return 0;
}

uint16_t ct_bytearray_to_uint16(const uint8_t* array, size_t idx1, size_t idx2)
{
	assert(array);
	return (uint16_t)array[idx1] | ((uint16_t)array[idx2] << 8);
}

uint32_t ct_bytearray_to_uint32(const uint8_t* array, size_t idx1, size_t idx2, size_t idx3, size_t idx4)
{
	assert(array);
	return (uint32_t)array[idx1] | ((uint32_t)array[idx2] << 8) | ((uint32_t)array[idx3] << 16) |
		   ((uint32_t)array[idx4] << 24);
}

uint64_t ct_bytearray_to_uint64(const uint8_t* array, size_t idx1, size_t idx2, size_t idx3, size_t idx4, size_t idx5,
								size_t idx6, size_t idx7, size_t idx8)
{
	assert(array);
	return (uint64_t)array[idx1] | ((uint64_t)array[idx2] << 8) | ((uint64_t)array[idx3] << 16) |
		   ((uint64_t)array[idx4] << 24) | ((uint64_t)array[idx5] << 32) | ((uint64_t)array[idx6] << 40) |
		   ((uint64_t)array[idx7] << 48) | ((uint64_t)array[idx8] << 56);
}

uint16_t ct_bytearray_to_uint16s(const uint8_t* array, ct_endian_t endian)
{
	assert(array);
	if (endian == CTEndian_System) {
		return ct_bytearray_to_uint16(array, 0, 1);
	} else {
		return ct_bytearray_to_uint16(array, 1, 0);
	}
}

uint32_t ct_bytearray_to_uint32s(const uint8_t* array, ct_endian_t endian)
{
	assert(array);
	if (endian == CTEndian_System) {
		return ct_bytearray_to_uint32(array, 0, 1, 2, 3);
	} else {
		return ct_bytearray_to_uint32(array, 3, 2, 1, 0);
	}
}

uint64_t ct_bytearray_to_uint64s(const uint8_t* array, ct_endian_t endian)
{
	assert(array);
	if (endian == CTEndian_System) {
		return ct_bytearray_to_uint64(array, 0, 1, 2, 3, 4, 5, 6, 7);
	} else {
		return ct_bytearray_to_uint64(array, 7, 6, 5, 4, 3, 2, 1, 0);
	}
}

void ct_bytearray_from_uint16(uint8_t* array, uint16_t val, size_t idx1, size_t idx2)
{
	assert(array);
	array[idx1] = (uint8_t)val;
	array[idx2] = (uint8_t)(val >> 8);
}

void ct_bytearray_from_uint32(uint8_t* array, uint32_t val, size_t idx1, size_t idx2, size_t idx3, size_t idx4)
{
	assert(array);
	array[idx1] = (uint8_t)val;
	array[idx2] = (uint8_t)(val >> 8);
	array[idx3] = (uint8_t)(val >> 16);
	array[idx4] = (uint8_t)(val >> 24);
}

void ct_bytearray_from_uint64(uint8_t* array, uint64_t val, size_t idx1, size_t idx2, size_t idx3, size_t idx4,
							  size_t idx5, size_t idx6, size_t idx7, size_t idx8)
{
	assert(array);
	array[idx1] = (uint8_t)val;
	array[idx2] = (uint8_t)(val >> 8);
	array[idx3] = (uint8_t)(val >> 16);
	array[idx4] = (uint8_t)(val >> 24);
	array[idx5] = (uint8_t)(val >> 32);
	array[idx6] = (uint8_t)(val >> 40);
	array[idx7] = (uint8_t)(val >> 48);
	array[idx8] = (uint8_t)(val >> 56);
}

void ct_bytearray_from_uint16s(uint8_t* array, uint16_t val, ct_endian_t endian)
{
	assert(array);
	if (endian == CTEndian_System) {
		ct_bytearray_from_uint16(array, val, 0, 1);
	} else {
		ct_bytearray_from_uint16(array, val, 1, 0);
	}
}

void ct_bytearray_from_uint32s(uint8_t* array, uint32_t val, ct_endian_t endian)
{
	assert(array);
	if (endian == CTEndian_System) {
		ct_bytearray_from_uint32(array, val, 0, 1, 2, 3);
	} else {
		ct_bytearray_from_uint32(array, val, 3, 2, 1, 0);
	}
}

void ct_bytearray_from_uint64s(uint8_t* array, uint64_t val, ct_endian_t endian)
{
	assert(array);
	if (endian == CTEndian_System) {
		ct_bytearray_from_uint64(array, val, 0, 1, 2, 3, 4, 5, 6, 7);
	} else {
		ct_bytearray_from_uint64(array, val, 7, 6, 5, 4, 3, 2, 1, 0);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------
