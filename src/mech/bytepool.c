/**
 * @file ct_bytepool.c
 * @brief 字节池
 * @author tayne3@dingtalk.com
 */
#include "coter/mech/bytepool.h"

#include "coter/base/atomic.h"

// -------------------------[STATIC DECLARATION]-------------------------

struct ct_bytepool {
	ct_list_t       bytes_list[1];   // 字节数组链表
	size_t          bytes_capacity;  // 字节数组容量
	size_t          max;             // 池容量
	ct_atomic_t     size;            // 池大小
	pthread_mutex_t lock;            // 自旋锁
};

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_bytepool_t *ct_bytepool_create(size_t max_size, size_t bytes_capacity) {
	struct ct_bytepool *self = (struct ct_bytepool *)malloc(sizeof(struct ct_bytepool));
	if (!self) {
		return NULL;
	}
	self->bytes_capacity = bytes_capacity;
	self->size           = CT_ATOMIC_VAR_INIT(0);
	self->max            = max_size;
	ct_list_init(self->bytes_list);
	pthread_mutex_init(&self->lock, NULL);
	return self;
}

void ct_bytepool_destroy(ct_bytepool_t *self) {
	assert(self);
	pthread_mutex_lock(&self->lock);
	ct_list_foreach_entry_safe (bytes, self->bytes_list, ct_bytes_t, list) {
		ct_list_remove(bytes->list);
		ct_bytes_destroy(bytes);
	}
	pthread_mutex_unlock(&self->lock);
	pthread_mutex_destroy(&self->lock);
	free(self);
}

ct_bytes_t *ct_bytepool_get(ct_bytepool_t *self) {
	assert(self);
	ct_bytes_t *bytes = NULL;

	if ((size_t)ct_atomic_load(&self->size) > 0) {
		pthread_mutex_lock(&self->lock);
		if ((size_t)ct_atomic_load(&self->size) > 0) {
			bytes = ct_list_last_entry(self->bytes_list, ct_bytes_t, list);
			ct_list_remove(bytes->list);
			ct_atomic_sub(&self->size, 1);
			pthread_mutex_unlock(&self->lock);
			assert(bytes);
			return bytes;
		}
		pthread_mutex_unlock(&self->lock);
	}

	return ct_bytes_create(self->bytes_capacity);
}

void ct_bytepool_put(ct_bytepool_t *self, ct_bytes_t *bytes) {
	assert(self);
	assert(bytes);

	ct_bytes_clear(bytes);

	bool added = false;
	if ((size_t)ct_atomic_load(&self->size) < self->max) {
		pthread_mutex_lock(&self->lock);
		if ((size_t)ct_atomic_load(&self->size) < self->max) {
			ct_list_append(self->bytes_list, bytes->list);
			ct_atomic_add(&self->size, 1);
			added = true;
		}
		pthread_mutex_unlock(&self->lock);
	}

	if (!added) {
		ct_bytes_destroy(bytes);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------
