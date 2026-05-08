/**
 * @file pubsub.c
 * @brief 发布订阅
 */
#include "coter/sync/pubsub.h"

// subscriber
typedef struct {
    uint32_t             type;       // 事件类型
    ct_pubsub_callback_t cb;         // 处理函数
    void*                user_data;  // 用户数据
} pubsub_sub_t;

void ct_pubsub_init(ct_pubsub_t* self) {
    if (!self) { return; }
    ct_rwlock_init(&self->rwlock);
    ct_array_init(&self->sub_list, sizeof(pubsub_sub_t), 0);
}

void ct_pubsub_destroy(ct_pubsub_t* self) {
    if (!self) { return; }
    ct_rwlock_destroy(&self->rwlock);
    ct_array_destroy(&self->sub_list);
}

int ct_pubsub_subscribe(ct_pubsub_t* self, uint32_t type, ct_pubsub_callback_t cb, void* user_data) {
    if (!self || !cb) { return -1; }
    ct_rwlock_wrlock(&self->rwlock);
    pubsub_sub_t sub = {
        .type      = type,
        .cb        = cb,
        .user_data = user_data,
    };
    if (!ct_array_push(&self->sub_list, &sub)) {
        ct_rwlock_wrunlock(&self->rwlock);
        return -1;
    }
    ct_rwlock_wrunlock(&self->rwlock);
    return 0;
}

int ct_pubsub_unsubscribe(ct_pubsub_t* self, uint32_t type, ct_pubsub_callback_t cb) {
    if (!self || !cb) { return -1; }
    int           ret = -1;
    pubsub_sub_t* sub;
    ct_rwlock_wrlock(&self->rwlock);
    for (size_t i = 0; i < ct_array_size(&self->sub_list); ++i) {
        sub = (pubsub_sub_t*)ct_array_at(&self->sub_list, i);
        if (sub && sub->type == type && (sub->cb == cb || cb == NULL)) {
            ct_array_erase(&self->sub_list, i--);
            ret = 0;
        }
    }
    ct_rwlock_wrunlock(&self->rwlock);
    return ret;
}

int ct_pubsub_publish(ct_pubsub_t* self, uint32_t type, void* data) {
    if (!self) { return -1; }
    pubsub_sub_t* sub;
    ct_rwlock_rdlock(&self->rwlock);
    for (size_t i = 0; i < ct_array_size(&self->sub_list); ++i) {
        sub = (pubsub_sub_t*)ct_array_at(&self->sub_list, i);
        if (sub && sub->type == type) { sub->cb(type, data, sub->user_data); }
    }
    ct_rwlock_rdunlock(&self->rwlock);
    return 0;
}
