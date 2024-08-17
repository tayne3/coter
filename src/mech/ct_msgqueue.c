/**
 * @file ct_msgqueue.c
 * @brief 消息队列
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "ct_msgqueue.h"
#include "base/ct_time.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_msgqueue]"

#define ct_msgqueue_lock(self)   pthread_mutex_lock(self->mutex)
#define ct_msgqueue_unlock(self) pthread_mutex_unlock(self->mutex)

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_msgqueue_init(ct_msgqueue_buf_t self, void *buffer, size_t byte, size_t max) {
	ct_queue_init(self->queue, buffer, byte, max);
	pthread_mutex_init(self->mutex, NULL);
	pthread_cond_init(self->not_empty, NULL);
	pthread_cond_init(self->not_full, NULL);
	self->is_shut = false;
}

void ct_msgqueue_close(ct_msgqueue_buf_t self) {
	if (self->is_shut) {
		return;
	}

	ct_msgqueue_lock(self);
	self->is_shut = true;
	pthread_cond_broadcast(self->not_empty);
	pthread_cond_broadcast(self->not_full);
	ct_msgqueue_unlock(self);
}

void ct_msgqueue_destroy(ct_msgqueue_buf_t self) {
	if (!self->is_shut) {
		ct_msgqueue_close(self);
	}

	pthread_cond_destroy(self->not_full);
	pthread_cond_destroy(self->not_empty);
	pthread_mutex_destroy(self->mutex);
}

bool ct_msgqueue_isempty(ct_msgqueue_buf_t self) {
	bool is_empty;
	ct_msgqueue_lock(self);
	is_empty = ct_queue_isempty(self->queue);
	ct_msgqueue_unlock(self);
	return is_empty;
}

bool ct_msgqueue_isfull(ct_msgqueue_buf_t self) {
	bool is_full;
	ct_msgqueue_lock(self);
	is_full = ct_queue_isfull(self->queue);
	ct_msgqueue_unlock(self);
	return is_full;
}

bool ct_msgqueue_enqueue(ct_msgqueue_buf_t self, const void *item) {
	if (self->is_shut) {
		return false;
	}

	ct_msgqueue_lock(self);
	for (; ct_queue_isfull(self->queue);) {
		if (pthread_cond_wait(self->not_full, self->mutex) != 0) {
			ct_msgqueue_unlock(self);
			return false;
		}
		if (self->is_shut) {
			ct_msgqueue_unlock(self);
			return false;
		}
	}

	ct_queue_enqueue(self->queue, item);
	pthread_cond_signal(self->not_empty);
	ct_msgqueue_unlock(self);
	return true;
}

bool ct_msgqueue_dequeue(ct_msgqueue_buf_t self, void *item) {
	if (self->is_shut) {
		return false;
	}

	ct_msgqueue_lock(self);
	for (; ct_queue_isempty(self->queue);) {
		if (pthread_cond_wait(self->not_empty, self->mutex) != 0) {
			ct_msgqueue_unlock(self);
			return false;
		}
		if (self->is_shut) {
			ct_msgqueue_unlock(self);
			return false;
		}
	}

	ct_queue_dequeue(self->queue, item);
	pthread_cond_signal(self->not_full);
	ct_msgqueue_unlock(self);
	return true;
}

bool ct_msgqueue_try_enqueue(ct_msgqueue_buf_t self, const void *item) {
	if (self->is_shut) {
		return false;
	}

	ct_msgqueue_lock(self);
	if (self->is_shut) {
		ct_msgqueue_unlock(self);
		return false;
	}
	if (ct_queue_isfull(self->queue)) {
		ct_msgqueue_unlock(self);
		return false;
	}
	ct_queue_enqueue(self->queue, item);
	pthread_cond_signal(self->not_empty);
	ct_msgqueue_unlock(self);
	return true;
}

bool ct_msgqueue_try_dequeue(ct_msgqueue_buf_t self, void *item) {
	if (self->is_shut) {
		return false;
	}

	ct_msgqueue_lock(self);
	if (self->is_shut) {
		ct_msgqueue_unlock(self);
		return false;
	}
	if (ct_queue_isempty(self->queue)) {
		ct_msgqueue_unlock(self);
		return false;
	}
	ct_queue_dequeue(self->queue, item);
	pthread_cond_signal(self->not_full);
	ct_msgqueue_unlock(self);
	return true;
}

// -------------------------[STATIC DEFINITION]-------------------------
