/**
 * @file msgbuf.h
 * @brief Dynamic message buffer with intrusive queueing support.
 */
#ifndef COTER_BYTES_MSGBUF_H
#define COTER_BYTES_MSGBUF_H

#include "coter/bytes/bytes.h"
#include "coter/container/list.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ct_msgbuf_t
 * @brief Dynamic message buffer structure.
 *
 * Designed with an intrusive list node to allow zero-allocation queueing.
 * By embedding the list node, a ct_msgbuf_t instance can be directly inserted
 * into any ct_list_t without requiring additional memory allocation for a wrapper node.
 */
typedef struct ct_msgbuf {
    ct_list_t list[1];    ///< Intrusive list node for zero-allocation queueing
    char*     write_pos;  ///< Current write position pointer
    size_t    cap;        ///< Total capacity of the buffer
    char      buffer[1];  ///< Flexible array member for payload
} ct_msgbuf_t;

#define ct_msgbuf_buffer(self)    ((self)->buffer)
#define ct_msgbuf_size(self)      (size_t)((self)->write_pos - (self)->buffer)
#define ct_msgbuf_capacity(self)  ((self)->cap)
#define ct_msgbuf_available(self) (size_t)(((self)->buffer + (self)->cap) - (self)->write_pos)
#define ct_msgbuf_clear(self)     ((self)->write_pos = (self)->buffer)
#define ct_msgbuf_isempty(self)   ((self)->write_pos == (self)->buffer)
#define ct_msgbuf_isfull(self)    ((self)->write_pos == ((self)->buffer + (self)->cap))

#define ct_msgbuf_fmt(self, ...)                                                          \
    do {                                                                                 \
        const size_t _available = ct_msgbuf_available(self);                              \
        if (_available > 0) {                                                            \
            const int _size = ct_snprintf_s((self)->write_pos, _available, __VA_ARGS__); \
            if (_size > 0) {                                                             \
                (self)->write_pos += _size;                                              \
            }                                                                            \
        }                                                                                \
    } while (0)

CT_INLINE int ct_msgbuf_seg(ct_msgbuf_t* self, ct_bytes_t* seg, size_t start, size_t end) {
    if (end < start || end > (size_t)self->cap) { return -1; }
    seg->data   = (uint8_t*)self->buffer + start;
    seg->cap    = self->cap - (uint32_t)start;
    seg->len    = (uint32_t)(end - start);
    seg->pos    = 0U;
    seg->endian = CT_ENDIAN_BIG;
    seg->hlswap = 0U;
    return 0;
}

/**
 * @brief Create a new message buffer instance.
 * @param capacity Requested capacity (defaults to 1024 if 0).
 * @return Pointer to the newly allocated message buffer, or NULL on failure.
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
 */
CT_API size_t ct_msgbuf_write(ct_msgbuf_t* self, const void* data, size_t length);

#ifdef __cplusplus
}
#endif
#endif  // COTER_BYTES_MSGBUF_H
