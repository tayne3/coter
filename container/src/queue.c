/**
 * @file queue.c
 * @brief FIFO queue implementation.
 */
#include "coter/container/queue.h"

#include <string.h>

#define CT_QUEUE_INDEX_INC(self_, idx_) ((idx_) + 1 >= (self_)->max_ ? 0 : (idx_) + 1)  // 队列-索引 递增
#define CT_QUEUE_INDEX_DEC(self_, idx_) ((idx_) == 0 ? (self_)->max_ - 1 : (idx_) - 1)  // 队列-索引 递减
#define CT_QUEUE_ITEM(self_, idx_)    (&(self_)->all_[(idx_) * (self_)->byte_])   // 队列-数据

void ct_queue_init(ct_queue_t* self, void* buffer, size_t byte, size_t max) {
    if (!self || !buffer || !byte) { return; }

    self->all_  = buffer;
    self->byte_ = byte;
    self->max_  = max;

    ct_queue_clear(self);
}

bool ct_queue_enqueue(ct_queue_t* self, const void* item) {
    if (!self || !self->byte_ || !item) { return false; }

    if (ct_queue_is_full(self)) { return false; }
    memcpy(CT_QUEUE_ITEM(self, self->tail_), item, self->byte_);
    self->tail_ = CT_QUEUE_INDEX_INC(self, self->tail_);
    self->size_++;
    return true;
}

bool ct_queue_dequeue(ct_queue_t* self, void* item) {
    if (!self || !self->byte_ || !item) { return false; }

    if (ct_queue_is_empty(self)) { return false; }
    memcpy(item, CT_QUEUE_ITEM(self, self->head_), self->byte_);
    self->head_ = CT_QUEUE_INDEX_INC(self, self->head_);
    self->size_--;
    return true;
}

bool ct_queue_head(ct_queue_t* self, void* item) {
    if (!self || !self->byte_ || !item) { return false; }

    if (ct_queue_is_empty(self)) { return false; }
    memcpy(item, CT_QUEUE_ITEM(self, self->head_), self->byte_);
    return true;
}

int ct_queue_traverse(ct_queue_t* self, int (*callback)(void* item, void* arg), void* arg) {
    if (!self || !self->byte_ || !callback) { return -1; }

    int rc = 0;
    for (size_t i = 0, idx = self->head_; i < self->size_; ++i) {
        if ((rc = callback(CT_QUEUE_ITEM(self, idx), arg)) != 0) { break; }
        idx = CT_QUEUE_INDEX_INC(self, idx);
    }
    return rc;
}
