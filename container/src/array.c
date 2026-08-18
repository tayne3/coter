#include "coter/container/array.h"

#include <stdlib.h>

int ct_array_init(ct_array_t* self, size_t byte, size_t capacity) {
    if (!self || !byte) { return -1; }
    self->_ptr  = NULL;
    self->_byte = byte;
    self->_cap  = 0;
    self->_size = 0;
    if (capacity > 0 && !ct_vector__reserve((void**)&self->_ptr, &self->_cap, self->_byte, capacity)) { return -1; }
    return 0;
}

void ct_array_destroy(ct_array_t* self) {
    if (!self) { return; }
    if (self->_ptr) {
        free(self->_ptr);
        self->_ptr = NULL;
    }
    self->_byte = self->_cap = self->_size = 0;
}

void ct_array_clear(ct_array_t* self) {
    if (!self) { return; }
    self->_size = 0;
}

size_t ct_array_capacity(const ct_array_t* self) {
    return self ? self->_cap : 0;
}

size_t ct_array_size(const ct_array_t* self) {
    return self ? self->_size : 0;
}

bool ct_array_empty(const ct_array_t* self) {
    return !self || self->_size == 0;
}

bool ct_array_reserve(ct_array_t* self, size_t capacity) {
    if (!self) { return false; }
    return ct_vector__reserve((void**)&self->_ptr, &self->_cap, self->_byte, capacity);
}

bool ct_array_resize(ct_array_t* self, size_t new_size) {
    if (!self) { return false; }
    return ct_vector__resize((void**)&self->_ptr, &self->_size, &self->_cap, self->_byte, new_size);
}

bool ct_array_shrink(ct_array_t* self) {
    if (!self) { return false; }
    return ct_vector__shrink((void**)&self->_ptr, self->_size, &self->_cap, self->_byte);
}

bool ct_array_insert(ct_array_t* self, size_t idx, const void* data) {
    if (!self) { return false; }
    return ct_vector__insert((void**)&self->_ptr, &self->_size, &self->_cap, self->_byte, idx, data);
}

bool ct_array_push(ct_array_t* self, const void* data) {
    if (!self) { return false; }
    return ct_vector__insert((void**)&self->_ptr, &self->_size, &self->_cap, self->_byte, self->_size, data);
}

bool ct_array_erase(ct_array_t* self, size_t idx) {
    if (!self) { return false; }
    return ct_vector__erase(self->_ptr, &self->_size, self->_byte, idx);
}

bool ct_array_pop(ct_array_t* self) {
    if (!self || !self->_ptr || self->_size == 0) { return false; }
    --self->_size;
    return true;
}

void* ct_array_at(ct_array_t* self, size_t idx) {
    return !self || !self->_ptr || idx >= self->_size ? NULL : self->_ptr + idx * self->_byte;
}

const void* ct_array_value(const ct_array_t* self, size_t idx) {
    return !self || !self->_ptr || idx >= self->_size ? NULL : self->_ptr + idx * self->_byte;
}

void* ct_array_front(ct_array_t* self) {
    return !self || !self->_ptr || self->_size == 0 ? NULL : self->_ptr;
}

void* ct_array_back(ct_array_t* self) {
    return !self || !self->_ptr || self->_size == 0 ? NULL : self->_ptr + (self->_size - 1) * self->_byte;
}
