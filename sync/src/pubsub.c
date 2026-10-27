/**
 * @file pubsub.c
 * @brief Thread-safe publish/subscribe implementation.
 */
#include "coter/sync/pubsub.h"

/** @brief Internal subscriber record. */
typedef struct {
    uint32_t             type_;      /**< Event type. */
    ct_pubsub_callback_t cb_;        /**< Event callback. */
    void*                user_data_; /**< Callback context. */
} pubsub_sub_t;

void ct_pubsub_init(ct_pubsub_t* self) {
    if (!self) { return; }
    ct_rwlock_init(&self->rwlock_);
    ct_array_init(&self->sub_list_, sizeof(pubsub_sub_t), 0);
}

void ct_pubsub_destroy(ct_pubsub_t* self) {
    if (!self) { return; }
    ct_rwlock_destroy(&self->rwlock_);
    ct_array_destroy(&self->sub_list_);
}

int ct_pubsub_subscribe(ct_pubsub_t* self, uint32_t type, ct_pubsub_callback_t cb, void* user_data) {
    if (!self || !cb) { return -1; }
    ct_rwlock_wrlock(&self->rwlock_);
    pubsub_sub_t sub = {
        .type_      = type,
        .cb_        = cb,
        .user_data_ = user_data,
    };
    if (!ct_array_push(&self->sub_list_, &sub)) {
        ct_rwlock_wrunlock(&self->rwlock_);
        return -1;
    }
    ct_rwlock_wrunlock(&self->rwlock_);
    return 0;
}

int ct_pubsub_unsubscribe(ct_pubsub_t* self, uint32_t type, ct_pubsub_callback_t cb) {
    if (!self || !cb) { return -1; }
    int           ret = -1;
    pubsub_sub_t* sub;
    ct_rwlock_wrlock(&self->rwlock_);
    for (size_t i = 0; i < ct_array_size(&self->sub_list_); ++i) {
        sub = (pubsub_sub_t*)ct_array_at(&self->sub_list_, i);
        if (sub && sub->type_ == type && (sub->cb_ == cb || cb == NULL)) {
            ct_array_erase(&self->sub_list_, i--);
            ret = 0;
        }
    }
    ct_rwlock_wrunlock(&self->rwlock_);
    return ret;
}

int ct_pubsub_publish(ct_pubsub_t* self, uint32_t type, void* data) {
    if (!self) { return -1; }
    pubsub_sub_t* sub;
    ct_rwlock_rdlock(&self->rwlock_);
    for (size_t i = 0; i < ct_array_size(&self->sub_list_); ++i) {
        sub = (pubsub_sub_t*)ct_array_at(&self->sub_list_, i);
        if (sub && sub->type_ == type) { sub->cb_(type, data, sub->user_data_); }
    }
    ct_rwlock_rdunlock(&self->rwlock_);
    return 0;
}
