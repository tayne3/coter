/**
 * @file msgqueue.c
 * @brief 消息队列
 */
#include "coter/event/msgqueue.h"

void ct_msgqueue_init(ct_msgqueue_t* self, void* buffer, size_t byte, size_t max) {
	ct_queue_init(self->queue, buffer, byte, max);
	ct_mutex_init(&self->mutex);
	ct_cond_init(&self->not_empty);
	ct_cond_init(&self->not_full);
	self->is_shut = false;
}

void ct_msgqueue_close(ct_msgqueue_t* self) {
	if (!self || self->is_shut) { return; }

	ct_mutex_lock(&self->mutex);
	if (self->is_shut) {
		ct_mutex_unlock(&self->mutex);
		return;
	}
	self->is_shut = true;
	ct_cond_broadcast(&self->not_empty);
	ct_cond_broadcast(&self->not_full);
	ct_mutex_unlock(&self->mutex);
}

void ct_msgqueue_destroy(ct_msgqueue_t* self) {
	if (!self) { return; }

	ct_msgqueue_close(self);
	ct_cond_destroy(&self->not_full);
	ct_cond_destroy(&self->not_empty);
	ct_mutex_destroy(&self->mutex);
}

bool ct_msgqueue_isempty(ct_msgqueue_t* self) {
	if (!self) { return true; }
	bool is_empty;
	ct_mutex_lock(&self->mutex);
	is_empty = ct_queue_isempty(self->queue);
	ct_mutex_unlock(&self->mutex);
	return is_empty;
}

bool ct_msgqueue_isfull(ct_msgqueue_t* self) {
	if (!self) { return true; }
	bool is_full;
	ct_mutex_lock(&self->mutex);
	is_full = ct_queue_isfull(self->queue);
	ct_mutex_unlock(&self->mutex);
	return is_full;
}

bool ct_msgqueue_enqueue(ct_msgqueue_t* self, const void* item) {
	if (!self || !item) { return false; }
	if (self->is_shut) { return false; }

	ct_mutex_lock(&self->mutex);
	if (self->is_shut) {
		ct_mutex_unlock(&self->mutex);
		return false;
	}
	for (; ct_queue_isfull(self->queue);) {
		if (ct_cond_wait(&self->not_full, &self->mutex) != 0) {
			ct_mutex_unlock(&self->mutex);
			return false;
		}
		if (self->is_shut) {
			ct_mutex_unlock(&self->mutex);
			return false;
		}
	}

	ct_queue_enqueue(self->queue, item);
	ct_cond_signal(&self->not_empty);
	ct_mutex_unlock(&self->mutex);
	return true;
}

bool ct_msgqueue_dequeue(ct_msgqueue_t* self, void* item) {
	if (!self || !item) { return false; }
	if (self->is_shut) { return false; }

	ct_mutex_lock(&self->mutex);
	if (self->is_shut) {
		ct_mutex_unlock(&self->mutex);
		return false;
	}
	for (; ct_queue_isempty(self->queue);) {
		if (ct_cond_wait(&self->not_empty, &self->mutex) != 0) {
			ct_mutex_unlock(&self->mutex);
			return false;
		}
		if (self->is_shut) {
			ct_mutex_unlock(&self->mutex);
			return false;
		}
	}

	ct_queue_dequeue(self->queue, item);
	ct_cond_signal(&self->not_full);
	ct_mutex_unlock(&self->mutex);
	return true;
}

bool ct_msgqueue_try_enqueue(ct_msgqueue_t* self, const void* item) {
	if (!self || !item) { return false; }
	if (self->is_shut) { return false; }

	ct_mutex_lock(&self->mutex);
	if (self->is_shut) {
		ct_mutex_unlock(&self->mutex);
		return false;
	}
	if (ct_queue_isfull(self->queue)) {
		ct_mutex_unlock(&self->mutex);
		return false;
	}
	ct_queue_enqueue(self->queue, item);
	ct_cond_signal(&self->not_empty);
	ct_mutex_unlock(&self->mutex);
	return true;
}

bool ct_msgqueue_try_dequeue(ct_msgqueue_t* self, void* item) {
	if (!self || !item) { return false; }
	if (self->is_shut) { return false; }

	ct_mutex_lock(&self->mutex);
	if (self->is_shut) {
		ct_mutex_unlock(&self->mutex);
		return false;
	}
	if (ct_queue_isempty(self->queue)) {
		ct_mutex_unlock(&self->mutex);
		return false;
	}
	ct_queue_dequeue(self->queue, item);
	ct_cond_signal(&self->not_full);
	ct_mutex_unlock(&self->mutex);
	return true;
}
