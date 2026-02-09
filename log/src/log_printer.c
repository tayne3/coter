/**
 * @file log_printer.c
 * @brief 日志打印器
 */
#include "coter/log/log_printer.h"

#include "coter/bytes/pool.h"
#include "coter/log/log_config.h"
#include "coter/strings/strings.h"
#include "coter/sync/atomic.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @struct ct_log_printer
 * @brief 日志打印器
 */
struct ct_log_printer {
	struct ct_bytepool *bytepool; /**< 字节池 */

	ct_bytes_t     *producer_buffer;  /**< 生产者缓存盒 */
	pthread_mutex_t producer_mutex;   /**< 生产者互斥锁 */
	ct_list_t       producer_head[1]; /**< 生产者缓存盒链表头 */

	ct_list_t        consumer_head[1]; /**< 消费者缓存盒链表头 */
	pthread_mutex_t  consumer_mutex;   /**< 消费者互斥锁 */
	ct_atomic_flag_t consumer_flag;    /**< 消费者标志 */
};

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_log_printer_t *ct_log_printer_create(struct ct_bytepool *bytepool) {
	if (!bytepool) {
		return NULL;
	}
	ct_log_printer_t *self = (ct_log_printer_t *)malloc(sizeof(ct_log_printer_t));
	if (!self) {
		return NULL;
	}

	self->bytepool = bytepool;

	ct_list_init(self->producer_head);
	self->producer_buffer = ct_bytepool_get(self->bytepool);
	pthread_mutex_init(&self->producer_mutex, NULL);

	ct_list_init(self->consumer_head);
	pthread_mutex_init(&self->consumer_mutex, NULL);
	self->consumer_flag = (ct_atomic_flag_t)CT_ATOMIC_FLAG_INIT;

	return self;
}

void ct_log_printer_destroy(ct_log_printer_t *self) {
	if (!self || !self->producer_buffer) {
		return;
	}

	pthread_mutex_lock(&self->producer_mutex);
	pthread_mutex_lock(&self->consumer_mutex);
	ct_list_splice_next(self->consumer_head, self->producer_head);
	ct_list_init(self->producer_head);
	ct_atomic_flag_clear(&self->consumer_flag);
	pthread_mutex_unlock(&self->consumer_mutex);
	pthread_mutex_unlock(&self->producer_mutex);

	ct_log_printer_schedule(self);

	fwrite(ct_bytes_buffer(self->producer_buffer), 1, ct_bytes_size(self->producer_buffer), stdout);
	ct_bytepool_put(self->bytepool, self->producer_buffer);
	self->producer_buffer = NULL;

	pthread_mutex_destroy(&self->producer_mutex);
	pthread_mutex_destroy(&self->consumer_mutex);
	free(self);
}

void ct_log_printer_handle(ct_log_printer_t *self, const char *buf, size_t size) {
	if (!self || !self->producer_buffer || !buf || !size) {
		return;
	}

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

void ct_log_printer_flush(ct_log_printer_t *self) {
	if (!self) {
		return;
	}

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

void ct_log_printer_schedule(ct_log_printer_t *self) {
	if (!self) {
		return;
	}
	if (ct_atomic_flag_test_and_set(&self->consumer_flag)) {
		return;
	}

	ct_list_t flush_head[1];
	ct_list_init(flush_head);
	pthread_mutex_lock(&self->consumer_mutex);
	ct_list_splice_next(flush_head, self->consumer_head);
	pthread_mutex_unlock(&self->consumer_mutex);

	ct_list_foreach_entry_safe (consumer, flush_head, ct_bytes_t, list) {
		fwrite(ct_bytes_buffer(consumer), 1, ct_bytes_size(consumer), stdout);  // 写入到 stdout, 耗时操作
		ct_bytepool_put(self->bytepool, consumer);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------
