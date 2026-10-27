/**
 * @file pqueue.c
 * @brief Four-ary priority queue implementation.
 */
#include "coter/container/pqueue.h"

#include <stdlib.h>
#include <string.h>

static void pqueue_sift_up(ct_pqueue_t* self, uint32_t idx) {
    char* tmp = (char*)malloc(self->item_size_);
    memcpy(tmp, self->all_ + (idx * self->item_size_), self->item_size_);

    while (idx > 0) {
        const uint32_t parent = (idx - 1) / 4;
        if (self->cmp_(tmp, self->all_ + (parent * self->item_size_)) >= 0) { break; }
        memcpy(self->all_ + (idx * self->item_size_), self->all_ + (parent * self->item_size_), self->item_size_);
        idx = parent;
    }
    memcpy(self->all_ + (idx * self->item_size_), tmp, self->item_size_);
    free(tmp);
}

static void pqueue_sift_down(ct_pqueue_t* self, uint32_t idx) {
    char* tmp = (char*)malloc(self->item_size_);
    memcpy(tmp, self->all_ + (idx * self->item_size_), self->item_size_);

    while (true) {
        uint32_t smallest = idx;
        for (uint32_t i = 1; i <= 4; ++i) {
            const uint32_t child = 4 * idx + i;
            if (child >= self->size_) { continue; }
            if (self->cmp_(self->all_ + (child * self->item_size_), self->all_ + (smallest * self->item_size_)) < 0) {
                smallest = child;
            }
        }

        if (smallest == idx) { break; }
        if (self->cmp_(tmp, self->all_ + (smallest * self->item_size_)) <= 0) { break; }

        memcpy(self->all_ + (idx * self->item_size_), self->all_ + (smallest * self->item_size_), self->item_size_);
        idx = smallest;
    }
    memcpy(self->all_ + (idx * self->item_size_), tmp, self->item_size_);
    free(tmp);
}

void ct_pqueue_destroy(ct_pqueue_t* self) {
    if (self->is_dynamic_ && self->all_) { free(self->all_); }
    self->all_  = NULL;
    self->cap_  = 0;
    self->size_ = 0;
}

bool ct_pqueue_push(ct_pqueue_t* self, const void* data) {
    if (self->size_ == self->cap_) {
        if (!self->is_dynamic_) { return false; }
        if (!ct_pqueue_reserve(self, self->cap_ == 0 ? 8 : self->cap_ * 2)) { return false; }
    }

    memcpy(self->all_ + (self->size_ * self->item_size_), data, self->item_size_);
    pqueue_sift_up(self, self->size_++);
    return true;
}

bool ct_pqueue_pop(ct_pqueue_t* self, void* out) {
    if (self->size_ == 0) { return false; }
    if (out) { memcpy(out, self->all_, self->item_size_); }

    if (--self->size_ > 0) {
        memcpy(self->all_, self->all_ + (self->size_ * self->item_size_), self->item_size_);
        pqueue_sift_down(self, 0);
    }
    return true;
}

void* ct_pqueue_top(const ct_pqueue_t* self) {
    return self->size_ == 0 ? NULL : self->all_;
}

void ct_pqueue_clear(ct_pqueue_t* self) {
    self->size_ = 0;
}

bool ct_pqueue_reserve(ct_pqueue_t* self, uint32_t new_cap) {
    if (!self->is_dynamic_) { return false; }
    if (new_cap <= self->cap_) { return true; }

    char* next = (char*)realloc(self->all_, new_cap * self->item_size_);
    if (!next) { return false; }

    self->all_ = next;
    self->cap_ = new_cap;
    return true;
}
