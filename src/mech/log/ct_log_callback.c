/**
 * @file ct_log_callback.c
 * @brief 日志回调器
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_log_callback.h"

#include "base/ct_atomic.h"
#include "ct_log_config.h"
#include "mech/ct_bytepool.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @struct ct_log_callback
 * @brief 日志回调器
 */
struct ct_log_callback {
	struct ct_bytepool *bytepool;                               /**< 字节池 */
	void (*routine)(const char *, size_t size, void *userdata); /**< 回调函数指针 */
	void *userdata;                                             /**< 用户自定义数据 */

	ct_bytes_t     *producer_buffer;  /**< 生产者缓存盒 */
	pthread_mutex_t producer_mutex;   /**< 生产者互斥锁 */
	ct_list_t       producer_head[1]; /**< 生产者缓存盒链表头 */

	ct_list_t        consumer_head[1]; /**< 消费者缓存盒链表头 */
	pthread_mutex_t  consumer_mutex;   /**< 消费者互斥锁 */
	ct_atomic_flag_t consumer_flag;    /**< 消费者标志 */
};

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_log_callback_t *ct_log_callback_create(struct ct_bytepool *bytepool, const struct ct_log_config *config) {
	assert(config);
	assert(bytepool);
	assert(config->callback_routine);
	ct_log_callback_t *self = (ct_log_callback_t *)malloc(sizeof(ct_log_callback_t));
	if (!self) {
		return NULL;
	}

	self->bytepool = bytepool;
	self->routine  = config->callback_routine;
	self->userdata = config->callback_userdata;

	ct_list_init(self->producer_head);
	self->producer_buffer = ct_bytepool_get(self->bytepool);
	pthread_mutex_init(&self->producer_mutex, NULL);

	ct_list_init(self->consumer_head);
	pthread_mutex_init(&self->consumer_mutex, NULL);
	self->consumer_flag = (ct_atomic_flag_t)CT_ATOMIC_FLAG_INIT;

	return self;
}

void ct_log_callback_destroy(ct_log_callback_t *self) {
	assert(self);
	assert(self->producer_buffer);

	pthread_mutex_lock(&self->producer_mutex);
	pthread_mutex_lock(&self->consumer_mutex);
	ct_list_splice_next(self->consumer_head, self->producer_head);
	ct_list_init(self->producer_head);
	ct_atomic_flag_clear(&self->consumer_flag);
	pthread_mutex_unlock(&self->consumer_mutex);
	pthread_mutex_unlock(&self->producer_mutex);

	ct_log_callback_schedule(self);

	self->routine(ct_bytes_buffer(self->producer_buffer), ct_bytes_size(self->producer_buffer), self->userdata);
	ct_bytepool_put(self->bytepool, self->producer_buffer);
	self->producer_buffer = NULL;

	pthread_mutex_destroy(&self->producer_mutex);
	pthread_mutex_destroy(&self->consumer_mutex);
	free(self);
}

void ct_log_callback_handle(ct_log_callback_t *self, char *buf, size_t size) {
	assert(self);
	assert(self->producer_buffer);
	assert(buf);
	assert(size > 0);

	char        *last_newline = ct_memrchr(buf, '\n', size);
	const size_t flush_size   = last_newline ? (last_newline - buf + 1) : 0;
	size_t       processed    = 0;

	pthread_mutex_lock(&self->producer_mutex);

	// 处理需要刷新的数据
	if (flush_size > 0) {
		while (processed < flush_size) {
			const size_t to_write = flush_size - processed;
			const size_t written  = ct_bytes_write(self->producer_buffer, buf, to_write);
			buf += written;
			processed += written;

			if (written < to_write) {
				ct_list_append(self->producer_head, self->producer_buffer->list);
				self->producer_buffer = ct_bytepool_get(self->bytepool);
			}
		}

		if (!ct_bytes_isempty(self->producer_buffer)) {
			ct_list_append(self->producer_head, self->producer_buffer->list);
			self->producer_buffer = ct_bytepool_get(self->bytepool);
		}

		pthread_mutex_lock(&self->consumer_mutex);
		ct_list_splice_next(self->consumer_head, self->producer_head);
		ct_atomic_flag_clear(&self->consumer_flag);
		pthread_mutex_unlock(&self->consumer_mutex);

		ct_list_init(self->producer_head);
	}

	// 处理剩余的数据
	while (processed < size) {
		const size_t to_write = size - processed;
		const size_t written  = ct_bytes_write(self->producer_buffer, buf, to_write);
		buf += written;
		processed += written;

		if (written < to_write) {
			ct_list_append(self->producer_head, self->producer_buffer->list);
			self->producer_buffer = ct_bytepool_get(self->bytepool);
		}
	}

	pthread_mutex_unlock(&self->producer_mutex);
	return;
}

void ct_log_callback_flush(ct_log_callback_t *self) {
	assert(self);

	pthread_mutex_lock(&self->producer_mutex);
	if (!ct_bytes_isempty(self->producer_buffer)) {
		ct_list_append(self->producer_head, self->producer_buffer->list);
		self->producer_buffer = ct_bytepool_get(self->bytepool);
	}
	if (!ct_list_isempty(self->producer_head)) {
		pthread_mutex_lock(&self->consumer_mutex);
		ct_list_splice_next(self->consumer_head, self->producer_head);
		ct_list_init(self->producer_head);
		ct_atomic_flag_clear(&self->consumer_flag);
		pthread_mutex_unlock(&self->consumer_mutex);
	}
	pthread_mutex_unlock(&self->producer_mutex);
}

void ct_log_callback_schedule(ct_log_callback_t *self) {
	assert(self);
	if (ct_atomic_flag_test_and_set(&self->consumer_flag)) {
		return;
	}

	ct_list_t flush_head[1];
	ct_list_init(flush_head);
	pthread_mutex_lock(&self->consumer_mutex);
	ct_list_splice_next(flush_head, self->consumer_head);
	pthread_mutex_unlock(&self->consumer_mutex);

	ct_list_foreach_entry_safe (bytes, flush_head, ct_bytes_t, list) {
		self->routine(ct_bytes_buffer(bytes), ct_bytes_size(bytes), self->userdata);
		ct_bytepool_put(self->bytepool, bytes);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------
