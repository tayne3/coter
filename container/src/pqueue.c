/**
 * @file pqueue.c
 * @brief 4-ary 优先队列实现
 */
#include "coter/container/pqueue.h"

static void ct_pqueue__sift_up(ct_pqueue_t* self, uint32_t idx) {
    char* tmp = (char*)malloc(self->_item_size);
    memcpy(tmp, self->_all + (idx * self->_item_size), self->_item_size);

    while (idx > 0) {
        const uint32_t parent = (idx - 1) / 4;
        if (self->_cmp(tmp, self->_all + (parent * self->_item_size)) >= 0) { break; }
        memcpy(self->_all + (idx * self->_item_size), self->_all + (parent * self->_item_size), self->_item_size);
        idx = parent;
    }
    memcpy(self->_all + (idx * self->_item_size), tmp, self->_item_size);
    free(tmp);
}

static void ct_pqueue__sift_down(ct_pqueue_t* self, uint32_t idx) {
    char* tmp = (char*)malloc(self->_item_size);
    memcpy(tmp, self->_all + (idx * self->_item_size), self->_item_size);

    while (true) {
        uint32_t smallest = idx;
        for (uint32_t i = 1; i <= 4; ++i) {
            const uint32_t child = 4 * idx + i;
            if (child >= self->_size) { continue; }
            if (self->_cmp(self->_all + (child * self->_item_size), self->_all + (smallest * self->_item_size)) < 0) {
                smallest = child;
            }
        }

        if (smallest == idx) { break; }
        if (self->_cmp(tmp, self->_all + (smallest * self->_item_size)) <= 0) { break; }

        memcpy(self->_all + (idx * self->_item_size), self->_all + (smallest * self->_item_size), self->_item_size);
        idx = smallest;
    }
    memcpy(self->_all + (idx * self->_item_size), tmp, self->_item_size);
    free(tmp);
}

void ct_pqueue_destroy(ct_pqueue_t* self) {
    if (self->_is_dynamic && self->_all) { free(self->_all); }
    self->_all  = NULL;
    self->_cap  = 0;
    self->_size = 0;
}

bool ct_pqueue_push(ct_pqueue_t* self, const void* data) {
    if (self->_size == self->_cap) {
        if (!self->_is_dynamic) { return false; }
        if (!ct_pqueue_reserve(self, self->_cap == 0 ? 8 : self->_cap * 2)) { return false; }
    }

    memcpy(self->_all + (self->_size * self->_item_size), data, self->_item_size);
    ct_pqueue__sift_up(self, self->_size++);
    return true;
}

bool ct_pqueue_pop(ct_pqueue_t* self, void* out) {
    if (self->_size == 0) { return false; }
    if (out) { memcpy(out, self->_all, self->_item_size); }

    if (--self->_size > 0) {
        memcpy(self->_all, self->_all + (self->_size * self->_item_size), self->_item_size);
        ct_pqueue__sift_down(self, 0);
    }
    return true;
}

void* ct_pqueue_top(const ct_pqueue_t* self) {
    return self->_size == 0 ? NULL : self->_all;
}

void ct_pqueue_clear(ct_pqueue_t* self) {
    self->_size = 0;
}

bool ct_pqueue_reserve(ct_pqueue_t* self, uint32_t new_cap) {
    if (!self->_is_dynamic) { return false; }
    if (new_cap <= self->_cap) { return true; }

    char* next = (char*)realloc(self->_all, new_cap * self->_item_size);
    if (!next) { return false; }

    self->_all = next;
    self->_cap = new_cap;
    return true;
}
