/**
 * @file msgbuf.h
 * @brief Fixed-capacity message buffer with intrusive queueing support.
 */
#ifndef COTER_BYTES_MSGBUF_H
#define COTER_BYTES_MSGBUF_H

#include "coter/bytes/bytes.h"
#include "coter/container/list.h"
#include "coter/core/strings.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ct_msgbuf_t
 * @brief Fixed-capacity message buffer structure.
 *
 * The buffer is allocated once with a caller-requested capacity and never
 * grows. Writes that exceed the remaining space are truncated.
 *
 * Designed with an intrusive list node to allow zero-allocation queueing.
 * By embedding the list node, a ct_msgbuf_t instance can be directly inserted
 * into any ct_list_t without requiring additional memory allocation for a wrapper node.
 */
typedef struct ct_msgbuf {
    ct_list_t list_[1];    ///< Intrusive list node for zero-allocation queueing
    char*     write_pos_;  ///< Current write position pointer
    size_t    cap_;        ///< Total capacity of the buffer
    char      buffer_[1];  ///< Flexible array member for payload
} ct_msgbuf_t;

#define ct_msgbuf_buffer(self_)    ((self_)->buffer_)
#define ct_msgbuf_size(self_)      (size_t)((self_)->write_pos_ - (self_)->buffer_)
#define ct_msgbuf_capacity(self_)  ((self_)->cap_)
#define ct_msgbuf_available(self_) (size_t)(((self_)->buffer_ + (self_)->cap_) - (self_)->write_pos_)
#define ct_msgbuf_clear(self_)     ((self_)->write_pos_ = (self_)->buffer_)
#define ct_msgbuf_is_empty(self_)  ((self_)->write_pos_ == (self_)->buffer_)
#define ct_msgbuf_is_full(self_)   ((self_)->write_pos_ == ((self_)->buffer_ + (self_)->cap_))

/**
 * @brief Format text into the remaining buffer space.
 * @note
 * When at least one byte is available, the output is truncated to
 * ct_msgbuf_available(self_) - 1 bytes and null-terminated. If the buffer is
 * full, nothing is written. The buffer never grows.
 */
#define ct_msgbuf_fmt(self_, ...)                                                          \
    do {                                                                                   \
        const size_t available_ = ct_msgbuf_available(self_);                              \
        if (available_ > 0) {                                                              \
            const int size_ = ct_snprintf_s((self_)->write_pos_, available_, __VA_ARGS__); \
            if (size_ > 0) { (self_)->write_pos_ += size_; }                               \
        }                                                                                  \
    } while (0)

/**
 * @brief Create a read/write view over the range [start, end).
 * @param self Message buffer.
 * @param seg Output view.
 * @param start Start offset in the message buffer.
 * @param end End offset in the message buffer.
 * @return 0 on success, -1 on invalid arguments, invalid range, or a view that
 *         cannot be represented by ct_bytes_t.
 * @note
 * The view does not copy data. seg->cap_ is the space from start to the end
 * of the original buffer, while seg->len_ is end - start.
 */
CT_INLINE int ct_msgbuf_seg(ct_msgbuf_t* self, ct_bytes_t* seg, size_t start, size_t end) {
    if (!self || !seg) { return -1; }
    if (end < start || end > self->cap_) { return -1; }

    const size_t view_cap = self->cap_ - start;
    if (view_cap > (size_t)UINT32_MAX) { return -1; }

    seg->data_     = (uint8_t*)self->buffer_ + start;
    seg->cap_      = (uint32_t)view_cap;
    seg->len_      = (uint32_t)(end - start);
    seg->pos_      = 0U;
    seg->endian_   = CT_ENDIAN_BIG;
    seg->hlswap_   = 0U;
    seg->overflow_ = 0U;
    return 0;
}

/**
 * @brief Create a new message buffer instance.
 * @param capacity Requested capacity in bytes; 0 selects the default (1024).
 * @return Pointer to the newly allocated message buffer, or NULL on failure.
 * @note The buffer is allocated once and never grows.
 */
CT_API ct_msgbuf_t* ct_msgbuf_create(size_t capacity);

/**
 * @brief Destroy a message buffer instance and free its memory.
 * @param self Pointer to the message buffer.
 */
CT_API void ct_msgbuf_destroy(ct_msgbuf_t* self);

/**
 * @brief Write data to the message buffer.
 * @param self Pointer to the message buffer.
 * @param data Source data to write.
 * @param length Length of the data in bytes.
 * @return Number of bytes successfully written.
 * @note
 * At most ct_msgbuf_available(self) bytes are written; excess data is
 * truncated and the buffer is not resized.
 */
CT_API size_t ct_msgbuf_write(ct_msgbuf_t* self, const void* data, size_t length);

#ifdef __cplusplus
}
#endif
#endif  // COTER_BYTES_MSGBUF_H
