/**
 * @file ct_bytes.c
 * @brief 字节数组
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_bytes.h"

#include "base/ct_platform.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_bytes]"

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_bytes_t *ct_bytes_create(size_t capacity) {
	capacity         = (capacity ? capacity : 1024UL);
	ct_bytes_t *self = (ct_bytes_t *)malloc(capacity + (sizeof(ct_bytes_t) << 1UL) - OFFSET_OF(ct_bytes_t, buffer));
	if (self == NULL) {
		return NULL;
	}

	ct_list_init(self->list);
	self->read_pos  = self->buffer;
	self->write_pos = self->buffer;
	self->cap       = capacity;
	return self;
}

void ct_bytes_destroy(ct_bytes_t *self) {
	if (self) {
		free(self);
	}
}

size_t ct_bytes_write(ct_bytes_t *self, const void *data, size_t length) {
	assert(self);
	assert(data);

	const size_t available_space = ct_bytes_available(self);
	if (length > available_space) {
		length = available_space;
	}
	if (length == 0) {
		return 0;
	}

	memcpy(self->write_pos, data, length);
	self->write_pos += length;
	return length;
}

size_t ct_bytes_read(ct_bytes_t *self, void *data, size_t max_length) {
	assert(self);
	assert(data);

	size_t available_space = self->write_pos - self->read_pos;
	if (available_space < max_length) {
		max_length = available_space;
	}
	if (max_length == 0) {
		return 0;
	}

	memcpy(data, self->read_pos, max_length);
	self->read_pos += max_length;
	return max_length;
}

// -------------------------[STATIC DEFINITION]-------------------------
