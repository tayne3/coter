/**
 * @file evhub.c
 * @brief 事件中枢
 */
#include "coter/event/hub.h"

// subscriber
typedef struct {
	uint32_t            type;       // 事件类型
	ct_evhub_callback_t cb;         // 处理函数
	void               *user_data;  // 用户数据
} evhub__sub_t;

void ct_evhub_init(ct_evhub_t *self) {
	if (!self) { return; }
	ct_rwlock_init(&self->rwlock, NULL);
	ct_array_init(&self->sub_list, sizeof(evhub__sub_t), 0);
}

void ct_evhub_deinit(ct_evhub_t *self) {
	if (!self) { return; }
	ct_rwlock_destroy(&self->rwlock);
	ct_array_destroy(&self->sub_list);
}

int ct_evhub_subscribe(ct_evhub_t *self, uint32_t type, ct_evhub_callback_t cb, void *user_data) {
	if (!self || !cb) { return -1; }
	ct_rwlock_wrlock(&self->rwlock);
	evhub__sub_t sub = {
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

int ct_evhub_unsubscribe(ct_evhub_t *self, uint32_t type, ct_evhub_callback_t cb) {
	if (!self || !cb) { return -1; }
	int           ret = -1;
	evhub__sub_t *sub;
	ct_rwlock_wrlock(&self->rwlock);
	for (size_t i = 0; i < ct_array_size(&self->sub_list); ++i) {
		sub = (evhub__sub_t *)ct_array_at(&self->sub_list, i);
		if (sub && sub->type == type && (sub->cb == cb || cb == NULL)) {
			ct_array_erase(&self->sub_list, i--);
			ret = 0;
		}
	}
	ct_rwlock_wrunlock(&self->rwlock);
	return ret;
}

int ct_evhub_publish(ct_evhub_t *self, uint32_t type, void *data) {
	if (!self) { return -1; }
	evhub__sub_t *sub;
	ct_rwlock_rdlock(&self->rwlock);
	for (size_t i = 0; i < ct_array_size(&self->sub_list); ++i) {
		sub = (evhub__sub_t *)ct_array_at(&self->sub_list, i);
		if (sub && sub->type == type) { sub->cb(type, data, sub->user_data); }
	}
	ct_rwlock_rdunlock(&self->rwlock);
	return 0;
}
