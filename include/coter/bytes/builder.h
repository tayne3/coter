#ifndef COTER_BYTES_BUILDER_H
#define COTER_BYTES_BUILDER_H

#include "coter/bytes/span.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dynamic byte builder with auto-grow capability.
 *
 * Manages a dynamically allocated buffer that grows automatically as needed.
 * Always owns its internal buffer and must be freed with ct_builder_destroy().
 */
typedef struct ct_builder {
	ct_span_t span;  ///< Embedded buffer for all operations
} ct_builder_t;

/**
 * @brief Create a new builder with specified initial capacity.
 * @param initial_capacity Initial buffer capacity (0 for lazy allocation)
 * @return Pointer to newly created builder, or NULL on allocation failure
 * @note
 * The builder owns its internal buffer and will grow automatically as needed.
 * Must be freed with ct_builder_destroy() to avoid memory leaks.
 */
COTER_API ct_builder_t *ct_builder_create(size_t initial_capacity);

/**
 * @brief Destroy builder and free all resources.
 * @param self Builder to destroy (can be NULL)
 */
COTER_API void ct_builder_destroy(ct_builder_t *self);

/**
 * @brief Grow buffer to ensure n more bytes can be written.
 * @param self Builder object
 * @param n Additional bytes needed beyond current length
 * @return 0 on success, -1 on allocation failure
 */
COTER_API int ct_builder_grow(ct_builder_t *self, size_t n);

/**
 * @brief Reserve total capacity.
 * @param self Builder object
 * @param capacity Minimum total capacity
 * @return 0 on success, -1 on allocation failure
 * @note
 * If current capacity >= requested capacity, this is a no-op.
 */
COTER_API int ct_builder_reserve(ct_builder_t *self, size_t capacity);

/**
 * @brief Write bytes to builder (auto-grows if needed).
 * @return Number of bytes written, or -1 on error
 */
COTER_API int ct_builder_write(ct_builder_t *self, const uint8_t *p, size_t length);

/**
 * @brief Fill builder with byte value (auto-grows if needed).
 * @return Number of bytes filled, or -1 on error
 */
COTER_API int ct_builder_fill(ct_builder_t *self, uint8_t bt, size_t length);

#define ct_builder_capacity(self)   ((size_t)(self)->span.cap)
#define ct_builder_count(self)      ((size_t)(self)->span.len)
#define ct_builder_pos(self)        ((size_t)(self)->span.pos)
#define ct_builder_readable(self)   ((size_t)((self)->span.len - (self)->span.pos))
#define ct_builder_writable(self)   ((size_t)((self)->span.cap - (self)->span.pos))
#define ct_builder_appendable(self) ((size_t)((self)->span.cap - (self)->span.len))
#define ct_builder_is_empty(self)   ((self)->span.len == 0U)
#define ct_builder_is_full(self)    ((self)->span.len == (self)->span.cap)
#define ct_builder_data(self)       ((self)->span.bytes)

#define ct_builder_get_endian(self)    ((self)->span.endian)
#define ct_builder_set_endian(self, e) ((self)->span.endian = (e))
#define ct_builder_get_hlswap(self)    ((self)->span.hlswap)
#define ct_builder_set_hlswap(self, h) ((self)->span.hlswap = (h))
#define ct_builder_rewind(self)        ((self)->span.pos = 0U)
#define ct_builder_clear(self)         ((self)->span.len = (self)->span.pos = 0U)

#define ct_builder_seek(self, offset)   ct_span_seek(&(self)->span, offset)
#define ct_builder_reseek(self, offset) ct_span_reseek(&(self)->span, offset)
#define ct_builder_skip(self, length)   ct_span_skip(&(self)->span, length)
#define ct_builder_commit(self, length) ct_span_commit(&(self)->span, length)

static inline int ct_builder_span(ct_builder_t *self, ct_span_t *span, size_t start, size_t end) {
	return ct_span_since(&self->span, span, start, end);
}
#define ct_builder_readable_span(self, s) ct_span_since(&(self)->span, (s), (self)->span.pos, (self)->span.len);
#define ct_builder_writable_span(self, s) ct_span_since(&(self)->span, (s), (self)->span.pos, (self)->span.cap);
#define ct_builder_compact(self)          ct_span_compact(&(self)->span)

#define ct_builder_peek(self, p, len) ct_span_peek(&(self)->span, p, len)
#define ct_builder_read(self, p, len) ct_span_read(&(self)->span, p, len)

#define ct_builder_take_u8(self)          ct_span_take_u8(&(self)->span)
#define ct_builder_take_u16(self)         ct_span_take_u16(&(self)->span)
#define ct_builder_take_u32(self)         ct_span_take_u32(&(self)->span)
#define ct_builder_take_u64(self)         ct_span_take_u64(&(self)->span)
#define ct_builder_take_arr8(self, o, c)  ct_span_take_arr8(&(self)->span, o, c)
#define ct_builder_take_arr16(self, o, c) ct_span_take_arr16(&(self)->span, o, c)
#define ct_builder_take_arr32(self, o, c) ct_span_take_arr32(&(self)->span, o, c)
#define ct_builder_take_arr64(self, o, c) ct_span_take_arr64(&(self)->span, o, c)

#define ct_builder_peek_u8(self, off)          ct_span_peek_u8(&(self)->span, off)
#define ct_builder_peek_u16(self, off)         ct_span_peek_u16(&(self)->span, off)
#define ct_builder_peek_u32(self, off)         ct_span_peek_u32(&(self)->span, off)
#define ct_builder_peek_u64(self, off)         ct_span_peek_u64(&(self)->span, off)
#define ct_builder_peek_arr8(self, off, o, c)  ct_span_peek_arr8(&(self)->span, off, o, c)
#define ct_builder_peek_arr16(self, off, o, c) ct_span_peek_arr16(&(self)->span, off, o, c)
#define ct_builder_peek_arr32(self, off, o, c) ct_span_peek_arr32(&(self)->span, off, o, c)
#define ct_builder_peek_arr64(self, off, o, c) ct_span_peek_arr64(&(self)->span, off, o, c)

#define ct_builder_overwrite_u8(self, off, v)       ct_span_overwrite_u8(&(self)->span, off, v)
#define ct_builder_overwrite_u16(self, off, v)      ct_span_overwrite_u16(&(self)->span, off, v)
#define ct_builder_overwrite_u32(self, off, v)      ct_span_overwrite_u32(&(self)->span, off, v)
#define ct_builder_overwrite_u64(self, off, v)      ct_span_overwrite_u64(&(self)->span, off, v)
#define ct_builder_overwrite_arr8(self, off, v, c)  ct_span_overwrite_arr8(&(self)->span, off, v, c)
#define ct_builder_overwrite_arr16(self, off, v, c) ct_span_overwrite_arr16(&(self)->span, off, v, c)
#define ct_builder_overwrite_arr32(self, off, v, c) ct_span_overwrite_arr32(&(self)->span, off, v, c)
#define ct_builder_overwrite_arr64(self, off, v, c) ct_span_overwrite_arr64(&(self)->span, off, v, c)

// Write uint8_t to builder. Auto-grows if needed.
COTER_API void ct_builder_put_u8(ct_builder_t *self, uint8_t v);
// Write uint16_t to builder. Auto-grows if needed.
COTER_API void ct_builder_put_u16(ct_builder_t *self, uint16_t v);
// Write uint32_t to builder. Auto-grows if needed.
COTER_API void ct_builder_put_u32(ct_builder_t *self, uint32_t v);
// Write uint64_t to builder. Auto-grows if needed.
COTER_API void ct_builder_put_u64(ct_builder_t *self, uint64_t v);

// Write uint8_t array to builder. Auto-grows if needed.
COTER_API void ct_builder_put_arr8(ct_builder_t *self, const uint8_t *v, size_t count);
// Write uint16_t array to builder. Auto-grows if needed.
COTER_API void ct_builder_put_arr16(ct_builder_t *self, const uint16_t *v, size_t count);
// Write uint32_t array to builder. Auto-grows if needed.
COTER_API void ct_builder_put_arr32(ct_builder_t *self, const uint32_t *v, size_t count);
// Write uint64_t array to builder. Auto-grows if needed.
COTER_API void ct_builder_put_arr64(ct_builder_t *self, const uint64_t *v, size_t count);

#ifdef __cplusplus
}
#endif

#endif  // COTER_BYTES_BUILDER_H
