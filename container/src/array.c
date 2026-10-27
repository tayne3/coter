/**
 * @file array.c
 * @brief Generic dynamic array implementation.
 */
#include "coter/container/array.h"

#include <stdlib.h>

int ct_array_init(ct_array_t* self, size_t byte, size_t capacity) {
    if (!self || !byte) { return -1; }
    self->ptr_  = NULL;
    self->byte_ = byte;
    self->cap_  = 0;
    self->size_ = 0;
    if (capacity > 0 && !vector_reserve((void**)&self->ptr_, &self->cap_, self->byte_, capacity)) { return -1; }
    return 0;
}

void ct_array_destroy(ct_array_t* self) {
    if (!self) { return; }
    if (self->ptr_) {
        free(self->ptr_);
        self->ptr_ = NULL;
    }
    self->byte_ = self->cap_ = self->size_ = 0;
}

void ct_array_clear(ct_array_t* self) {
    if (!self) { return; }
    self->size_ = 0;
}

size_t ct_array_capacity(const ct_array_t* self) {
    return self ? self->cap_ : 0;
}

size_t ct_array_size(const ct_array_t* self) {
    return self ? self->size_ : 0;
}

bool ct_array_empty(const ct_array_t* self) {
    return !self || self->size_ == 0;
}

bool ct_array_reserve(ct_array_t* self, size_t capacity) {
    if (!self) { return false; }
    return vector_reserve((void**)&self->ptr_, &self->cap_, self->byte_, capacity);
}

bool ct_array_resize(ct_array_t* self, size_t new_size) {
    if (!self) { return false; }
    return vector_resize((void**)&self->ptr_, &self->size_, &self->cap_, self->byte_, new_size);
}

bool ct_array_shrink(ct_array_t* self) {
    if (!self) { return false; }
    return vector_shrink((void**)&self->ptr_, self->size_, &self->cap_, self->byte_);
}

bool ct_array_insert(ct_array_t* self, size_t idx, const void* data) {
    if (!self) { return false; }
    return vector_insert((void**)&self->ptr_, &self->size_, &self->cap_, self->byte_, idx, data);
}

bool ct_array_push(ct_array_t* self, const void* data) {
    if (!self) { return false; }
    return vector_insert((void**)&self->ptr_, &self->size_, &self->cap_, self->byte_, self->size_, data);
}

bool ct_array_erase(ct_array_t* self, size_t idx) {
    if (!self) { return false; }
    return vector_erase(self->ptr_, &self->size_, self->byte_, idx);
}

bool ct_array_pop(ct_array_t* self) {
    if (!self || !self->ptr_ || self->size_ == 0) { return false; }
    --self->size_;
    return true;
}

void* ct_array_at(ct_array_t* self, size_t idx) {
    return !self || !self->ptr_ || idx >= self->size_ ? NULL : self->ptr_ + idx * self->byte_;
}

const void* ct_array_value(const ct_array_t* self, size_t idx) {
    return !self || !self->ptr_ || idx >= self->size_ ? NULL : self->ptr_ + idx * self->byte_;
}

void* ct_array_front(ct_array_t* self) {
    return !self || !self->ptr_ || self->size_ == 0 ? NULL : self->ptr_;
}

void* ct_array_back(ct_array_t* self) {
    return !self || !self->ptr_ || self->size_ == 0 ? NULL : self->ptr_ + (self->size_ - 1) * self->byte_;
}
