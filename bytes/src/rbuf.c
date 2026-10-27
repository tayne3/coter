/**
 * @file rbuf.c
 * @brief Ring buffer implementation.
 */
#include "coter/bytes/rbuf.h"

#include <string.h>

size_t ct_rbuf_write(ct_rbuf_t* self, const uint8_t* src, size_t len) {
    if (!self || !src) { return 0; }
    if (len + self->len_ > self->cap_) { len = self->cap_ - self->len_; }
    if (len == 0) { return 0; }

    if (self->tail_ + len <= self->cap_) {
        memcpy(&self->data_[self->tail_], src, len);
        self->tail_ += len;
    } else {
        const size_t first = self->cap_ - self->tail_;
        const size_t rest  = len - first;
        memcpy(&self->data_[self->tail_], src, first);
        memcpy(self->data_, &src[first], rest);
        self->tail_ = rest;
    }

    self->len_ += len;
    if (self->tail_ == self->cap_) { self->tail_ = 0; }
    return len;
}

size_t ct_rbuf_read(ct_rbuf_t* self, uint8_t* dst, size_t len) {
    if (!self || !dst) { return 0; }
    if (len > self->len_) { len = self->len_; }
    if (len == 0) { return 0; }

    if (self->head_ + len <= self->cap_) {
        memcpy(dst, &self->data_[self->head_], len);
        self->head_ += len;
    } else {
        const size_t first = self->cap_ - self->head_;
        const size_t rest  = len - first;
        memcpy(dst, &self->data_[self->head_], first);
        memcpy(&dst[first], self->data_, rest);
        self->head_ = rest;
    }

    self->len_ -= len;
    if (self->head_ == self->cap_) { self->head_ = 0; }
    return len;
}

size_t ct_rbuf_peek(const ct_rbuf_t* self, uint8_t* dst, size_t len) {
    if (!self || !dst) { return 0; }
    if (len > self->len_) { len = self->len_; }
    if (len == 0) { return 0; }

    if (self->head_ + len <= self->cap_) {
        memcpy(dst, &self->data_[self->head_], len);
    } else {
        const size_t first = self->cap_ - self->head_;
        const size_t rest  = len - first;
        memcpy(dst, &self->data_[self->head_], first);
        memcpy(&dst[first], self->data_, rest);
    }

    return len;
}
