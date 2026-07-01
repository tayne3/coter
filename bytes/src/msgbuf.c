/**
 * @file msgbuf.c
 * @brief Implementation of the dynamic message buffer.
 */
#include "coter/bytes/msgbuf.h"

#include <stdlib.h>
#include <string.h>

ct_msgbuf_t* ct_msgbuf_create(size_t capacity) {
    capacity          = (capacity ? capacity : 1024UL);
    ct_msgbuf_t* self = (ct_msgbuf_t*)malloc(sizeof(ct_msgbuf_t) - 1 + capacity);
    if (self == NULL) { return NULL; }

    ct_list_init(self->list);
    self->write_pos = self->buffer;
    self->cap       = capacity;
    return self;
}

void ct_msgbuf_destroy(ct_msgbuf_t* self) {
    if (!self) { return; }
    free(self);
}

size_t ct_msgbuf_write(ct_msgbuf_t* self, const void* data, size_t length) {
    if (!self || !data || !length) { return 0; }

    const size_t available_space = ct_msgbuf_available(self);
    if (length > available_space) { length = available_space; }
    if (length == 0) { return 0; }

    memcpy(self->write_pos, data, length);
    self->write_pos += length;
    return length;
}
