#include "ct_bytepool.h"

#include "base/ct_atomic.h"
#include "ct_spinlock.h"

struct ct_bytepool {
	ct_list_t     box_list[1];   // 字节数组链表
	size_t        box_capacity;  // 字节数组容量
	size_t        max;           // 池容量
	ct_atomic_t   size;          // 池大小
	ct_spinlock_t lock;          // 自旋锁
};

ct_bytepool_t *ct_bytepool_create(size_t max_size, size_t bytes_capacity) {
	struct ct_bytepool *self = (struct ct_bytepool *)malloc(sizeof(struct ct_bytepool));
	if (!self) {
		return NULL;
	}
	self->box_capacity = bytes_capacity;
	self->size         = CT_ATOMIC_VAR_INIT(0);
	self->max          = max_size;
	ct_list_init(self->box_list);
	ct_spinlock_init(&self->lock);
	return self;
}

void ct_bytepool_destroy(ct_bytepool_t *self) {
	if (self) {
		ct_spinlock_lock(&self->lock);
		ct_list_foreach_entry_safe (bytes, self->box_list, ct_bytes_t, list) {
			ct_list_remove(bytes->list);
			ct_bytes_destroy(bytes);
		}
		ct_spinlock_unlock(&self->lock);
		ct_spinlock_destroy(&self->lock);
		free(self);
	}
}

ct_bytes_t *ct_bytepool_get(ct_bytepool_t *self) {
	assert(self);
	ct_bytes_t *bytes = NULL;

	if ((size_t)ct_atomic_load(&self->size) > 0) {
		ct_spinlock_lock(&self->lock);
		if ((size_t)ct_atomic_load(&self->size) > 0) {
			bytes = ct_list_last_entry(self->box_list, ct_bytes_t, list);
			ct_list_remove(bytes->list);
			ct_atomic_sub(&self->size, 1);
			ct_spinlock_unlock(&self->lock);
			assert(bytes);
			return bytes;
		}
		ct_spinlock_unlock(&self->lock);
	}

	return ct_bytes_create(self->box_capacity);
}

void ct_bytepool_put(ct_bytepool_t *self, ct_bytes_t *bytes) {
	assert(self);
	assert(bytes);

	ct_bytes_clear(bytes);

	bool added = false;
	if ((size_t)ct_atomic_load(&self->size) < self->max) {
		ct_spinlock_lock(&self->lock);
		if ((size_t)ct_atomic_load(&self->size) < self->max) {
			ct_list_append(self->box_list, bytes->list);
			ct_atomic_add(&self->size, 1);
			added = true;
		}
		ct_spinlock_unlock(&self->lock);
	}

	if (!added) {
		ct_bytes_destroy(bytes);
	}
}
