#include "coter/bytes/rbuf.h"

size_t ct_rbuf_write(ct_rbuf_t *self, const uint8_t *src, size_t len) {
	if (!self || !src) { return 0; }
	if (len + self->len > self->cap) { len = self->cap - self->len; }
	if (len == 0) { return 0; }

	if (self->tail + len <= self->cap) {
		memcpy(&self->data[self->tail], src, len);
		self->tail += len;
	} else {
		const size_t first = self->cap - self->tail;
		const size_t rest  = len - first;
		memcpy(&self->data[self->tail], src, first);
		memcpy(self->data, &src[first], rest);
		self->tail = rest;
	}

	self->len += len;
	if (self->tail == self->cap) { self->tail = 0; }
	return len;
}

size_t ct_rbuf_read(ct_rbuf_t *self, uint8_t *dst, size_t len) {
	if (!self || !dst) { return 0; }
	if (len > self->len) { len = self->len; }
	if (len == 0) { return 0; }

	if (self->head + len <= self->cap) {
		memcpy(dst, &self->data[self->head], len);
		self->head += len;
	} else {
		const size_t first = self->cap - self->head;
		const size_t rest  = len - first;
		memcpy(dst, &self->data[self->head], first);
		memcpy(&dst[first], self->data, rest);
		self->head = rest;
	}

	self->len -= len;
	if (self->head == self->cap) { self->head = 0; }
	return len;
}

size_t ct_rbuf_peek(const ct_rbuf_t *self, uint8_t *dst, size_t len) {
	if (!self || !dst) { return 0; }
	if (len > self->len) { len = self->len; }
	if (len == 0) { return 0; }

	if (self->head + len <= self->cap) {
		memcpy(dst, &self->data[self->head], len);
	} else {
		const size_t first = self->cap - self->head;
		const size_t rest  = len - first;
		memcpy(dst, &self->data[self->head], first);
		memcpy(&dst[first], self->data, rest);
	}

	return len;
}
