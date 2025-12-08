/**
 * @file msgqueue.c
 * @brief 消息队列
 */
#include "coter/event/msgqueue.h"

#include "coter/log/log.h"
#include "coter/time/time.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_msgqueue_init(ct_msgqueue_buf_t self, void *buffer, size_t byte, size_t max) {
	ct_queue_init(self->queue, buffer, byte, max);
	pthread_mutex_init(self->mutex, NULL);
	pthread_cond_init(self->not_empty, NULL);
	pthread_cond_init(self->not_full, NULL);
	self->is_shut = false;
}

void ct_msgqueue_close(ct_msgqueue_buf_t self) {
	if (!self || self->is_shut) {
		return;
	}

	pthread_mutex_lock(self->mutex);
	if (self->is_shut) {
		pthread_mutex_unlock(self->mutex);
		return;
	}
	self->is_shut = true;
	pthread_cond_broadcast(self->not_empty);
	pthread_cond_broadcast(self->not_full);
	pthread_mutex_unlock(self->mutex);
}

void ct_msgqueue_destroy(ct_msgqueue_buf_t self) {
	if (!self) {
		return;
	}

	ct_msgqueue_close(self);
	pthread_cond_destroy(self->not_full);
	pthread_cond_destroy(self->not_empty);
	pthread_mutex_destroy(self->mutex);
}

bool ct_msgqueue_isempty(ct_msgqueue_buf_t self) {
	if (!self) {
		return true;
	}
	bool is_empty;
	pthread_mutex_lock(self->mutex);
	is_empty = ct_queue_isempty(self->queue);
	pthread_mutex_unlock(self->mutex);
	return is_empty;
}

bool ct_msgqueue_isfull(ct_msgqueue_buf_t self) {
	if (!self) {
		return true;
	}
	bool is_full;
	pthread_mutex_lock(self->mutex);
	is_full = ct_queue_isfull(self->queue);
	pthread_mutex_unlock(self->mutex);
	return is_full;
}

bool ct_msgqueue_enqueue(ct_msgqueue_buf_t self, const void *item) {
	if (!self || !item) {
		return false;
	}
	if (self->is_shut) {
		return false;
	}

	pthread_mutex_lock(self->mutex);
	if (self->is_shut) {
		pthread_mutex_unlock(self->mutex);
		return false;
	}
	for (; ct_queue_isfull(self->queue);) {
		if (pthread_cond_wait(self->not_full, self->mutex) != 0) {
			pthread_mutex_unlock(self->mutex);
			return false;
		}
		if (self->is_shut) {
			pthread_mutex_unlock(self->mutex);
			return false;
		}
	}

	ct_queue_enqueue(self->queue, item);
	pthread_cond_signal(self->not_empty);
	pthread_mutex_unlock(self->mutex);
	return true;
}

bool ct_msgqueue_dequeue(ct_msgqueue_buf_t self, void *item) {
	if (!self || !item) {
		return false;
	}
	if (self->is_shut) {
		return false;
	}

	pthread_mutex_lock(self->mutex);
	if (self->is_shut) {
		pthread_mutex_unlock(self->mutex);
		return false;
	}
	for (; ct_queue_isempty(self->queue);) {
		if (pthread_cond_wait(self->not_empty, self->mutex) != 0) {
			pthread_mutex_unlock(self->mutex);
			return false;
		}
		if (self->is_shut) {
			pthread_mutex_unlock(self->mutex);
			return false;
		}
	}

	ct_queue_dequeue(self->queue, item);
	pthread_cond_signal(self->not_full);
	pthread_mutex_unlock(self->mutex);
	return true;
}

bool ct_msgqueue_try_enqueue(ct_msgqueue_buf_t self, const void *item) {
	if (!self || !item) {
		return false;
	}
	if (self->is_shut) {
		return false;
	}

	pthread_mutex_lock(self->mutex);
	if (self->is_shut) {
		pthread_mutex_unlock(self->mutex);
		return false;
	}
	if (ct_queue_isfull(self->queue)) {
		pthread_mutex_unlock(self->mutex);
		return false;
	}
	ct_queue_enqueue(self->queue, item);
	pthread_cond_signal(self->not_empty);
	pthread_mutex_unlock(self->mutex);
	return true;
}

bool ct_msgqueue_try_dequeue(ct_msgqueue_buf_t self, void *item) {
	if (!self || !item) {
		return false;
	}
	if (self->is_shut) {
		return false;
	}

	pthread_mutex_lock(self->mutex);
	if (self->is_shut) {
		pthread_mutex_unlock(self->mutex);
		return false;
	}
	if (ct_queue_isempty(self->queue)) {
		pthread_mutex_unlock(self->mutex);
		return false;
	}
	ct_queue_dequeue(self->queue, item);
	pthread_cond_signal(self->not_full);
	pthread_mutex_unlock(self->mutex);
	return true;
}

// -------------------------[STATIC DEFINITION]-------------------------
