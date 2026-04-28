/**
 * @file bytes.c
 * @brief 字节数组
 */
#include "coter/bytes/bytes.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_bytes_t* ct_bytes_create(size_t capacity) {
    capacity         = (capacity ? capacity : 1024UL);
    ct_bytes_t* self = (ct_bytes_t*)malloc(sizeof(ct_bytes_t) - 1 + capacity);
    if (self == NULL) { return NULL; }

    ct_list_init(self->list);
    self->write_pos = self->buffer;
    self->cap       = capacity;
    return self;
}

void ct_bytes_destroy(ct_bytes_t* self) {
    if (!self) { return; }
    free(self);
}

size_t ct_bytes_write(ct_bytes_t* self, const void* data, size_t length) {
    if (!self || !data || !length) { return 0; }

    const size_t available_space = ct_bytes_available(self);
    if (length > available_space) { length = available_space; }
    if (length == 0) { return 0; }

    memcpy(self->write_pos, data, length);
    self->write_pos += length;
    return length;
}

// -------------------------[STATIC DEFINITION]-------------------------
