#include "coter/bytes/span.h"
#include "coter/encoding/binary.h"

void ct_span_compact(ct_span_t *self) {
	if (self->pos == 0) {
		return;
	}
	if (self->pos >= self->len) {
		ct_span_clear(self);
		return;
	}
	const size_t readable = ct_span_readable(self);
	memmove(self->bytes, self->bytes + self->pos, readable);
	self->len = (uint32_t)readable;
	self->pos = 0U;
}

int ct_span_peek(ct_span_t *self, uint8_t *p, size_t length) {
	if (!p || !length) {
		return 0;
	}
	const size_t readable = ct_span_readable(self);
	if (!readable) {
		return 0;
	}
	if (length > readable) {
		length = readable;
	}
	memcpy(p, self->bytes + self->pos, length);
	return (int)length;
}

int ct_span_fill(ct_span_t *self, uint8_t bt, size_t length) {
	if (!length) {
		return 0;
	}
	const size_t writable = ct_span_writable(self);
	if (!writable) {
		return 0;
	}
	if (length > writable) {
		length = writable;
	}

	memset(self->bytes + self->pos, bt, length);
	self->pos += (uint32_t)length;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
	return (int)length;
}

int ct_span_read(ct_span_t *self, uint8_t *p, size_t length) {
	if (!p || !length) {
		return -1;
	}
	const size_t readable = ct_span_readable(self);
	if (!readable) {
		return 0;
	}
	if (length > readable) {
		length = readable;
	}

	memcpy(p, self->bytes + self->pos, length);
	self->pos += (uint32_t)length;
	return (int)length;
}

int ct_span_write(ct_span_t *self, const uint8_t *p, size_t length) {
	if (!p || !length) {
		return -1;
	}
	const size_t writable = ct_span_writable(self);
	if (!writable) {
		return 0;
	}
	if (length > writable) {
		length = writable;
	}

	memcpy(self->bytes + self->pos, p, length);
	self->pos += (uint32_t)length;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
	return (int)length;
}

void ct_span_put_u8(ct_span_t *self, uint8_t v) {
	if (ct_span_writable(self) < 1) {
		return;
	}

	self->bytes[self->pos++] = v;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

void ct_span_put_u16(ct_span_t *self, uint16_t v) {
	if (ct_span_writable(self) < 2) {
		return;
	}

	if (self->endian != CT_ENDIAN_SYSTEM) {
		v = ct_binary_bswap16(v);
	}

	memcpy(self->bytes + self->pos, &v, 2);
	self->pos += 2;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

void ct_span_put_u32(ct_span_t *self, uint32_t v) {
	if (ct_span_writable(self) < 4) {
		return;
	}

	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x2(v);
		}
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}

	memcpy(self->bytes + self->pos, &v, 4);
	self->pos += 4;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

void ct_span_put_u64(ct_span_t *self, uint64_t v) {
	if (ct_span_writable(self) < 8) {
		return;
	}

	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x4(v);
		}
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}

	memcpy(self->bytes + self->pos, &v, 8);
	self->pos += 8;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

void ct_span_put_arr8(ct_span_t *self, const uint8_t *v, size_t count) {
	if (!count) {
		return;
	}
	if (ct_span_writable(self) < count) {
		return;
	}

	memcpy(self->bytes + self->pos, v, count);
	self->pos += (uint32_t)count;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

void ct_span_put_arr16(ct_span_t *self, const uint16_t *v, size_t count) {
	if (!count) {
		return;
	}
	const size_t required = count << 1;
	if (ct_span_writable(self) < required) {
		return;
	}
	memcpy(self->bytes + self->pos, v, required);
	if (self->endian != CT_ENDIAN_SYSTEM) {
		ct_binary_bswap16_batch((uint16_t *)(self->bytes + self->pos), count);
	}
	self->pos += (uint32_t)required;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

void ct_span_put_arr32(ct_span_t *self, const uint32_t *v, size_t count) {
	if (!count) {
		return;
	}
	const size_t required = count << 2;
	if (ct_span_writable(self) < required) {
		return;
	}
	memcpy(self->bytes + self->pos, v, required);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			ct_binary_bswap16_x2_batch((uint32_t *)(self->bytes + self->pos), count);
		}
	} else {
		if (self->hlswap) {
			ct_binary_reverse_words32_batch((uint32_t *)(self->bytes + self->pos), count);
		} else {
			ct_binary_bswap32_batch((uint32_t *)(self->bytes + self->pos), count);
		}
	}

	self->pos += (uint32_t)required;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

void ct_span_put_arr64(ct_span_t *self, const uint64_t *v, size_t count) {
	if (!count) {
		return;
	}
	const size_t required = count * 8;
	if (ct_span_writable(self) < required) {
		return;
	}
	memcpy(self->bytes + self->pos, v, required);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			ct_binary_bswap16_x4_batch((uint64_t *)(self->bytes + self->pos), count);
		}
	} else {
		if (self->hlswap) {
			ct_binary_reverse_words64_batch((uint64_t *)(self->bytes + self->pos), count);
		} else {
			ct_binary_bswap64_batch((uint64_t *)(self->bytes + self->pos), count);
		}
	}
	self->pos += (uint32_t)required;
	if (self->pos > self->len) {
		self->len = self->pos;
	}
}

uint8_t ct_span_take_u8(ct_span_t *self) {
	if (ct_span_readable(self) < 1) {
		return 0;
	}
	return self->bytes[self->pos++];
}

uint16_t ct_span_take_u16(ct_span_t *self) {
	if (ct_span_readable(self) < 2) {
		return 0;
	}
	uint16_t v;
	memcpy(&v, self->bytes + self->pos, 2);
	self->pos += 2;

	if (self->endian != CT_ENDIAN_SYSTEM) {
		v = ct_binary_bswap16(v);
	}
	return v;
}

uint32_t ct_span_take_u32(ct_span_t *self) {
	if (ct_span_readable(self) < 4) {
		return 0;
	}
	uint32_t v;
	memcpy(&v, self->bytes + self->pos, 4);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x2(v);
		}
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

uint64_t ct_span_take_u64(ct_span_t *self) {
	if (ct_span_readable(self) < 8) {
		return 0;
	}
	uint64_t v;
	memcpy(&v, self->bytes + self->pos, 8);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x4(v);
		}
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

int ct_span_take_arr8(ct_span_t *self, uint8_t *out, size_t count) {
	if (count == 0) {
		return 0;
	}
	if (ct_span_readable(self) < count) {
		return -1;
	}
	memcpy(out, self->bytes + self->pos, count);
	self->pos += (uint32_t)count;
	return 0;
}

int ct_span_take_arr16(ct_span_t *self, uint16_t *out, size_t count) {
	if (count == 0) {
		return 0;
	}
	const size_t required = count * 2;
	if (ct_span_readable(self) < required) {
		return -1;
	}

	memcpy(out, self->bytes + self->pos, required);
	self->pos += (uint32_t)required;

	if (self->endian != CT_ENDIAN_SYSTEM) {
		ct_binary_bswap16_batch(out, count);
	}
	return 0;
}

int ct_span_take_arr32(ct_span_t *self, uint32_t *out, size_t count) {
	if (count == 0) {
		return 0;
	}
	const size_t required = count * 4;
	if (ct_span_readable(self) < required) {
		return -1;
	}

	memcpy(out, self->bytes + self->pos, required);
	self->pos += (uint32_t)required;

	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			ct_binary_bswap16_x2_batch(out, count);
		}
	} else {
		if (self->hlswap) {
			ct_binary_reverse_words32_batch(out, count);
		} else {
			ct_binary_bswap32_batch(out, count);
		}
	}
	return 0;
}

int ct_span_take_arr64(ct_span_t *self, uint64_t *out, size_t count) {
	if (count == 0) {
		return 0;
	}
	const size_t required = count * 8;
	if (ct_span_readable(self) < required) {
		return -1;
	}

	memcpy(out, self->bytes + self->pos, required);
	self->pos += (uint32_t)required;

	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			ct_binary_bswap16_x4_batch(out, count);
		}
	} else {
		if (self->hlswap) {
			ct_binary_reverse_words64_batch(out, count);
		} else {
			ct_binary_bswap64_batch(out, count);
		}
	}
	return 0;
}

uint8_t ct_span_peek_u8(ct_span_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 1 > (int)self->len) {
		return 0;
	}
	return self->bytes[absPos];
}

uint16_t ct_span_peek_u16(ct_span_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 2 > (int)self->len) {
		return 0;
	}
	uint16_t v;
	memcpy(&v, self->bytes + absPos, 2);
	if (self->endian != CT_ENDIAN_SYSTEM) {
		v = ct_binary_bswap16(v);
	}
	return v;
}

uint32_t ct_span_peek_u32(ct_span_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 4 > (int)self->len) {
		return 0;
	}
	uint32_t v;
	memcpy(&v, self->bytes + absPos, 4);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x2(v);
		}
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}
	return v;
}

uint64_t ct_span_peek_u64(ct_span_t *self, int offset) {
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + 8 > (int)self->len) {
		return 0;
	}
	uint64_t v;
	memcpy(&v, self->bytes + absPos, 8);
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x4(v);
		}
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}
	return v;
}

int ct_span_peek_arr8(ct_span_t *self, int offset, uint8_t *out, size_t count) {
	if (!out || count == 0) {
		return 0;
	}
	const int absPos = (int)self->pos + offset;
	if (absPos < 0 || absPos + (int)count > (int)self->len) {
		return -1;
	}
	memcpy(out, self->bytes + absPos, count);
	return 0;
}

int ct_span_peek_arr16(ct_span_t *self, int offset, uint16_t *out, size_t count) {
	if (!out || count == 0) {
		return 0;
	}
	const size_t required = count * 2;
	const int    absPos   = (int)self->pos + offset;
	if (absPos < 0 || absPos + (int)required > (int)self->len) {
		return -1;
	}

	size_t readPos = absPos;
	for (size_t i = 0; i < count; i++) {
		uint16_t v;
		memcpy(&v, self->bytes + readPos, 2);
		if (self->endian != CT_ENDIAN_SYSTEM) {
			v = ct_binary_bswap16(v);
		}
		out[i] = v;
		readPos += 2;
	}
	return 0;
}

int ct_span_peek_arr32(ct_span_t *self, int offset, uint32_t *out, size_t count) {
	if (!out || count == 0) {
		return 0;
	}
	const size_t required = count * 4;
	const int    absPos   = (int)self->pos + offset;
	if (absPos < 0 || absPos + (int)required > (int)self->len) {
		return -1;
	}

	size_t readPos = absPos;
	for (size_t i = 0; i < count; i++) {
		uint32_t v;
		memcpy(&v, self->bytes + readPos, 4);
		if (self->endian == CT_ENDIAN_SYSTEM) {
			if (self->hlswap) {
				v = ct_binary_bswap16_x2(v);
			}
		} else {
			if (self->hlswap) {
				v = ct_binary_reverse_words32(v);
			} else {
				v = ct_binary_bswap32(v);
			}
		}
		out[i] = v;
		readPos += 4;
	}
	return 0;
}

int ct_span_peek_arr64(ct_span_t *self, int offset, uint64_t *out, size_t count) {
	if (!out || count == 0) {
		return 0;
	}
	const size_t required = count * 8;
	const int    absPos   = (int)self->pos + offset;
	if (absPos < 0 || absPos + (int)required > (int)self->len) {
		return -1;
	}

	size_t readPos = absPos;
	for (size_t i = 0; i < count; i++) {
		uint64_t v;
		memcpy(&v, self->bytes + readPos, 8);
		if (self->endian == CT_ENDIAN_SYSTEM) {
			if (self->hlswap) {
				v = ct_binary_bswap16_x4(v);
			}
		} else {
			if (self->hlswap) {
				v = ct_binary_reverse_words64(v);
			} else {
				v = ct_binary_bswap64(v);
			}
		}
		out[i] = v;
		readPos += 8;
	}
	return 0;
}

int ct_span_overwrite_u8(ct_span_t *self, size_t offset, uint8_t v) {
	if (offset + 1 > self->len) {
		return -1;
	}
	self->bytes[offset] = v;
	return 0;
}

int ct_span_overwrite_u16(ct_span_t *self, size_t offset, uint16_t v) {
	if (offset + 2 > self->len) {
		return -1;
	}
	if (self->endian != CT_ENDIAN_SYSTEM) {
		v = ct_binary_bswap16(v);
	}
	memcpy(self->bytes + offset, &v, 2);
	return 0;
}

int ct_span_overwrite_u32(ct_span_t *self, size_t offset, uint32_t v) {
	if (offset + 4 > self->len) {
		return -1;
	}
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x2(v);
		}
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words32(v);
		} else {
			v = ct_binary_bswap32(v);
		}
	}
	memcpy(self->bytes + offset, &v, 4);
	return 0;
}

int ct_span_overwrite_u64(ct_span_t *self, size_t offset, uint64_t v) {
	if (offset + 8 > self->len) {
		return -1;
	}
	if (self->endian == CT_ENDIAN_SYSTEM) {
		if (self->hlswap) {
			v = ct_binary_bswap16_x4(v);
		}
	} else {
		if (self->hlswap) {
			v = ct_binary_reverse_words64(v);
		} else {
			v = ct_binary_bswap64(v);
		}
	}
	memcpy(self->bytes + offset, &v, 8);
	return 0;
}

int ct_span_overwrite_arr8(ct_span_t *self, size_t offset, const uint8_t *v, size_t count) {
	if (!v || count == 0) {
		return 0;
	}
	if (offset + count > self->len) {
		return -1;
	}
	memcpy(self->bytes + offset, v, count);
	return 0;
}

int ct_span_overwrite_arr16(ct_span_t *self, size_t offset, const uint16_t *v, size_t count) {
	if (!v || count == 0) {
		return 0;
	}
	const size_t required = count * 2;
	if (offset + required > self->len) {
		return -1;
	}

	size_t writePos = offset;
	for (size_t i = 0; i < count; i++) {
		uint16_t val = v[i];
		if (self->endian != CT_ENDIAN_SYSTEM) {
			val = ct_binary_bswap16(val);
		}
		memcpy(self->bytes + writePos, &val, 2);
		writePos += 2;
	}
	return 0;
}

int ct_span_overwrite_arr32(ct_span_t *self, size_t offset, const uint32_t *v, size_t count) {
	if (!v || count == 0) {
		return 0;
	}
	const size_t required = count * 4;
	if (offset + required > self->len) {
		return -1;
	}

	size_t writePos = offset;
	for (size_t i = 0; i < count; i++) {
		uint32_t val = v[i];
		if (self->endian == CT_ENDIAN_SYSTEM) {
			if (self->hlswap) {
				val = ct_binary_bswap16_x2(val);
			}
		} else {
			if (self->hlswap) {
				val = ct_binary_reverse_words32(val);
			} else {
				val = ct_binary_bswap32(val);
			}
		}
		memcpy(self->bytes + writePos, &val, 4);
		writePos += 4;
	}
	return 0;
}

int ct_span_overwrite_arr64(ct_span_t *self, size_t offset, const uint64_t *v, size_t count) {
	if (!v || count == 0) {
		return 0;
	}
	const size_t required = count * 8;
	if (offset + required > self->len) {
		return -1;
	}

	size_t writePos = offset;
	for (size_t i = 0; i < count; i++) {
		uint64_t val = v[i];
		if (self->endian == CT_ENDIAN_SYSTEM) {
			if (self->hlswap) {
				val = ct_binary_bswap16_x4(val);
			}
		} else {
			if (self->hlswap) {
				val = ct_binary_reverse_words64(val);
			} else {
				val = ct_binary_bswap64(val);
			}
		}
		memcpy(self->bytes + writePos, &val, 8);
		writePos += 8;
	}
	return 0;
}
