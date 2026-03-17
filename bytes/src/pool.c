/**
 * @file bytepool.c
 * @brief 字节池
 */
#include "coter/bytes/pool.h"

#include "coter/sync/atomic.h"
#include "coter/sync/mutex.h"

// -------------------------[STATIC DECLARATION]-------------------------

struct ct_bytepool {
	ct_list_t        bytes_list[1];   // 字节数组链表
	size_t           bytes_capacity;  // 字节数组容量
	size_t           max;             // 池容量
	ct_atomic_long_t size;            // 池大小
	ct_mutex_t       lock;            // 保护锁
};

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_bytepool_t *ct_bytepool_create(size_t max_size, size_t bytes_capacity) {
	struct ct_bytepool *self = (struct ct_bytepool *)malloc(sizeof(struct ct_bytepool));
	if (!self) { return NULL; }
	self->bytes_capacity = bytes_capacity;
	self->size           = CT_ATOMIC_VAR_INIT(0);
	self->max            = max_size;
	ct_list_init(self->bytes_list);
	ct_mutex_init(&self->lock);
	return self;
}

void ct_bytepool_destroy(ct_bytepool_t *self) {
	if (!self) { return; }
	ct_mutex_lock(&self->lock);
	ct_list_foreach_entry_safe (bytes, self->bytes_list, ct_bytes_t, list) {
		ct_list_remove(bytes->list);
		ct_bytes_destroy(bytes);
	}
	ct_mutex_unlock(&self->lock);
	ct_mutex_destroy(&self->lock);
	free(self);
}

ct_bytes_t *ct_bytepool_get(ct_bytepool_t *self) {
	if (!self) { return NULL; }
	ct_bytes_t *bytes = NULL;

	if ((size_t)ct_atomic_long_load(&self->size) > 0) {
		ct_mutex_lock(&self->lock);
		if ((size_t)ct_atomic_long_load(&self->size) > 0) {
			bytes = ct_list_last_entry(self->bytes_list, ct_bytes_t, list);
			if (bytes) {
				ct_list_remove(bytes->list);
				ct_atomic_long_sub(&self->size, 1);
				ct_mutex_unlock(&self->lock);
				return bytes;
			}
		}
		ct_mutex_unlock(&self->lock);
	}

	return ct_bytes_create(self->bytes_capacity);
}

void ct_bytepool_put(ct_bytepool_t *self, ct_bytes_t *bytes) {
	if (!self || !bytes) { return; }

	ct_bytes_clear(bytes);

	bool added = false;
	if ((size_t)ct_atomic_long_load(&self->size) < self->max) {
		ct_mutex_lock(&self->lock);
		if ((size_t)ct_atomic_long_load(&self->size) < self->max) {
			ct_list_append(self->bytes_list, bytes->list);
			ct_atomic_long_add(&self->size, 1);
			added = true;
		}
		ct_mutex_unlock(&self->lock);
	}

	if (!added) { ct_bytes_destroy(bytes); }
}

// -------------------------[STATIC DEFINITION]-------------------------
