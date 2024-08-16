/**
 * @file ct_vector.c
 * @brief 动态数组实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.15
 */
#include "ct_vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_vector]"

#define CT_VECTOR_ITEM(self, idx) (&(self)->_all[idx * (self)->_byte])  // 动态数组-指定元素
#define CT_VECTOR_MAX(self)       ((self)->_max)                        // 动态数组-最大容量
#define CT_VECTOR_SIZE(self)      ((self)->_size)                       // 动态数组-当前大小
#define CT_VECTOR_MEMORY_MAX      0x80000000                            // 动态数组-最大内存限制

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_vector_init(ct_vector_buf_t self, size_t byte, size_t max)
{
	assert(self);
	assert(byte);
	assert(max);
	assert(byte * max <= CT_VECTOR_MEMORY_MAX);

	self->_all = calloc(max, byte);
	assert(self->_all);

	self->_byte = byte;
	self->_max = self->_size = max;
}

void ct_vector_destroy(ct_vector_buf_t self)
{
	assert(self);

	if (self->_all) {
		free(self->_all);
		self->_all = ct_nullptr;
	}
	self->_byte = self->_max = self->_size = 0;
}

size_t ct_vector_size(const ct_vector_buf_t self)
{
	assert(self);

	return CT_VECTOR_SIZE(self);
}

void* ct_vector_at(ct_vector_buf_t self, size_t idx)
{
	assert(self);
	assert(self->_byte);
	assert(self->_all);

	if (idx >= self->_size) {
		return ct_nullptr;
	}

	return CT_VECTOR_ITEM(self, idx);
}

const void* ct_vector_value(const ct_vector_buf_t self, size_t idx)
{
	assert(self);
	assert(self->_byte);
	assert(self->_all);

	if (idx >= self->_size) {
		return ct_nullptr;
	}

	return CT_VECTOR_ITEM(self, idx);
}

bool ct_vector_insert(ct_vector_buf_t self, size_t idx, const void* data)
{
	assert(self);
	assert(self->_byte);
	assert(self->_all);

	if (idx >= self->_size) {
		if (!ct_vector_resize(self, idx + 1)) {
			return false;
		}
	}

	memcpy(CT_VECTOR_ITEM(self, idx), data, self->_byte);
	return true;
}

bool ct_vector_resize(ct_vector_buf_t self, size_t size)
{
	assert(self);
	assert(self->_byte);
	assert(self->_all);

	if (size <= self->_max) {
		self->_size = size;
		return true;
	}

	const size_t max    = self->_max * 2 < size ? size : self->_max * 2;
	const size_t blocks = max * self->_byte;

	if (blocks > CT_VECTOR_MEMORY_MAX) {
		return false;
	}

	self->_all = realloc(self->_all, blocks);

	if (!self->_all) {
		return false;
	}

	self->_max  = max;
	self->_size = size;
	return true;
}

// -------------------------[STATIC DEFINITION]-------------------------
