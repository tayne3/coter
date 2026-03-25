#ifndef COTER_BYTES_RBUF_H
#define COTER_BYTES_RBUF_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A byte-oriented ring buffer mapped over an externally provided memory chunk.
 *
 * This structure supports two primary usage patterns:
 * 1. Standard I/O: using `ct_rbuf_write()` and `ct_rbuf_read()`.
 * 2. Zero-copy I/O: acquiring a writable/readable memory span via `ct_rbuf_write_ptr()`
 *    or `ct_rbuf_read_ptr()`, followed by `ct_rbuf_commit()` or `ct_rbuf_remove()`.
 *
 * @note This component is not thread-safe.
 */
typedef struct ct_rbuf {
    uint8_t* data;  ///< Pointer to the externally allocated backing memory.
    size_t   cap;   ///< Total buffer capacity in bytes.
    size_t   head;  ///< Read cursor. Points to the oldest readable byte.
    size_t   tail;  ///< Write cursor. Points to the next writable byte.
    size_t   len;   ///< Current number of readable bytes in the buffer.
} ct_rbuf_t;

#define CT_RBUF_INIT(__buf, __cap) {(uint8_t*)(__buf), (__cap), 0, 0, 0}

#define ct_rbuf_capacity(self) ((self)->cap)
#define ct_rbuf_count(self)    ((self)->len)
#define ct_rbuf_remain(self)   ((self)->cap - (self)->len)
#define ct_rbuf_is_empty(self) ((self)->len == 0)
#define ct_rbuf_is_full(self)  ((self)->len == (self)->cap)
#define ct_rbuf_clear(self)    ((self)->len = (self)->head = (self)->tail = 0)

/**
 * @brief Binds a ring buffer struct to an existing chunk of memory.
 *
 * @param self   The ring buffer instance to initialize.
 * @param buffer Pointer to the externally allocated backing memory.
 * @param cap    Total capacity of the buffer in bytes.
 */
CT_INLINE void ct_rbuf_init(ct_rbuf_t* self, uint8_t* buffer, size_t cap) {
    if (!self) { return; }
    self->data = buffer;
    self->cap  = cap;
    self->head = 0;
    self->tail = 0;
    self->len  = 0;
}

/**
 * @brief Copies up to `len` bytes from `src` into the ring buffer.
 *
 * @param self The ring buffer instance.
 * @param src  Pointer to the source data to be copied.
 * @param len  The requested number of bytes to write.
 * @return     The actual number of bytes successfully written.
 */
CT_API size_t ct_rbuf_write(ct_rbuf_t* self, const uint8_t* src, size_t len);

/**
 * @brief Copies and consumes up to `len` bytes from the ring buffer into `dst`.
 *
 * @param self The ring buffer instance.
 * @param dst  Pointer to the destination buffer where data will be copied.
 * @param len  The requested number of bytes to read.
 * @return     The actual number of bytes successfully read and consumed.
 */
CT_API size_t ct_rbuf_read(ct_rbuf_t* self, uint8_t* dst, size_t len);

/**
 * @brief Copies up to `len` bytes into `dst` WITHOUT consuming the readable data.
 *
 * @param self The ring buffer instance.
 * @param dst  Pointer to the destination buffer where peeked data will be copied.
 * @param len  The requested number of bytes to peek.
 * @return     The actual number of bytes successfully peeked.
 */
CT_API size_t ct_rbuf_peek(const ct_rbuf_t* self, uint8_t* dst, size_t len);

/**
 * @brief Provides direct zero-copy access to the readable data.
 *
 * Due to wrap-around, this returns only the FIRST CONTIGUOUS chunk of readable memory.
 * It does not consume the data. You MUST call `ct_rbuf_remove()` after processing
 * to actually consume and free up the buffer space.
 *
 * @param self      The ring buffer instance.
 * @param chunk_len Output pointer that receives the length of the contiguous span returned.
 * @return          A read-only pointer to the contiguous readable data, or NULL if empty.
 */
CT_INLINE const uint8_t* ct_rbuf_read_ptr(const ct_rbuf_t* self, size_t* chunk_len) {
    if (!self || self->len == 0) {
        if (chunk_len) { *chunk_len = 0; }
        return NULL;
    }

    if (chunk_len) {
        const size_t until_end = self->cap - self->head;
        if (self->len <= until_end) {
            *chunk_len = self->len;
        } else {
            *chunk_len = until_end;
        }
    }
    return &self->data[self->head];
}

/**
 * @brief Provides direct zero-copy access to the writable space.
 *
 * Due to wrap-around, this returns only the FIRST CONTIGUOUS chunk of free memory.
 * It does not make the data readable. You MUST call `ct_rbuf_commit()` after writing
 * to legitimately publish the data into the buffer.
 *
 * @param self      The ring buffer instance.
 * @param chunk_len Output pointer that receives the length of the contiguous space returned.
 * @return          A pointer to the contiguous writable space, or NULL if full.
 */
CT_INLINE uint8_t* ct_rbuf_write_ptr(ct_rbuf_t* self, size_t* chunk_len) {
    if (!self) {
        if (chunk_len) { *chunk_len = 0; }
        return NULL;
    }

    const size_t writable = self->cap - self->len;
    if (writable == 0) {
        if (chunk_len) { *chunk_len = 0; }
        return NULL;
    }

    if (chunk_len) {
        const size_t until_end = self->cap - self->tail;
        if (writable <= until_end) {
            *chunk_len = writable;
        } else {
            *chunk_len = until_end;
        }
    }
    return &self->data[self->tail];
}

/**
 * @brief Discards (consumes) up to `len` bytes from the front of the ring buffer.
 *
 * If the requested `len` exceeds the currently readable bytes, it safely discards
 * all available bytes without overflowing.
 *
 * @param self The ring buffer instance.
 * @param len  The requested number of bytes to remove.
 * @return     The actual number of bytes removed (capped at current readable length).
 * @note       Typically used as the second step of a zero-copy read operation to
 *             confirm data processing after calling `ct_rbuf_read_ptr()`.
 */
CT_INLINE size_t ct_rbuf_remove(ct_rbuf_t* self, size_t len) {
    if (!self) { return 0; }
    if (len > self->len) { len = self->len; }
    if (len > 0) {
        self->head += len;
        if (self->head >= self->cap) { self->head -= self->cap; }
        self->len -= len;
    }
    return len;
}

/**
 * @brief Commits up to `len` recently written bytes, making them readable data.
 *
 * If the requested `len` exceeds the currently available writable space, it safely
 * limits the commit size to prevent overflow.
 *
 * @param self The ring buffer instance.
 * @param len  The requested number of bytes to commit.
 * @return     The actual number of bytes committed (capped at current writable space).
 * @note       Typically used as the second step of a zero-copy write operation to
 *             publish the data written into the span from `ct_rbuf_write_ptr()`.
 */
CT_INLINE size_t ct_rbuf_commit(ct_rbuf_t* self, size_t len) {
    if (!self) { return 0; }
    if (len + self->len > self->cap) { len = self->cap - self->len; }
    if (len > 0) {
        self->tail += len;
        if (self->tail >= self->cap) { self->tail -= self->cap; }
        self->len += len;
    }
    return len;
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_BYTES_RBUF_H
