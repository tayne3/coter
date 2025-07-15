/**
 * @file ct_packet.c
 * @brief 报文缓冲盒子
 * @author tayne3@dingtalk.com
 */
#include "coter/container/packet.h"

#include "coter/base/utils.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_packet_init(ct_packet_buf_t self, uint8_t *_buffer, uint16_t _max) {
	assert(self);
	self->_buffer = _buffer;
	self->_total  = 0;
	self->_past   = 0;
	self->_max    = _max;
}

void ct_packet_reset(ct_packet_buf_t self) {
	assert(self);
	self->_total = 0;
	self->_past  = 0;
}

void ct_packet_clean(ct_packet_buf_t self) {
	assert(self);
	assert(self->_buffer);
	memset(self->_buffer, 0, self->_total);
	self->_total = 0;
	self->_past  = 0;
}

uint8_t ct_packet_get_u8(const ct_packet_buf_t self, uint16_t offset) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint8_t) <= self->_total);

	if (offset + self->_past + sizeof(uint8_t) > self->_total) {
		return 0;
	}

	return self->_buffer[offset + self->_past];
}

uint16_t ct_packet_get_u16(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint16_t) <= self->_total);

	if (offset + self->_past + sizeof(uint16_t) > self->_total) {
		return 0;
	}

	uint16_t       u16;
	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_System) {
		memcpy(&u16, source, sizeof(uint16_t));
	} else {
		ct_reverse_memcpy(&u16, source, sizeof(uint16_t));
	}

	return u16;
}

uint32_t ct_packet_get_u32(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint32_t) <= self->_total);

	if (offset + self->_past + sizeof(uint32_t) > self->_total) {
		return 0;
	}

	uint32_t       u32;
	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_System) {
		memcpy(&u32, source, sizeof(uint32_t));
	} else {
		ct_reverse_memcpy(&u32, source, sizeof(uint32_t));
	}

	return u32;
}

uint64_t ct_packet_get_u64(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint64_t) <= self->_total);

	if (offset + self->_past + sizeof(uint64_t) > self->_total) {
		return 0;
	}

	uint64_t       u64;
	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_System) {
		memcpy(&u64, source, sizeof(uint64_t));
	} else {
		ct_reverse_memcpy(&u64, source, sizeof(uint64_t));
	}

	return u64;
}

float ct_packet_get_float(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(float) <= self->_total);

	if (offset + self->_past + sizeof(float) > self->_total) {
		return 0;
	}

	float          f32;
	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_System) {
		memcpy(&f32, source, sizeof(float));
	} else {
		ct_reverse_memcpy(&f32, source, sizeof(float));
	}

	return f32;
}

double ct_packet_get_double(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(double) <= self->_total);

	if (offset + self->_past + sizeof(double) > self->_total) {
		return 0;
	}

	double         f64;
	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_System) {
		memcpy(&f64, source, sizeof(double));
	} else {
		ct_reverse_memcpy(&f64, source, sizeof(double));
	}

	return f64;
}

uint8_t ct_packet_take_u8(ct_packet_buf_t self) {
	assert(self);
	assert(self->_buffer);
	assert(self->_past + sizeof(uint8_t) <= self->_total);

	if (self->_past + sizeof(uint8_t) > self->_total) {
		return 0;
	}

	const uint8_t value = self->_buffer[self->_past];
	self->_past += sizeof(uint8_t);
	return value;
}

uint16_t ct_packet_take_u16(ct_packet_buf_t self, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(self->_past + sizeof(uint16_t) <= self->_total);

	if (self->_past + sizeof(uint16_t) > self->_total) {
		return 0;
	}

	uint16_t       u16;
	const uint8_t *source = self->_buffer + self->_past;

	if (endian == CTEndian_System) {
		memcpy(&u16, source, sizeof(uint16_t));
	} else {
		ct_reverse_memcpy(&u16, source, sizeof(uint16_t));
	}

	self->_past += sizeof(uint16_t);
	return u16;
}

uint32_t ct_packet_take_u32(ct_packet_buf_t self, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(self->_past + sizeof(uint32_t) <= self->_total);

	if (self->_past + sizeof(uint32_t) > self->_total) {
		return 0;
	}

	uint32_t       u32;
	const uint8_t *source = self->_buffer + self->_past;
	if (endian == CTEndian_System) {
		memcpy(&u32, source, sizeof(uint32_t));
	} else {
		ct_reverse_memcpy(&u32, source, sizeof(uint32_t));
	}

	self->_past += sizeof(uint32_t);
	return u32;
}

uint64_t ct_packet_take_u64(ct_packet_buf_t self, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(self->_past + sizeof(uint64_t) <= self->_total);

	if (self->_past + sizeof(uint64_t) > self->_total) {
		return 0;
	}

	uint64_t       u64;
	const uint8_t *source = self->_buffer + self->_past;

	if (endian == CTEndian_System) {
		memcpy(&u64, source, sizeof(uint64_t));
	} else {
		ct_reverse_memcpy(&u64, source, sizeof(uint64_t));
	}

	self->_past += sizeof(uint64_t);
	return u64;
}

float ct_packet_take_float(ct_packet_buf_t self, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(self->_past + sizeof(float) <= self->_total);

	if (self->_past + sizeof(float) > self->_total) {
		return 0;
	}

	float          f32;
	const uint8_t *source = self->_buffer + self->_past;
	if (endian == CTEndian_System) {
		memcpy(&f32, source, sizeof(float));
	} else {
		ct_reverse_memcpy(&f32, source, sizeof(float));
	}

	self->_past += sizeof(float);
	return f32;
}

double ct_packet_take_double(ct_packet_buf_t self, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(self->_past + sizeof(double) <= self->_total);

	if (self->_past + sizeof(double) > self->_total) {
		return 0;
	}

	double         f64;
	const uint8_t *source = self->_buffer + self->_past;
	if (endian == CTEndian_System) {
		memcpy(&f64, source, sizeof(double));
	} else {
		ct_reverse_memcpy(&f64, source, sizeof(double));
	}

	self->_past += sizeof(double);
	return f64;
}

void ct_packet_skip(ct_packet_buf_t self, uint16_t length) {
	assert(self);
	assert(self->_buffer);
	assert(self->_past + length <= self->_total);

	if (self->_past + length > self->_total) {
		return;
	}

	self->_past += length;
}

void ct_packet_set_u8(ct_packet_buf_t self, uint16_t offset, uint8_t value) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint8_t) <= self->_total);

	if (offset + self->_past + sizeof(uint8_t) > self->_total) {
		return;  // 超出缓冲区范围
	}

	uint8_t *target = self->_buffer + self->_past + offset;
	*target         = value;
}

void ct_packet_set_u16(ct_packet_buf_t self, uint16_t offset, uint16_t value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint16_t) <= self->_total);

	if (offset + self->_past + sizeof(uint16_t) > self->_total) {
		return;  // 超出缓冲区范围
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_past + offset;

	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(uint16_t));
	} else {
		ct_reverse_memcpy(target, source, sizeof(uint16_t));
	}
}

void ct_packet_set_u32(ct_packet_buf_t self, uint16_t offset, uint32_t value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint32_t) <= self->_total);

	if (offset + self->_past + sizeof(uint32_t) > self->_total) {
		return;  // 超出缓冲区范围
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_past + offset;

	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(uint32_t));
	} else {
		ct_reverse_memcpy(target, source, sizeof(uint32_t));
	}
}

void ct_packet_set_u64(ct_packet_buf_t self, uint16_t offset, uint64_t value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(uint64_t) <= self->_total);

	if (offset + self->_past + sizeof(uint64_t) > self->_total) {
		return;  // 超出缓冲区范围
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_past + offset;

	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(uint64_t));
	} else {
		ct_reverse_memcpy(target, source, sizeof(uint64_t));
	}
}

void ct_packet_set_float(ct_packet_buf_t self, uint16_t offset, float value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(float) <= self->_total);

	if (offset + self->_past + sizeof(float) > self->_total) {
		return;  // 超出缓冲区范围
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_past + offset;

	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(float));
	} else {
		ct_reverse_memcpy(target, source, sizeof(float));
	}
}

void ct_packet_set_double(ct_packet_buf_t self, uint16_t offset, double value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + sizeof(double) <= self->_total);

	if (offset + self->_past + sizeof(double) > self->_total) {
		return;  // 超出缓冲区范围
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_past + offset;
	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(double));
	} else {
		ct_reverse_memcpy(target, source, sizeof(double));
	}
}

void ct_packet_put_u8(ct_packet_buf_t self, uint8_t value) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint8_t) > self->_max) {
		return;
	}

	uint8_t *target = self->_buffer + self->_total;
	*target         = value;

	self->_total += sizeof(uint8_t);
}

void ct_packet_put_u16(ct_packet_buf_t self, uint16_t value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint16_t) > self->_max) {
		return;
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_total;
	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(uint16_t));
	} else {
		ct_reverse_memcpy(target, source, sizeof(uint16_t));
	}

	self->_total += sizeof(uint16_t);
}

void ct_packet_put_u32(ct_packet_buf_t self, uint32_t value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint32_t) > self->_max) {
		return;
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_total;
	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(uint32_t));
	} else {
		ct_reverse_memcpy(target, source, sizeof(uint32_t));
	}

	self->_total += sizeof(uint32_t);
}

void ct_packet_put_u64(ct_packet_buf_t self, uint64_t value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint64_t) > self->_max) {
		return;
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_total;
	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(uint64_t));
	} else {
		ct_reverse_memcpy(target, source, sizeof(uint64_t));
	}

	self->_total += sizeof(uint64_t);
}

void ct_packet_put_float(ct_packet_buf_t self, float value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(float) > self->_max) {
		return;
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_total;
	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(float));
	} else {
		ct_reverse_memcpy(target, source, sizeof(float));
	}

	self->_total += sizeof(float);
}

void ct_packet_put_double(ct_packet_buf_t self, double value, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(double) > self->_max) {
		return;
	}

	const uint8_t *source = (const uint8_t *)&value;
	uint8_t       *target = self->_buffer + self->_total;
	if (endian == CTEndian_System) {
		memcpy(target, source, sizeof(double));
	} else {
		ct_reverse_memcpy(target, source, sizeof(double));
	}

	self->_total += sizeof(double);
}

void ct_packet_over(ct_packet_buf_t self) {
	assert(self);
	assert(self->_buffer);

	self->_past = self->_total;
}

uint16_t ct_packet_get_u8s(const ct_packet_buf_t self, uint16_t offset, uint8_t *buffer, uint16_t max) {
	assert(self);
	assert(self->_buffer);

	if (offset + self->_past + sizeof(uint8_t) > self->_total) {
		return 0;
	}

	if (max + offset + self->_past > self->_total) {
		max = self->_total - offset - self->_past;
	}
	memcpy(buffer, self->_buffer + self->_past + offset, max);
	return max;
}

uint16_t ct_packet_get_u16s(const ct_packet_buf_t self, uint16_t offset, uint16_t *buffer, uint16_t max,
							ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (offset + self->_past + sizeof(uint16_t) > self->_total) {
		return 0;
	}

	if ((max << 1) + offset + self->_past > self->_total) {
		max = (self->_total - offset - self->_past) >> 1;
	}

	const uint16_t *source = (const uint16_t *)(self->_buffer + self->_past + offset);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < max; i++) {
			memcpy(buffer++, source++, sizeof(uint16_t));
		}
	} else {
		for (uint16_t i = 0; i < max; i++) {
			ct_reverse_memcpy(buffer++, source++, sizeof(uint16_t));
		}
	}

	return max;  // 返回读取的元素数量
}

uint16_t ct_packet_get_u32s(const ct_packet_buf_t self, uint16_t offset, uint32_t *buffer, uint16_t max,
							ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (offset + self->_past + sizeof(uint32_t) > self->_total) {
		return 0;
	}

	if ((max << 2) + offset + self->_past > self->_total) {
		max = (self->_total - offset - self->_past) >> 2;
	}

	const uint32_t *source = (const uint32_t *)(self->_buffer + self->_past + offset);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < max; i++) {
			memcpy(buffer++, source++, sizeof(uint32_t));
		}
	} else {
		for (uint16_t i = 0; i < max; i++) {
			ct_reverse_memcpy(buffer++, source++, sizeof(uint32_t));
		}
	}

	return max;  // 返回读取的元素数量
}

uint16_t ct_packet_get_u64s(const ct_packet_buf_t self, uint16_t offset, uint64_t *buffer, uint16_t max,
							ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (offset + self->_past + sizeof(uint64_t) > self->_total) {
		return 0;
	}

	if ((max << 3) + offset + self->_past > self->_total) {
		max = (self->_total - offset - self->_past) >> 3;
	}

	const uint64_t *source = (const uint64_t *)(self->_buffer + self->_past + offset);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < max; i++) {
			memcpy(buffer++, source++, sizeof(uint64_t));
		}
	} else {
		for (uint16_t i = 0; i < max; i++) {
			ct_reverse_memcpy(buffer++, source++, sizeof(uint64_t));
		}
	}

	return max;  // 返回读取的元素数量
}

uint16_t ct_packet_get_floats(const ct_packet_buf_t self, uint16_t offset, float *buffer, uint16_t max,
							  ct_endian_t endian) {
	return ct_packet_get_u32s(self, offset, (uint32_t *)buffer, max, endian);
}

uint16_t ct_packet_get_doubles(const ct_packet_buf_t self, uint16_t offset, double *buffer, uint16_t max,
							   ct_endian_t endian) {
	return ct_packet_get_u64s(self, offset, (uint64_t *)buffer, max, endian);
}

uint16_t ct_packet_take_u8s(ct_packet_buf_t self, uint8_t *buffer, uint16_t max) {
	assert(self);
	assert(self->_buffer);

	if (self->_past + sizeof(uint8_t) > self->_total) {
		return 0;
	}

	if (max + self->_past > self->_total) {
		max = self->_total - self->_past;
	}

	memcpy(buffer, self->_buffer + self->_past, max);

	self->_past += max;
	return max;  // 返回取出的元素数量
}

uint16_t ct_packet_take_u16s(ct_packet_buf_t self, uint16_t *buffer, uint16_t max, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_past + sizeof(uint16_t) > self->_total) {
		return 0;
	}

	if ((max << 1) + self->_past > self->_total) {
		max = (self->_total - self->_past) >> 1;
	}

	const uint16_t *source = (const uint16_t *)(self->_buffer + self->_past);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < max; i++) {
			memcpy(buffer++, source++, sizeof(uint16_t));
		}
	} else {
		for (uint16_t i = 0; i < max; i++) {
			ct_reverse_memcpy(buffer++, source++, sizeof(uint16_t));
		}
	}

	self->_past += max << 1;
	return max;  // 返回取出的元素数量
}

uint16_t ct_packet_take_u32s(ct_packet_buf_t self, uint32_t *buffer, uint16_t max, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_past + sizeof(uint32_t) > self->_total) {
		return 0;
	}

	if ((max << 2) + self->_past > self->_total) {
		max = (self->_total - self->_past) >> 2;
	}

	const uint32_t *source = (const uint32_t *)(self->_buffer + self->_past);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < max; i++) {
			memcpy(buffer++, source++, sizeof(uint32_t));
		}
	} else {
		for (uint16_t i = 0; i < max; i++) {
			ct_reverse_memcpy(buffer++, source++, sizeof(uint32_t));
		}
	}

	self->_past += max << 2;
	return max;  // 返回取出的元素数量
}

uint16_t ct_packet_take_u64s(ct_packet_buf_t self, uint64_t *buffer, uint16_t max, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_past + sizeof(uint64_t) > self->_total) {
		return 0;
	}

	if ((max << 3) + self->_past > self->_total) {
		max = (self->_total - self->_past) >> 3;
	}

	const uint64_t *source = (const uint64_t *)(self->_buffer + self->_past);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < max; i++) {
			memcpy(buffer++, source++, sizeof(uint64_t));
		}
	} else {
		for (uint16_t i = 0; i < max; i++) {
			ct_reverse_memcpy(buffer++, source++, sizeof(uint64_t));
		}
	}

	self->_past += max << 3;
	return max;  // 返回取出的元素数量
}

uint16_t ct_packet_take_floats(ct_packet_buf_t self, float *buffer, uint16_t max, ct_endian_t endian) {
	return ct_packet_take_u32s(self, (uint32_t *)buffer, max, endian);
}

uint16_t ct_packet_take_doubles(ct_packet_buf_t self, double *buffer, uint16_t max, ct_endian_t endian) {
	return ct_packet_take_u64s(self, (uint64_t *)buffer, max, endian);
}

uint16_t ct_packet_put_u8s(ct_packet_buf_t self, const uint8_t *buffer, uint16_t length) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint8_t) > self->_max) {
		return 0;
	}

	if (length + self->_total > self->_max) {
		length = self->_max - self->_total;
	}

	memcpy(self->_buffer + self->_total, buffer, length);
	self->_total += length;
	return length;
}

uint16_t ct_packet_put_u16s(ct_packet_buf_t self, const uint16_t *buffer, uint16_t length, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint16_t) > self->_max) {
		return 0;
	}

	if ((length << 1) + self->_total > self->_max) {
		length = (self->_max - self->_total) >> 1;
	}

	uint16_t *target = (uint16_t *)(self->_buffer + self->_total);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < length; i++) {
			memcpy(target++, buffer++, sizeof(uint16_t));
		}
	} else {
		for (uint16_t i = 0; i < length; i++) {
			ct_reverse_memcpy(target++, buffer++, sizeof(uint16_t));
		}
	}

	self->_total += length << 1;
	return length;  // 返回写入的元素数量
}

uint16_t ct_packet_put_u32s(ct_packet_buf_t self, const uint32_t *buffer, uint16_t length, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint32_t) > self->_max) {
		return 0;
	}

	if ((length << 2) + self->_total > self->_max) {
		length = (self->_max - self->_total) >> 2;
	}

	uint32_t *target = (uint32_t *)(self->_buffer + self->_total);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < length; i++) {
			memcpy(target++, buffer++, sizeof(uint32_t));
		}
	} else {
		for (uint16_t i = 0; i < length; i++) {
			ct_reverse_memcpy(target++, buffer++, sizeof(uint32_t));
		}
	}

	self->_total += length << 2;
	return length;  // 返回写入的元素数量
}

uint16_t ct_packet_put_u64s(ct_packet_buf_t self, const uint64_t *buffer, uint16_t length, ct_endian_t endian) {
	assert(self);
	assert(self->_buffer);

	if (self->_total + sizeof(uint64_t) > self->_max) {
		return 0;
	}

	if ((length << 3) + self->_total > self->_max) {
		length = (self->_max - self->_total) >> 3;
	}

	uint64_t *target = (uint64_t *)(self->_buffer + self->_total);
	if (endian == CTEndian_System) {
		for (uint16_t i = 0; i < length; i++) {
			memcpy(target++, buffer++, sizeof(uint64_t));
		}
	} else {
		for (uint16_t i = 0; i < length; i++) {
			ct_reverse_memcpy(target++, buffer++, sizeof(uint64_t));
		}
	}

	self->_total += length << 3;
	return length;  // 返回写入的元素数量
}

uint16_t ct_packet_put_floats(ct_packet_buf_t self, const float *buffer, uint16_t length, ct_endian_t endian) {
	return ct_packet_put_u32s(self, (const uint32_t *)buffer, length, endian);
}

uint16_t ct_packet_put_doubles(ct_packet_buf_t self, const double *buffer, uint16_t length, ct_endian_t endian) {
	return ct_packet_put_u64s(self, (const uint64_t *)buffer, length, endian);
}

// -------------------------[STATIC DEFINITION]-------------------------
