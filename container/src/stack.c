/**
 * @file stack.c
 * @brief LIFO stack implementation.
 */
#include "coter/container/stack.h"

#include <string.h>

int ct_stack_init(ct_stack_buf_t self, void* buffer, size_t byte, size_t max) {
    if (!self || !byte) { return -1; }
    self->all_  = (char*)buffer;
    self->byte_ = byte;
    self->max_  = max;
    ct_stack_clear(self);
    return 0;
}

bool ct_stack_push(ct_stack_buf_t self, const void* item) {
    if (!self || ct_stack_isfull(self)) { return false; }
    memcpy(self->all_ + self->size_ * self->byte_, item, self->byte_);
    self->size_++;
    return true;
}

bool ct_stack_pop(ct_stack_buf_t self, void* item) {
    if (!self || ct_stack_isempty(self)) { return false; }
    self->size_--;
    if (item) { memcpy(item, self->all_ + self->size_ * self->byte_, self->byte_); }
    return true;
}

bool ct_stack_top(ct_stack_buf_t self, void* item) {
    if (!self || ct_stack_isempty(self)) { return false; }
    if (item) { memcpy(item, self->all_ + (self->size_ - 1) * self->byte_, self->byte_); }
    return true;
}
