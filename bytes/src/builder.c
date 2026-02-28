#include "coter/bytes/builder.h"

/**
 * @brief Ensure the stream has at least the required capacity
 * @param self Stream object
 * @param required Required capacity in bytes
 * @return 0 on success, -1 on failure
 */
static int ensure_capacity(ct_builder_t *self, size_t required) {
	if (required <= self->seg.cap) { return 0; }

	size_t new_cap;
	if (self->seg.cap == 0) {
		new_cap = required < 64 ? 64 : required;
	} else {
		new_cap = self->seg.cap * 2;
		if (new_cap < required) { new_cap = required; }
	}

	uint8_t *new_buffer = (uint8_t *)realloc(self->seg.data, new_cap);
	if (!new_buffer) { return -1; }
	self->seg.data = new_buffer;
	self->seg.cap  = (uint32_t)new_cap;
	return 0;
}

ct_builder_t *ct_builder_create(size_t initial_capacity) {
	ct_builder_t *self = (ct_builder_t *)malloc(sizeof(ct_builder_t));
	if (!self) { return NULL; }

	if (initial_capacity == 0) { initial_capacity = 64; }
	uint8_t *buffer = (uint8_t *)malloc(initial_capacity);
	if (!buffer) {
		free(self);
		return NULL;
	}

	ct_seg_init(&self->seg, buffer, initial_capacity);
	return self;
}

void ct_builder_destroy(ct_builder_t *self) {
	if (self) {
		if (self->seg.data) { free(self->seg.data); }
		free(self);
	}
}

int ct_builder_grow(ct_builder_t *self, size_t n) {
	const size_t required = self->seg.len + n;
	return ensure_capacity(self, required);
}

int ct_builder_reserve(ct_builder_t *self, size_t capacity) {
	return ensure_capacity(self, capacity);
}

int ct_builder_write(ct_builder_t *self, const uint8_t *p, size_t length) {
	if (!p || length == 0) { return 0; }

	const size_t required = self->seg.pos + length;
	if (ensure_capacity(self, required) != 0) {
		return -1;  // Failed to grow
	}
	return ct_seg_write(&self->seg, p, length);
}

int ct_builder_fill(ct_builder_t *self, uint8_t bt, size_t length) {
	if (length == 0) { return 0; }

	const size_t required = self->seg.pos + length;
	if (ensure_capacity(self, required) != 0) {
		return -1;  // Failed to grow
	}
	return ct_seg_fill(&self->seg, bt, length);
}

void ct_builder_put_u8(ct_builder_t *self, uint8_t v) {
	const size_t required = self->seg.pos + 1;
	if (ensure_capacity(self, required) != 0) {
		return;  // Failed to grow, silent failure
	}
	ct_seg_put_u8(&self->seg, v);
}

void ct_builder_put_u16(ct_builder_t *self, uint16_t v) {
	const size_t required = self->seg.pos + 2;
	if (ensure_capacity(self, required) != 0) { return; }
	ct_seg_put_u16(&self->seg, v);
}

void ct_builder_put_u32(ct_builder_t *self, uint32_t v) {
	const size_t required = self->seg.pos + 4;
	if (ensure_capacity(self, required) != 0) { return; }
	ct_seg_put_u32(&self->seg, v);
}

void ct_builder_put_u64(ct_builder_t *self, uint64_t v) {
	const size_t required = self->seg.pos + 8;
	if (ensure_capacity(self, required) != 0) { return; }
	ct_seg_put_u64(&self->seg, v);
}
