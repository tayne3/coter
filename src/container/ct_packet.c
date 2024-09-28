/**
 * @file ct_packet.c
 * @brief 报文缓冲盒子
 * @author tayne3@dingtalk.com
 * @date 2023.12.08
 */
#include "ct_packet.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_packet_init(ct_packet_buf_t self, uint8_t *_buffer, uint16_t _max)
{
	assert(self);
	self->_buffer = _buffer;
	self->_total  = 0;
	self->_past   = 0;
	self->_max    = _max;
}

void ct_packet_reset(ct_packet_buf_t self)
{
	assert(self);
	self->_total = 0;
	self->_past  = 0;
}

void ct_packet_clean(ct_packet_buf_t self)
{
	assert(self);
	assert(self->_buffer);
	memset(self->_buffer, 0, self->_total);
	self->_total = 0;
	self->_past  = 0;
}

uint8_t ct_packet_get_u8(const ct_packet_buf_t self, uint16_t offset)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 1 <= self->_total);

	if (offset + self->_past + 1 > self->_total) {
		return 0;
	}

	return self->_buffer[offset + self->_past];
}

uint16_t ct_packet_get_u16(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 2 <= self->_total);

	if (offset + self->_past + 2 > self->_total) {
		return 0;
	}

	uint16_t       value;
	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_Big) {
		value = ((uint16_t)source[0] << 8) | (uint16_t)source[1];
	} else {
		value = ((uint16_t)source[1] << 8) | (uint16_t)source[0];
	}
	return value;
}

uint32_t ct_packet_get_u32(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 4 <= self->_total);

	if (offset + self->_past + 4 > self->_total) {
		return 0;
	}

	uint32_t       value;
	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_Big) {
		value = ((uint32_t)source[0] << 24) | ((uint32_t)source[1] << 16) | ((uint32_t)source[2] << 8) |
				(uint32_t)source[3];
	} else {
		value = ((uint32_t)source[3] << 24) | ((uint32_t)source[2] << 16) | ((uint32_t)source[1] << 8) |
				(uint32_t)source[0];
	}
	return value;
}

float ct_packet_get_float(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 4 <= self->_total);

	if (offset + self->_past + 4 > self->_total) {
		return 0;
	}

	union {
		uint32_t u32;
		float    f;
	} v;

	const uint8_t *source = self->_buffer + self->_past + offset;
	if (endian == CTEndian_Big) {
		v.u32 = ((uint32_t)source[0] << 24) | ((uint32_t)source[1] << 16) | ((uint32_t)source[2] << 8) |
				(uint32_t)source[3];
	} else {
		v.u32 = ((uint32_t)source[3] << 24) | ((uint32_t)source[2] << 16) | ((uint32_t)source[1] << 8) |
				(uint32_t)source[0];
	}
	return v.f;
}

uint8_t ct_packet_take_u8(ct_packet_buf_t self)
{
	assert(self);
	assert(self->_buffer);
	assert(self->_past + 1 <= self->_total);

	if (self->_past + 1 > self->_total) {
		return 0;
	}

	const uint8_t value = self->_buffer[self->_past];
	self->_past += 1;
	return value;
}

uint16_t ct_packet_take_u16(ct_packet_buf_t self, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(self->_past + 2 <= self->_total);

	if (self->_past + 2 > self->_total) {
		return 0;
	}

	uint16_t       value;
	const uint8_t *source = self->_buffer + self->_past;
	if (endian == CTEndian_Big) {
		value = ((uint16_t)source[0] << 8) | (uint16_t)source[1];
	} else {
		value = ((uint16_t)source[1] << 8) | (uint16_t)source[0];
	}
	self->_past += 2;
	return value;
}

uint32_t ct_packet_take_u32(ct_packet_buf_t self, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(self->_past + 4 <= self->_total);

	if (self->_past + 4 > self->_total) {
		return 0;
	}

	uint32_t       value;
	const uint8_t *source = self->_buffer + self->_past;
	if (endian == CTEndian_Big) {
		value = ((uint32_t)source[0] << 24) | ((uint32_t)source[1] << 16) | ((uint32_t)source[2] << 8) |
				(uint32_t)source[3];
	} else {
		value = ((uint32_t)source[3] << 24) | ((uint32_t)source[2] << 16) | ((uint32_t)source[1] << 8) |
				(uint32_t)source[0];
	}
	self->_past += 4;
	return value;
}

float ct_packet_take_float(ct_packet_buf_t self, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(self->_past + 4 <= self->_total);

	if (self->_past + 4 > self->_total) {
		return 0;
	}

	union {
		uint32_t u32;
		float    f;
	} v;

	const uint8_t *source = self->_buffer + self->_past;
	if (endian == CTEndian_Big) {
		v.u32 = ((uint32_t)source[0] << 24) | ((uint32_t)source[1] << 16) | ((uint32_t)source[2] << 8) |
				(uint32_t)source[3];
	} else {
		v.u32 = ((uint32_t)source[3] << 24) | ((uint32_t)source[2] << 16) | ((uint32_t)source[1] << 8) |
				(uint32_t)source[0];
	}
	self->_past += 4;
	return v.f;
}

void ct_packet_skip(ct_packet_buf_t self, uint16_t length)
{
	assert(self);
	assert(self->_buffer);
	assert(self->_past + length <= self->_total);

	if (self->_past + length > self->_total) {
		return;
	}

	self->_past += length;
}

void ct_packet_set_u8(ct_packet_buf_t self, uint16_t offset, uint8_t value)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 1 <= self->_max);

	if (offset + self->_past + 1 > self->_max) {
		return;  // 超出缓冲区范围
	}

	uint8_t *target = self->_buffer + self->_past;
	*target         = value;

	if (self->_total < offset + self->_past + 1) {
		self->_total = offset + self->_past + 1;
	}
}

void ct_packet_set_u16(ct_packet_buf_t self, uint16_t offset, uint16_t value, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 2 <= self->_max);

	if (offset + self->_past + 2 > self->_max) {
		return;  // 超出缓冲区范围
	}

	uint8_t *target = self->_buffer + self->_past;
	if (endian == CTEndian_Big) {
		*target++ = (uint8_t)(value >> 8);
		*target   = (uint8_t)(value);
	} else {
		*target++ = (uint8_t)(value);
		*target   = (uint8_t)(value >> 8);
	}

	if (self->_total < offset + self->_past + 2) {
		self->_total = offset + self->_past + 2;
	}
}

void ct_packet_set_u32(ct_packet_buf_t self, uint16_t offset, uint32_t value, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 4 <= self->_max);

	if (offset + self->_past + 4 > self->_max) {
		return;  // 超出缓冲区范围
	}

	uint8_t *target = self->_buffer + self->_past;
	if (endian == CTEndian_Big) {
		*target++ = (uint8_t)(value >> 24);
		*target++ = (uint8_t)(value >> 16);
		*target++ = (uint8_t)(value >> 8);
		*target   = (uint8_t)(value);
	} else {
		*target++ = (uint8_t)(value);
		*target++ = (uint8_t)(value >> 8);
		*target++ = (uint8_t)(value >> 16);
		*target   = (uint8_t)(value >> 24);
	}

	if (self->_total < offset + self->_past + 4) {
		self->_total = offset + self->_past + 4;
	}
}

void ct_packet_set_float(ct_packet_buf_t self, uint16_t offset, float value, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);
	assert(offset + self->_past + 4 <= self->_max);

	if (offset + self->_past + 4 > self->_max) {
		return;  // 超出缓冲区范围
	}

	union {
		uint32_t u32;
		float    f;
	} v = {
		.f = value,
	};

	uint8_t *target = self->_buffer + self->_past;
	if (endian == CTEndian_Big) {
		*target++ = (uint8_t)(v.u32 >> 24);
		*target++ = (uint8_t)(v.u32 >> 16);
		*target++ = (uint8_t)(v.u32 >> 8);
		*target   = (uint8_t)(v.u32);
	} else {
		*target++ = (uint8_t)(v.u32);
		*target++ = (uint8_t)(v.u32 >> 8);
		*target++ = (uint8_t)(v.u32 >> 16);
		*target   = (uint8_t)(v.u32 >> 24);
	}

	if (self->_total < offset + self->_past + 4) {
		self->_total = offset + self->_past + 4;
	}
}

void ct_packet_put_u8(ct_packet_buf_t self, uint8_t value)
{
	assert(self);
	assert(self->_buffer);

	if (self->_total + 1 > self->_max) {
		return;
	}

	uint8_t *target = self->_buffer + self->_total;
	*target         = value;

	self->_total += 1;
}

void ct_packet_put_u16(ct_packet_buf_t self, uint16_t value, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if (self->_total + 2 > self->_max) {
		return;
	}

	uint8_t *target = self->_buffer + self->_total;
	if (endian == CTEndian_Big) {
		*target++ = (uint8_t)(value >> 8);
		*target   = (uint8_t)(value);
	} else {
		*target++ = (uint8_t)(value);
		*target   = (uint8_t)(value >> 8);
	}

	self->_total += 2;
}

void ct_packet_put_u32(ct_packet_buf_t self, uint32_t value, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if (self->_total + 4 > self->_max) {
		return;
	}

	uint8_t *target = self->_buffer + self->_total;
	if (endian == CTEndian_Big) {
		*target++ = (uint8_t)(value >> 24);
		*target++ = (uint8_t)(value >> 16);
		*target++ = (uint8_t)(value >> 8);
		*target   = (uint8_t)(value);
	} else {
		*target++ = (uint8_t)(value);
		*target++ = (uint8_t)(value >> 8);
		*target++ = (uint8_t)(value >> 16);
		*target   = (uint8_t)(value >> 24);
	}

	self->_total += 4;
}

void ct_packet_put_float(ct_packet_buf_t self, float value, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if (self->_total + 4 > self->_max) {
		return;
	}

	union {
		uint32_t u32;
		float    f;
	} v = {
		.f = value,
	};

	uint8_t *target = self->_buffer + self->_total;
	if (endian == CTEndian_Big) {
		*target++ = (uint8_t)(v.u32 >> 24);
		*target++ = (uint8_t)(v.u32 >> 16);
		*target++ = (uint8_t)(v.u32 >> 8);
		*target   = (uint8_t)(v.u32);
	} else {
		*target++ = (uint8_t)(v.u32);
		*target++ = (uint8_t)(v.u32 >> 8);
		*target++ = (uint8_t)(v.u32 >> 16);
		*target   = (uint8_t)(v.u32 >> 24);
	}

	self->_total += 4;
}

void ct_packet_over(ct_packet_buf_t self)
{
	assert(self);
	assert(self->_buffer);

	self->_past = self->_total;
}

uint16_t ct_packet_get_u8s(const ct_packet_buf_t self, uint16_t offset, uint8_t *buffer, uint16_t max)
{
	assert(self);
	assert(self->_buffer);

	if (offset + self->_past >= self->_total) {
		return 0;
	}

	if (max + offset + self->_past > self->_total) {
		max = self->_total - offset - self->_past;
	}
	memcpy(buffer, self->_buffer + self->_past + offset, max);
	return max;
}

uint16_t ct_packet_get_u16s(const ct_packet_buf_t self, uint16_t offset, uint16_t *buffer, uint16_t max,
							 ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if (offset + self->_past >= self->_total) {
		return 0;
	}

	if ((max << 1) + offset + self->_past > self->_total) {
		max = (self->_total - offset - self->_past) >> 1;
	}

	const uint8_t *source = self->_buffer + self->_past + offset;
	for (uint16_t i = 0; i < max; i++) {
		const uint16_t byte1 = (uint16_t)*source++;
		const uint16_t byte2 = (uint16_t)*source++;
		if (endian == CTEndian_Big) {
			buffer[i] = (byte1 << 8) | byte2;
		} else {
			buffer[i] = (byte2 << 8) | byte1;
		}
	}

	return max;  // 返回读取的元素数量
}

uint16_t ct_packet_get_u32s(const ct_packet_buf_t self, uint16_t offset, uint32_t *buffer, uint16_t max,
							 ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if (offset + self->_past >= self->_total) {
		return 0;
	}

	if ((max << 2) + offset + self->_past > self->_total) {
		max = (self->_total - offset - self->_past) >> 2;
	}

	const uint8_t *source = self->_buffer + self->_past + offset;
	for (uint16_t i = 0; i < max; i++) {
		const uint32_t byte1 = *source++;
		const uint32_t byte2 = *source++;
		const uint32_t byte3 = *source++;
		const uint32_t byte4 = *source++;
		if (endian == CTEndian_Big) {
			buffer[i] = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
		} else {
			buffer[i] = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | byte1;
		}
	}

	return max;  // 返回读取的元素数量
}

uint16_t ct_packet_get_floats(const ct_packet_buf_t self, uint16_t offset, float *buffer, uint16_t max,
							   ct_endian_t endian)
{
	return ct_packet_get_u32s(self, offset, (uint32_t *)buffer, max, endian);
}

uint16_t ct_packet_take_u8s(ct_packet_buf_t self, uint8_t *buffer, uint16_t max)
{
	assert(self);
	assert(self->_buffer);

	if (self->_past >= self->_total) {
		return 0;
	}

	if (max + self->_past > self->_total) {
		max = self->_total - self->_past;
	}

	memcpy(buffer, self->_buffer + self->_past, max);

	self->_past += max;
	return max;  // 返回取出的元素数量
}

uint16_t ct_packet_take_u16s(ct_packet_buf_t self, uint16_t *buffer, uint16_t max, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if (self->_past >= self->_total) {
		return 0;
	}

	if ((max << 1) + self->_past > self->_total) {
		max = (self->_total - self->_past) >> 1;
	}

	const uint8_t *source = self->_buffer + self->_past;
	for (uint16_t i = 0; i < max; i++) {
		const uint16_t byte1 = (uint16_t)*source++;
		const uint16_t byte2 = (uint16_t)*source++;
		if (endian == CTEndian_Big) {
			buffer[i] = (byte1 << 8) | byte2;
		} else {
			buffer[i] = (byte2 << 8) | byte1;
		}
	}

	self->_past += max << 1;
	return max;  // 返回取出的元素数量
}

uint16_t ct_packet_take_u32s(ct_packet_buf_t self, uint32_t *buffer, uint16_t max, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if (self->_past >= self->_total) {
		return 0;
	}

	if ((max << 2) + self->_past > self->_total) {
		max = (self->_total - self->_past) >> 2;
	}

	const uint8_t *source = self->_buffer + self->_past;
	for (uint16_t i = 0; i < max; i++) {
		const uint32_t byte1 = *source++;
		const uint32_t byte2 = *source++;
		const uint32_t byte3 = *source++;
		const uint32_t byte4 = *source++;
		if (endian == CTEndian_Big) {
			buffer[i] = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
		} else {
			buffer[i] = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | byte1;
		}
	}

	self->_past += max << 2;
	return max;  // 返回取出的元素数量
}

uint16_t ct_packet_take_floats(ct_packet_buf_t self, float *buffer, uint16_t max, ct_endian_t endian)
{
	return ct_packet_take_u32s(self, (uint32_t *)buffer, max, endian);
}

uint16_t ct_packet_put_u8s(ct_packet_buf_t self, const uint8_t *buffer, uint16_t length)
{
	assert(self);
	assert(self->_buffer);

	if (length + self->_total > self->_max) {
		length = self->_max - self->_total;
	}

	memcpy(self->_buffer + self->_total, buffer, length);
	self->_total += length;
	return length;
}

uint16_t ct_packet_put_u16s(ct_packet_buf_t self, const uint16_t *buffer, uint16_t length, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if ((length << 1) + self->_total > self->_max) {
		length = (self->_max - self->_total) >> 1;
	}

	uint8_t *target = self->_buffer + self->_total;
	for (uint16_t i = 0; i < length; i++) {
		const uint16_t value = buffer[i];
		if (endian == CTEndian_Big) {
			*target++ = (uint8_t)(value >> 8);
			*target++ = (uint8_t)(value);
		} else {
			*target++ = (uint8_t)(value);
			*target++ = (uint8_t)(value >> 8);
		}
	}

	self->_total += length << 1;
	return length;  // 返回写入的元素数量
}

uint16_t ct_packet_put_u32s(ct_packet_buf_t self, const uint32_t *buffer, uint16_t length, ct_endian_t endian)
{
	assert(self);
	assert(self->_buffer);

	if ((length << 2) + self->_total > self->_max) {
		length = (self->_max - self->_total) >> 2;
	}

	uint8_t *target = self->_buffer + self->_total;
	for (uint16_t i = 0; i < length; i++) {
		const uint32_t value = buffer[i];
		if (endian == CTEndian_Big) {
			*target++ = (uint8_t)(value >> 24);
			*target++ = (uint8_t)(value >> 16);
			*target++ = (uint8_t)(value >> 8);
			*target++ = (uint8_t)(value);
		} else {
			*target++ = (uint8_t)(value);
			*target++ = (uint8_t)(value >> 8);
			*target++ = (uint8_t)(value >> 16);
			*target++ = (uint8_t)(value >> 24);
		}
	}

	self->_total += length << 2;
	return length;  // 返回写入的元素数量
}

uint16_t ct_packet_put_floats(ct_packet_buf_t self, const float *buffer, uint16_t length, ct_endian_t endian)
{
	return ct_packet_put_u32s(self, (const uint32_t *)buffer, length, endian);
}

// -------------------------[STATIC DEFINITION]-------------------------
