/**
 * @file bytes.h
 * @brief Fixed-size byte buffer with position tracking.
 */
#ifndef COTER_BYTES_BYTES_H
#define COTER_BYTES_BYTES_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Binary buffer with position tracking and endianness control.
 *
 * Manages a fixed-size byte buffer with read/write position tracking.
 * Supports endianness conversion and high-low word swap for multi-byte values.
 */
typedef struct ct_bytes {
    uint8_t* data_;          ///< Base address of buffer
    uint32_t cap_;           ///< Total capacity in data
    uint32_t len_;           ///< Valid data length [0, cap]
    uint32_t pos_;           ///< Current read/write position [0, len]
    uint32_t endian_ : 1;    ///< Byte order
    uint32_t hlswap_ : 1;    ///< High-low 16-bit word swap for 32/64-bit values
    uint32_t overflow_ : 1;  ///< Out-of-bounds error flag
} ct_bytes_t;

#define CT_BYTES_INIT(buf_, cap_) {(uint8_t*)(buf_), (uint32_t)(cap_), 0U, 0U, CT_ENDIAN_BIG, 0U, 0U}

#define CT_BYTES_FROM(buf_, cap_, len_) \
    {(uint8_t*)(buf_), (uint32_t)(cap_), CT_MIN((uint32_t)(len_), (uint32_t)(cap_)), 0U, CT_ENDIAN_BIG, 0U, 0U}

#define ct_bytes_capacity(self_)   ((size_t)(self_)->cap_)
#define ct_bytes_count(self_)      ((size_t)(self_)->len_)
#define ct_bytes_pos(self_)        ((size_t)(self_)->pos_)
#define ct_bytes_readable(self_)   ((size_t)((self_)->len_ - (self_)->pos_))
#define ct_bytes_writable(self_)   ((size_t)((self_)->cap_ - (self_)->pos_))
#define ct_bytes_appendable(self_) ((size_t)((self_)->cap_ - (self_)->len_))
#define ct_bytes_is_empty(self_)   ((self_)->len_ == 0U)
#define ct_bytes_is_full(self_)    ((self_)->len_ == (self_)->cap_)
#define ct_bytes_data(self_)       ((self_)->data_)
#define ct_bytes_has_error(self_)  ((self_)->overflow_)

#define ct_bytes_rewind(self_)        ((self_)->pos_ = 0U)
#define ct_bytes_clear(self_)         ((self_)->len_ = (self_)->pos_ = 0U, (self_)->overflow_ = 0U)
#define ct_bytes_clear_error(self_)   ((self_)->overflow_ = 0U)
#define ct_bytes_get_endian(self_)    ((self_)->endian_)
#define ct_bytes_set_endian(self_, byte_order_) ((self_)->endian_ = (byte_order_))
#define ct_bytes_get_hlswap(self_)              ((self_)->hlswap_)
#define ct_bytes_set_hlswap(self_, word_swap_)  ((self_)->hlswap_ = (word_swap_))

/**
 * @brief Initialize buffer with external memory.
 * @param self Buffer to initialize
 * @param data External byte array
 * @param cap Capacity in data
 */
CT_INLINE void ct_bytes_init(ct_bytes_t* self, uint8_t* data, size_t cap) {
    self->data_     = data;
    self->cap_      = (uint32_t)(cap);
    self->len_      = 0U;
    self->pos_      = 0U;
    self->endian_   = CT_ENDIAN_BIG;
    self->hlswap_   = 0U;
    self->overflow_ = 0U;
}

/**
 * @brief Initialize buffer from existing data.
 * @param self Buffer to initialize
 * @param data External byte array with existing data
 * @param cap Capacity in data
 * @param len Valid data length (must be <= cap)
 */
CT_INLINE void ct_bytes_from(ct_bytes_t* self, uint8_t* data, size_t cap, size_t len) {
    uint32_t final_cap = (uint32_t)(cap);
    uint32_t final_len = (uint32_t)(len);
    self->data_         = data;
    self->cap_          = final_cap;
    self->len_          = final_len <= final_cap ? final_len : final_cap;
    self->pos_          = 0U;
    self->endian_       = CT_ENDIAN_BIG;
    self->hlswap_       = 0U;
    self->overflow_     = 0U;
}

// Set position to absolute offset from start. Returns -1 if out of bounds.
CT_INLINE int ct_bytes_seek(ct_bytes_t* self, size_t offset) {
    if (offset > (size_t)self->len_) {
        self->overflow_ = 1;
        return -1;
    }
    self->pos_ = (uint32_t)offset;
    return 0;
}

// Set position to offset from end (pos = len - offset). Returns -1 if out of bounds.
CT_INLINE int ct_bytes_reseek(ct_bytes_t* self, size_t offset) {
    if (offset > (size_t)self->len_) {
        self->overflow_ = 1;
        return -1;
    }
    self->pos_ = self->len_ - (uint32_t)offset;
    return 0;
}

// Advance position forward. Returns actual data skipped.
CT_INLINE int ct_bytes_skip(ct_bytes_t* self, size_t length) {
    const size_t readable = ct_bytes_readable(self);
    if (length > readable) {
        self->overflow_ = 1;
        length         = readable;
    }
    self->pos_ += (uint32_t)length;
    return (int)length;
}

// Advance position and extend len if needed. Used after writing directly to data. Returns actual data committed.
CT_INLINE int ct_bytes_commit(ct_bytes_t* self, size_t length) {
    const size_t writable = ct_bytes_writable(self);
    if (length > writable) {
        self->overflow_ = 1;
        length         = writable;
    }
    self->pos_ += (uint32_t)length;
    if (self->pos_ > self->len_) { self->len_ = self->pos_; }
    return (int)length;
}

// Truncates the buffer to a smaller length, discarding data after new_len.
CT_INLINE void ct_bytes_truncate(ct_bytes_t* self, size_t new_len) {
    if (new_len >= self->len_) { return; }
    self->len_ = (uint32_t)new_len;
    if (self->pos_ > self->len_) { self->pos_ = self->len_; }
}

// Create a view buffer pointing to a range [start, end] of the original buffer.
CT_INLINE int ct_bytes_since(const ct_bytes_t* self, ct_bytes_t* since, size_t start, size_t end) {
    if (end == 0) { end = self->len_; }
    if (end < start || end > (size_t)self->cap_) { return -1; }
    since->data_     = self->data_ + start;
    since->cap_      = self->cap_ - (uint32_t)start;
    since->len_      = (uint32_t)(end - start);
    since->pos_      = 0U;
    since->endian_   = self->endian_;
    since->hlswap_   = self->hlswap_;
    since->overflow_ = 0U;
    return 0;
}
// Create view of readable portion [pos, len]
#define ct_bytes_readable_since(self_, since_) ct_bytes_since((self_), (since_), (self_)->pos_, (self_)->len_);
// Create view of writable portion [pos, cap]
#define ct_bytes_writable_since(self_, since_) ct_bytes_since((self_), (since_), (self_)->pos_, (self_)->cap_);

// Remove read data by moving unread portion to start.
CT_API void ct_bytes_compact(ct_bytes_t* self);

// Find byte in readable portion. Returns relative offset from pos, or -1 if not found.
CT_API int ct_bytes_find(const ct_bytes_t* self, uint8_t bt, size_t offset);

// Fill writable portion with byte, advancing pos. Returns actual bytes filled.
CT_API int ct_bytes_fill(ct_bytes_t* self, uint8_t bt, size_t length);

// Fill buffer memory absolutely. Does not advance pos. Typically used for zeroing.
CT_API int ct_bytes_overfill(ct_bytes_t* self, uint8_t bt, size_t length);

/**
 * @brief Get reads data from the buffer absolute offset into p.
 * @return The actual number of data read.
 */
CT_API int ct_bytes_get_bytes(const ct_bytes_t* self, size_t offset, uint8_t* p, size_t length);

/**
 * @brief Set writes data from p into the buffer absolute offset.
 * @return The actual number of data written. 0 if successful, -1 if any part is out of bounds.
 */
CT_API int ct_bytes_set_bytes(ct_bytes_t* self, size_t offset, const uint8_t* p, size_t length);

/**
 * @brief Peek reads data from the buffer at the specified relative offset from pos into p.
 * @return The actual number of data read.
 */
CT_API int ct_bytes_peek_bytes(const ct_bytes_t* self, int offset, uint8_t* p, size_t length);

/**
 * @brief Poke writes data from p into the buffer at the specified relative offset from pos.
 * @return The actual number of data written. 0 if successful, -1 if any part is out of bounds.
 */
CT_API int ct_bytes_poke_bytes(ct_bytes_t* self, int offset, const uint8_t* p, size_t length);

/**
 * @brief Take reads data from the buffer into p (advances pos).
 * @return The actual number of data read (0 if empty).
 */
CT_API int ct_bytes_take_bytes(ct_bytes_t* self, uint8_t* p, size_t length);

/**
 * @brief Put writes data from p into the buffer (advances pos).
 * @return The actual number of data written.
 */
CT_API int ct_bytes_put_bytes(ct_bytes_t* self, const uint8_t* p, size_t length);

// Write uint8_t with endianness conversion.
CT_API void ct_bytes_put_u8(ct_bytes_t* self, uint8_t v);
// Write uint16_t with endianness conversion.
CT_API void ct_bytes_put_u16(ct_bytes_t* self, uint16_t v);
// Write uint32_t with endianness conversion.
CT_API void ct_bytes_put_u32(ct_bytes_t* self, uint32_t v);
// Write uint64_t with endianness conversion.
CT_API void ct_bytes_put_u64(ct_bytes_t* self, uint64_t v);

// Read uint8_t with endianness conversion. Advances pos.
CT_API uint8_t ct_bytes_take_u8(ct_bytes_t* self);
// Read uint16_t with endianness conversion. Advances pos.
CT_API uint16_t ct_bytes_take_u16(ct_bytes_t* self);
// Read uint32_t with endianness conversion. Advances pos.
CT_API uint32_t ct_bytes_take_u32(ct_bytes_t* self);
// Read uint64_t with endianness conversion. Advances pos.
CT_API uint64_t ct_bytes_take_u64(ct_bytes_t* self);

// Peek uint8_t at pos+offset without advancing pos.
CT_API uint8_t ct_bytes_peek_u8(const ct_bytes_t* self, int offset);
// Peek uint16_t at pos+offset without advancing pos.
CT_API uint16_t ct_bytes_peek_u16(const ct_bytes_t* self, int offset);
// Peek uint32_t at pos+offset without advancing pos.
CT_API uint32_t ct_bytes_peek_u32(const ct_bytes_t* self, int offset);
// Peek uint64_t at pos+offset without advancing pos.
CT_API uint64_t ct_bytes_peek_u64(const ct_bytes_t* self, int offset);

// Read uint8_t at absolute offset.
CT_API uint8_t ct_bytes_get_u8(const ct_bytes_t* self, size_t offset);
// Read uint16_t at absolute offset.
CT_API uint16_t ct_bytes_get_u16(const ct_bytes_t* self, size_t offset);
// Read uint32_t at absolute offset.
CT_API uint32_t ct_bytes_get_u32(const ct_bytes_t* self, size_t offset);
// Read uint64_t at absolute offset.
CT_API uint64_t ct_bytes_get_u64(const ct_bytes_t* self, size_t offset);

// Set uint8_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
CT_API int ct_bytes_set_u8(ct_bytes_t* self, size_t offset, uint8_t v);
// Set uint16_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
CT_API int ct_bytes_set_u16(ct_bytes_t* self, size_t offset, uint16_t v);
// Set uint32_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
CT_API int ct_bytes_set_u32(ct_bytes_t* self, size_t offset, uint32_t v);
// Set uint64_t at absolute offset. Does not change pos or len. Returns 0 on success, -1 on out of bounds.
CT_API int ct_bytes_set_u64(ct_bytes_t* self, size_t offset, uint64_t v);

#ifdef __cplusplus
}
#endif
#endif  // coter_bytes_bytes_H
