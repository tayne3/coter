/**
 * @file evhub.c
 * @brief 事件中枢
 */
#include "coter/mech/evhub.h"

// subscriber
typedef struct {
	uint32_t            type;       // 事件类型
	ct_evhub_callback_t cb;         // 处理函数
	void               *user_data;  // 用户数据
} evhub__sub_t;

void ct_evhub_init(ct_evhub_t *self) {
	assert(self);
	pthread_rwlock_init(&self->rwlock, NULL);
	ct_vector_init(&self->sub_list, sizeof(evhub__sub_t), 0);
}

void ct_evhub_deinit(ct_evhub_t *self) {
	assert(self);
	pthread_rwlock_destroy(&self->rwlock);
	ct_vector_destroy(&self->sub_list);
}

int ct_evhub_subscribe(ct_evhub_t *self, uint32_t type, ct_evhub_callback_t cb, void *user_data) {
	assert(self);
	if (!cb) {
		return -1;
	}
	pthread_rwlock_wrlock(&self->rwlock);
	evhub__sub_t sub = {
		.type      = type,
		.cb        = cb,
		.user_data = user_data,
	};
	if (!ct_vector_push(&self->sub_list, &sub)) {
		pthread_rwlock_unlock(&self->rwlock);
		return -1;
	}
	pthread_rwlock_unlock(&self->rwlock);
	return 0;
}

int ct_evhub_unsubscribe(ct_evhub_t *self, uint32_t type, ct_evhub_callback_t cb) {
	assert(self);
	if (!cb) {
		return -1;
	}
	int           ret = -1;
	evhub__sub_t *sub;
	pthread_rwlock_wrlock(&self->rwlock);
	for (size_t i = 0; i < ct_vector_size(&self->sub_list); ++i) {
		sub = (evhub__sub_t *)ct_vector_at(&self->sub_list, i);
		if (sub && sub->type == type && (sub->cb == cb || cb == NULL)) {
			ct_vector_erase(&self->sub_list, i--);
			ret = 0;
		}
	}
	pthread_rwlock_unlock(&self->rwlock);
	return ret;
}

int ct_evhub_publish(ct_evhub_t *self, uint32_t type, void *data) {
	assert(self);
	evhub__sub_t *sub;
	pthread_rwlock_rdlock(&self->rwlock);
	for (size_t i = 0; i < ct_vector_size(&self->sub_list); ++i) {
		sub = (evhub__sub_t *)ct_vector_at(&self->sub_list, i);
		if (sub && sub->type == type) {
			sub->cb(type, data, sub->user_data);
		}
	}
	pthread_rwlock_unlock(&self->rwlock);
	return 0;
}
