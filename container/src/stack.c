/**
 * @file stack.c
 * @brief LIFO 栈
 */
#include "coter/container/stack.h"

int ct_stack_init(ct_stack_buf_t self, void* buffer, size_t byte, size_t max) {
    if (!self || !byte) { return -1; }
    self->_all  = (char*)buffer;
    self->_byte = byte;
    self->_max  = max;
    ct_stack_clear(self);
    return 0;
}

bool ct_stack_push(ct_stack_buf_t self, const void* item) {
    if (!self || ct_stack_isfull(self)) { return false; }
    memcpy(self->_all + self->_size * self->_byte, item, self->_byte);
    self->_size++;
    return true;
}

bool ct_stack_pop(ct_stack_buf_t self, void* item) {
    if (!self || ct_stack_isempty(self)) { return false; }
    self->_size--;
    if (item) { memcpy(item, self->_all + self->_size * self->_byte, self->_byte); }
    return true;
}

bool ct_stack_top(ct_stack_buf_t self, void* item) {
    if (!self || ct_stack_isempty(self)) { return false; }
    if (item) { memcpy(item, self->_all + (self->_size - 1) * self->_byte, self->_byte); }
    return true;
}
