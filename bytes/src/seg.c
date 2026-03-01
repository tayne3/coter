#include "coter/bytes/seg.h"

#include "coter/encoding/binary.h"

void ct_seg_compact(ct_seg_t *self) {
	if (self->pos == 0) { return; }
	if (self->pos >= self->len) {
		ct_seg_clear(self);
		return;
	}
	const size_t readable = ct_seg_readable(self);
	memmove(self->data, self->data + self->pos, readable);
	self->len = (uint32_t)readable;
	self->pos = 0U;
}

int ct_seg_find(const ct_seg_t *self, uint8_t bt, size_t offset) {
	if (self->len <= self->pos + offset) { return -1; }
	const uint8_t *p = memchr(self->data + self->pos + offset, bt, self->len - self->pos - offset);
	if (!p) { return -1; }
	return (int)(p - self->data) - (int)(self->pos + offset);
}

int ct_seg_fill(ct_seg_t *self, uint8_t bt, size_t length) {
	const size_t writable = ct_seg_writable(self);
	if (length > writable) { length = writable; }
	if (!length) { return 0; }

	memset(self->data + self->pos, bt, length);
	self->pos += (uint32_t)length;
	if (self->pos > self->len) { self->len = self->pos; }
	return (int)length;
}

int ct_seg_overfill(ct_seg_t *self, uint8_t bt, size_t length) {
	if (length > self->cap) { length = self->cap; }
	if (!length) { return 0; }

	memset(self->data, bt, length);
	return (int)length;
}

int ct_seg_get_bytes(const ct_seg_t *self, size_t offset, uint8_t *p, size_t length) {
	if (offset >= self->len) { return 0; }
	if (length + offset > self->len) { length = self->len - offset; }
	if (!p || !length) { return 0; }

	memcpy(p, self->data + offset, length);
	return (int)length;
}

int ct_seg_set_bytes(ct_seg_t *self, size_t offset, const uint8_t *p, size_t length) {
	if (offset + length > self->len) { return -1; }
	if (!p || !length) { return 0; }

	memcpy(self->data + offset, p, length);
	return 0;
}

int ct_seg_peek_bytes(const ct_seg_t *self, int offset, uint8_t *p, size_t length) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos >= (int)self->len) { return 0; }
	if ((size_t)absPos + length > self->len) { length = self->len - (size_t)absPos; }
	if (!p || !length) { return 0; }

	memcpy(p, self->data + absPos, length);
	return (int)length;
}

int ct_seg_poke_bytes(ct_seg_t *self, int offset, const uint8_t *p, size_t length) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || (size_t)absPos + length > self->len) { return -1; }
	if (!p || !length) { return 0; }

	memcpy(self->data + absPos, p, length);
	return 0;
}

int ct_seg_take_bytes(ct_seg_t *self, uint8_t *p, size_t length) {
	if (length + self->pos > self->len) { length = self->len - self->pos; }
	if (!p || !length) { return 0; }

	memcpy(p, self->data + self->pos, length);
	self->pos += (uint32_t)length;
	return (int)length;
}

int ct_seg_put_bytes(ct_seg_t *self, const uint8_t *p, size_t length) {
	if (length + self->pos > self->cap) { length = self->cap - self->pos; }
	if (!p || !length) { return 0; }

	memcpy(self->data + self->pos, p, length);
	self->pos += (uint32_t)length;
	if (self->pos > self->len) { self->len = self->pos; }
	return (int)length;
}

void ct_seg_put_u8(ct_seg_t *self, uint8_t v) {
	if (ct_seg_writable(self) < 1) { return; }

	self->data[self->pos++] = v;
	if (self->pos > self->len) { self->len = self->pos; }
}

void ct_seg_put_u16(ct_seg_t *self, uint16_t v) {
	if (ct_seg_writable(self) < 2) { return; }

	if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }

	memcpy(self->data + self->pos, &v, 2);
	self->pos += 2;
	if (self->pos > self->len) { self->len = self->pos; }
}

void ct_seg_put_u32(ct_seg_t *self, uint32_t v) {
	if (ct_seg_writable(self) < 4) { return; }

	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x2(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}

	memcpy(self->data + self->pos, &v, 4);
	self->pos += 4;
	if (self->pos > self->len) { self->len = self->pos; }
}

void ct_seg_put_u64(ct_seg_t *self, uint64_t v) {
	if (ct_seg_writable(self) < 8) { return; }

	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x4(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}

	memcpy(self->data + self->pos, &v, 8);
	self->pos += 8;
	if (self->pos > self->len) { self->len = self->pos; }
}

uint8_t ct_seg_take_u8(ct_seg_t *self) {
	if (ct_seg_readable(self) < 1) { return 0; }
	return self->data[self->pos++];
}

uint16_t ct_seg_take_u16(ct_seg_t *self) {
	if (ct_seg_readable(self) < 2) { return 0; }
	uint16_t v;
	memcpy(&v, self->data + self->pos, 2);
	self->pos += 2;

	if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
	return v;
}

uint32_t ct_seg_take_u32(ct_seg_t *self) {
	if (ct_seg_readable(self) < 4) { return 0; }
	uint32_t v;
	memcpy(&v, self->data + self->pos, 4);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x2(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}
	self->pos += 4;
	return v;
}

uint64_t ct_seg_take_u64(ct_seg_t *self) {
	if (ct_seg_readable(self) < 8) { return 0; }
	uint64_t v;
	memcpy(&v, self->data + self->pos, 8);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x4(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}
	self->pos += 8;
	return v;
}

uint8_t ct_seg_peek_u8(const ct_seg_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 1 > (int)self->len) { return 0; }
	return self->data[absPos];
}

uint16_t ct_seg_peek_u16(const ct_seg_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 2 > (int)self->len) { return 0; }
	uint16_t v;
	memcpy(&v, self->data + absPos, 2);
	if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
	return v;
}

uint32_t ct_seg_peek_u32(const ct_seg_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 4 > (int)self->len) { return 0; }
	uint32_t v;
	memcpy(&v, self->data + absPos, 4);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x2(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}
	return v;
}

uint64_t ct_seg_peek_u64(const ct_seg_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 8 > (int)self->len) { return 0; }
	uint64_t v;
	memcpy(&v, self->data + absPos, 8);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x4(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}
	return v;
}

uint8_t ct_seg_get_u8(const ct_seg_t *self, size_t offset) {
	if (offset + 1 > self->len) { return 0; }
	return self->data[offset];
}

uint16_t ct_seg_get_u16(const ct_seg_t *self, size_t offset) {
	if (offset + 2 > self->len) { return 0; }
	uint16_t v;
	memcpy(&v, self->data + offset, 2);
	if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
	return v;
}

uint32_t ct_seg_get_u32(const ct_seg_t *self, size_t offset) {
	if (offset + 4 > self->len) { return 0; }
	uint32_t v;
	memcpy(&v, self->data + offset, 4);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x2(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}
	return v;
}

uint64_t ct_seg_get_u64(const ct_seg_t *self, size_t offset) {
	if (offset + 8 > self->len) { return 0; }
	uint64_t v;
	memcpy(&v, self->data + offset, 8);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x4(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}
	return v;
}

int ct_seg_set_u8(ct_seg_t *self, size_t offset, uint8_t v) {
	if (offset + 1 > self->len) { return -1; }
	self->data[offset] = v;
	return 0;
}

int ct_seg_set_u16(ct_seg_t *self, size_t offset, uint16_t v) {
	if (offset + 2 > self->len) { return -1; }
	if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
	memcpy(self->data + offset, &v, 2);
	return 0;
}

int ct_seg_set_u32(ct_seg_t *self, size_t offset, uint32_t v) {
	if (offset + 4 > self->len) { return -1; }
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x2(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}
	memcpy(self->data + offset, &v, 4);
	return 0;
}

int ct_seg_set_u64(ct_seg_t *self, size_t offset, uint64_t v) {
	if (offset + 8 > self->len) { return -1; }
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) { v = ct_binary_bswap16_x4(v); }
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}
	memcpy(self->data + offset, &v, 8);
	return 0;
}
