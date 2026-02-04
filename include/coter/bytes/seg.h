#ifndef COTER_BYTES_SEG_H
#define COTER_BYTES_SEG_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Binary buffer with position tracking and endianness control.
 *
 * Manages a fixed-size byte buffer with read/write position tracking.
 * Supports endianness conversion and high-low word swap for multi-byte values.
 */
typedef struct ct_seg {
	uint8_t *bytes;       ///< Base address of buffer
	uint32_t cap;         ///< Total capacity in bytes
	uint32_t len;         ///< Valid data length [0, cap]
	uint32_t pos;         ///< Current read/write position [0, len]
	uint32_t endian : 1;  ///< Byte order
	uint32_t hlswap : 1;  ///< High-low 16-bit word swap for 32/64-bit values
} ct_seg_t;

#define CT_SEG_INIT(__b, __cap)      \
	{                                \
		.bytes  = (uint8_t *)(__b),  \
		.cap    = (uint32_t)(__cap), \
		.len    = 0U,                \
		.pos    = 0U,                \
		.endian = CT_ENDIAN_BIG,     \
		.hlswap = 0U,                \
	}

#define CT_SEG_FROM(__b, __cap, __len) \
	{                                  \
		.bytes  = (uint8_t *)(__b),    \
		.cap    = (uint32_t)(__cap),   \
		.len    = (uint32_t)(__len),   \
		.pos    = 0U,                  \
		.endian = CT_ENDIAN_BIG,       \
		.hlswap = 0U,                  \
	}

#define ct_seg_capacity(self)   ((size_t)(self)->cap)
#define ct_seg_count(self)      ((size_t)(self)->len)
#define ct_seg_pos(self)        ((size_t)(self)->pos)
#define ct_seg_readable(self)   ((size_t)((self)->len - (self)->pos))
#define ct_seg_writable(self)   ((size_t)((self)->cap - (self)->pos))
#define ct_seg_appendable(self) ((size_t)((self)->cap - (self)->len))
#define ct_seg_is_empty(self)   ((self)->len == 0U)
#define ct_seg_is_full(self)    ((self)->len == (self)->cap)
#define ct_seg_data(self)       ((self)->bytes)

#define ct_seg_get_endian(self)    ((self)->endian)
#define ct_seg_set_endian(self, e) ((self)->endian = (e))
#define ct_seg_get_hlswap(self)    ((self)->hlswap)
#define ct_seg_set_hlswap(self, h) ((self)->hlswap = (h))
#define ct_seg_rewind(self)        ((self)->pos = 0U)
#define ct_seg_clear(self)         ((self)->len = (self)->pos = 0U)

/**
 * @brief Initialize buffer with external memory.
 * @param self Buffer to initialize
 * @param bytes External byte array
 * @param cap Capacity in bytes
 */
static inline void ct_seg_init(ct_seg_t *self, uint8_t *bytes, size_t cap) {
	self->bytes  = bytes;
	self->cap    = (uint32_t)cap;
	self->len    = 0U;
	self->pos    = 0U;
	self->endian = CT_ENDIAN_BIG;
	self->hlswap = 0U;
}

/**
 * @brief Initialize buffer from existing data.
 * @param self Buffer to initialize
 * @param bytes External byte array with existing data
 * @param cap Capacity in bytes
 * @param len Valid data length (must be <= cap)
 */
static inline void ct_seg_from(ct_seg_t *self, uint8_t *bytes, size_t cap, size_t len) {
	self->bytes  = bytes;
	self->cap    = (uint32_t)cap;
	self->len    = (uint32_t)(len <= cap ? len : cap);
	self->pos    = 0U;
	self->endian = CT_ENDIAN_BIG;
	self->hlswap = 0U;
}

// Set position to absolute offset from start. Returns -1 if out of bounds.
static inline int ct_seg_seek(ct_seg_t *self, size_t offset) {
	if (self->len < (uint32_t)offset) { return -1; }
	self->pos = (uint32_t)offset;
	return 0;
}

// Set position to offset from end (pos = len - offset). Returns -1 if out of bounds.
static inline int ct_seg_reseek(ct_seg_t *self, size_t offset) {
	if (self->len < (uint32_t)offset) { return -1; }
	self->pos = self->len - (uint32_t)offset;
	return 0;
}

// Advance position forward. Returns actual bytes skipped.
static inline int ct_seg_skip(ct_seg_t *self, size_t length) {
	const size_t readable = ct_seg_readable(self);
	if (length > readable) { length = readable; }
	self->pos += (uint32_t)length;
	return (int)length;
}

// Advance position and extend len if needed. Used after writing directly to bytes. Returns actual bytes committed.
static inline int ct_seg_commit(ct_seg_t *self, size_t length) {
	const size_t writable = ct_seg_writable(self);
	if (length > writable) { length = writable; }
	self->pos += (uint32_t)length;
	if (self->pos > self->len) { self->len = self->pos; }
	return (int)length;
}

// Create a view buffer pointing to a range [start, end] of the original buffer.
static inline int ct_seg_since(ct_seg_t *self, ct_seg_t *since, size_t start, size_t end) {
	if (end == 0) { end = self->len; }
	if (end < start || end > (size_t)self->cap) { return -1; }
	since->bytes  = self->bytes + start;
	since->cap    = self->cap - (uint32_t)start;
	since->len    = (uint32_t)(end - start);
	since->pos    = 0U;
	since->endian = self->endian;
	since->hlswap = self->hlswap;
	return 0;
}
// Create view of readable portion [pos, len]
#define ct_seg_readable_since(self, since) ct_seg_since((self), (since), (self)->pos, (self)->len);
// Create view of writable portion [pos, cap]
#define ct_seg_writable_since(self, since) ct_seg_since((self), (since), (self)->pos, (self)->cap);

// Remove read data by moving unread portion to start.
COTER_API void ct_seg_compact(ct_seg_t *self);

/**
 * @brief Peek reads data from the current position into p without advancing the position.
 * @return The actual number of bytes read.
 */
COTER_API int ct_seg_peek(const ct_seg_t *self, uint8_t *p, size_t length);

/**
 * @brief Fill fills the buffer with byte bt for the specified length.
 * @return The actual number of bytes filled.
 */
COTER_API int ct_seg_fill(ct_seg_t *self, uint8_t bt, size_t length);

/**
 * @brief Read reads data from the buffer into p (advances pos).
 * @return The actual number of bytes read (0 if empty).
 */
COTER_API int ct_seg_read(ct_seg_t *self, uint8_t *p, size_t length);

/**
 * @brief Write writes data from p into the buffer (advances pos).
 * @return The actual number of bytes written (0 if full).
 */
COTER_API int ct_seg_write(ct_seg_t *self, const uint8_t *p, size_t length);

// Write uint8_t with endianness conversion.
COTER_API void ct_seg_put_u8(ct_seg_t *self, uint8_t v);
// Write uint16_t with endianness conversion.
COTER_API void ct_seg_put_u16(ct_seg_t *self, uint16_t v);
// Write uint32_t with endianness conversion.
COTER_API void ct_seg_put_u32(ct_seg_t *self, uint32_t v);
// Write uint64_t with endianness conversion.
COTER_API void ct_seg_put_u64(ct_seg_t *self, uint64_t v);
// Write uint8_t array with endianness conversion.
COTER_API void ct_seg_put_arr8(ct_seg_t *self, const uint8_t *v, size_t count);
// Write uint16_t array with endianness conversion.
COTER_API void ct_seg_put_arr16(ct_seg_t *self, const uint16_t *v, size_t count);
// Write uint32_t array with endianness conversion.
COTER_API void ct_seg_put_arr32(ct_seg_t *self, const uint32_t *v, size_t count);
// Write uint64_t array with endianness conversion.
COTER_API void ct_seg_put_arr64(ct_seg_t *self, const uint64_t *v, size_t count);

// Read uint8_t with endianness conversion. Advances pos.
COTER_API uint8_t ct_seg_take_u8(ct_seg_t *self);
// Read uint16_t with endianness conversion. Advances pos.
COTER_API uint16_t ct_seg_take_u16(ct_seg_t *self);
// Read uint32_t with endianness conversion. Advances pos.
COTER_API uint32_t ct_seg_take_u32(ct_seg_t *self);
// Read uint64_t with endianness conversion. Advances pos.
COTER_API uint64_t ct_seg_take_u64(ct_seg_t *self);
// Read uint8_t array with endianness conversion. Advances pos. Returns 0 on success, -1 on insufficient data.
COTER_API int ct_seg_take_arr8(ct_seg_t *self, uint8_t *out, size_t count);
// Read uint16_t array with endianness conversion. Advances pos. Returns 0 on success, -1 on insufficient data.
COTER_API int ct_seg_take_arr16(ct_seg_t *self, uint16_t *out, size_t count);
// Read uint32_t array with endianness conversion. Advances pos. Returns 0 on success, -1 on insufficient data.
COTER_API int ct_seg_take_arr32(ct_seg_t *self, uint32_t *out, size_t count);
// Read uint64_t array with endianness conversion. Advances pos. Returns 0 on success, -1 on insufficient data.
COTER_API int ct_seg_take_arr64(ct_seg_t *self, uint64_t *out, size_t count);

// Peek uint8_t at pos+offset without advancing pos.
COTER_API uint8_t ct_seg_peek_u8(const ct_seg_t *self, int offset);
// Peek uint16_t at pos+offset without advancing pos.
COTER_API uint16_t ct_seg_peek_u16(const ct_seg_t *self, int offset);
// Peek uint32_t at pos+offset without advancing pos.
COTER_API uint32_t ct_seg_peek_u32(const ct_seg_t *self, int offset);
// Peek uint64_t at pos+offset without advancing pos.
COTER_API uint64_t ct_seg_peek_u64(const ct_seg_t *self, int offset);
// Peek uint8_t array at pos+offset without advancing pos. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_peek_arr8(const ct_seg_t *self, int offset, uint8_t *out, size_t count);
// Peek uint16_t array at pos+offset without advancing pos. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_peek_arr16(const ct_seg_t *self, int offset, uint16_t *out, size_t count);
// Peek uint32_t array at pos+offset without advancing pos. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_peek_arr32(const ct_seg_t *self, int offset, uint32_t *out, size_t count);
// Peek uint64_t array at pos+offset without advancing pos. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_peek_arr64(const ct_seg_t *self, int offset, uint64_t *out, size_t count);

// Overwrite uint8_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_u8(ct_seg_t *self, size_t offset, uint8_t v);
// Overwrite uint16_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_u16(ct_seg_t *self, size_t offset, uint16_t v);
// Overwrite uint32_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_u32(ct_seg_t *self, size_t offset, uint32_t v);
// Overwrite uint64_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_u64(ct_seg_t *self, size_t offset, uint64_t v);
// Overwrite uint8_t array at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_arr8(ct_seg_t *self, size_t offset, const uint8_t *v, size_t count);
// Overwrite uint16_t array at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_arr16(ct_seg_t *self, size_t offset, const uint16_t *v, size_t count);
// Overwrite uint32_t array at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_arr32(ct_seg_t *self, size_t offset, const uint32_t *v, size_t count);
// Overwrite uint64_t array at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
COTER_API int ct_seg_overwrite_arr64(ct_seg_t *self, size_t offset, const uint64_t *v, size_t count);

#ifdef __cplusplus
}
#endif
#endif  // COTER_BYTES_SEG_H
