/**
 * @file async_bridge.c
 * @brief Internal async log buffer bridge.
 */
#include "async_bridge.h"

#include <stdlib.h>

#include "coter/bytes/bytes.h"
#include "coter/core/strings.h"
#include "coter/sync/atomic.h"
#include "coter/sync/mutex.h"
#include "coter/thread/cache.h"

struct ct_log_async_bridge {
    ct_bytepool_t*          bytepool;           // 字节池
    ct_log_async_policy_t   policy;             // 策略
    size_t                  threshold;          // 阈值
    size_t                  max_pending_bytes;  // 最大等待字节数
    ct_log_async_consume_fn consume;            // 消费函数
    void*                   consume_ctx;        // 消费上下文

    ct_bytes_t*     producer_buffer;   // 生产者缓冲区
    ct_mutex_t      producer_mutex;    // 生产者互斥锁
    ct_list_t       producer_head[1];  // 生产者队列
    size_t          producer_size;     // 生产者大小
    ct_atomic_int_t pending_bytes;     // 等待字节数

    ct_list_t        consumer_head[1];  // 消费者队列
    ct_mutex_t       consumer_mutex;    // 消费者互斥锁
    ct_atomic_flag_t consumer_flag;     // 消费者标志
};

struct ct_log_tls_batch {
    ct_log_async_bridge_t* bridge;
    ct_bytes_t*            buffer;
};

// 生产者入队
static void async_bridge_enqueue_locked(ct_log_async_bridge_t* self);
// 生产者追加
static void async_bridge_append_locked(ct_log_async_bridge_t* self, const char* buf, size_t size);
// 生产者发布
static void async_bridge_publish_locked(ct_log_async_bridge_t* self);

static void tc__cleanup_batch(void* ptr);

static void tc__pull_from_cache(ct_threadcache_t* tc, void* arg, bool force);

static void tc__strip_bridge(ct_threadcache_t* tc, void* arg, bool force);

ct_log_async_bridge_t* ct_log_async_bridge_create(const ct_log_async_config_t* config) {
    if (!config || !config->bytepool || !config->consume) { return NULL; }
    if (config->policy == CT_LOG_ASYNC_POLICY_THRESHOLD && config->threshold == 0) { return NULL; }

    ct_log_async_bridge_t* self = (ct_log_async_bridge_t*)malloc(sizeof(ct_log_async_bridge_t));
    if (!self) { return NULL; }

    self->bytepool          = config->bytepool;
    self->policy            = config->policy;
    self->threshold         = config->threshold;
    self->max_pending_bytes = config->max_pending_bytes;
    self->consume           = config->consume;
    self->consume_ctx       = config->consume_ctx;

    self->producer_buffer = ct_bytepool_get(self->bytepool);
    if (!self->producer_buffer) {
        free(self);
        return NULL;
    }

    ct_mutex_init(&self->producer_mutex);
    ct_list_init(self->producer_head);
    self->producer_size = 0;
    self->pending_bytes = CT_ATOMIC_VAR_INIT(0);

    ct_mutex_init(&self->consumer_mutex);
    ct_list_init(self->consumer_head);
    self->consumer_flag = (ct_atomic_flag_t)CT_ATOMIC_FLAG_INIT;
    ct_atomic_flag_test_and_set(&self->consumer_flag);

    return self;
}

void ct_log_async_bridge_destroy(ct_log_async_bridge_t* self) {
    if (!self) { return; }

    ct_threadcache_foreach(tc__strip_bridge, self, true);

    ct_log_async_bridge_flush(self);
    ct_log_async_bridge_schedule(self);

    if (self->producer_buffer) {
        ct_bytepool_put(self->bytepool, self->producer_buffer);
        self->producer_buffer = NULL;
    }

    ct_mutex_destroy(&self->producer_mutex);
    ct_mutex_destroy(&self->consumer_mutex);
    free(self);
}

void ct_log_async_bridge_push(ct_log_async_bridge_t* self, const char* buf, size_t size) {
    if (!self || !buf || !size) { return; }

    ct_threadcache_t*        tc    = ct_threadcache_get();
    struct ct_log_tls_batch* batch = (struct ct_log_tls_batch*)ct_threadcache_get_async_data(tc);
    if (!batch) {
        batch = (struct ct_log_tls_batch*)calloc(1, sizeof(struct ct_log_tls_batch));
        if (batch) { ct_threadcache_set_async_data(tc, batch, tc__cleanup_batch); }
    }

    if (batch) {
        ct_threadcache_lock(tc);
        if (batch->bridge != self) {
            if (batch->bridge && batch->buffer) {
                ct_bytes_t* next = ct_log_async_bridge_submit(batch->bridge, batch->buffer);
                if (next) { ct_bytepool_put(batch->bridge->bytepool, next); }
                batch->buffer = NULL;
            }
            batch->bridge = self;
        }

        if (!batch->buffer) { batch->buffer = ct_log_async_bridge_acquire(self); }

        if (batch->buffer) {
            ct_list_t submit_list;
            ct_list_init(&submit_list);

            bool         crossed_boundary = false;
            const char*  original_buf     = buf;
            const size_t original_size    = size;

            while (size > 0) {
                size_t written = ct_bytes_write(batch->buffer, buf, size);
                buf += written;
                size -= written;

                if (size > 0) {
                    crossed_boundary = true;
                    ct_atomic_int_add(&self->pending_bytes, (int)ct_bytes_size(batch->buffer));
                    ct_list_append(&submit_list, batch->buffer->list);
                    batch->buffer = ct_log_async_bridge_acquire(self);
                    if (!batch->buffer) break;
                }
            }

            if (batch->buffer) {
                bool trigger = crossed_boundary;
                if (self->policy == CT_LOG_ASYNC_POLICY_NEWLINE) {
                    if (ct_memrchr(original_buf, '\n', original_size)) { trigger = true; }
                } else if (self->policy == CT_LOG_ASYNC_POLICY_THRESHOLD) {
                    if (ct_bytes_size(batch->buffer) >= self->threshold) { trigger = true; }
                }

                if (trigger || ct_bytes_available(batch->buffer) == 0) {
                    ct_atomic_int_add(&self->pending_bytes, (int)ct_bytes_size(batch->buffer));
                    ct_list_append(&submit_list, batch->buffer->list);
                    batch->buffer = NULL;
                }
            }

            if (!ct_list_isempty(&submit_list)) {
                ct_mutex_lock(&self->consumer_mutex);
                ct_list_splice_next(self->consumer_head, &submit_list);
                ct_atomic_flag_clear(&self->consumer_flag);
                ct_mutex_unlock(&self->consumer_mutex);
            }

            ct_threadcache_unlock(tc);
            return;
        }
        ct_threadcache_unlock(tc);
    }

    ct_mutex_lock(&self->producer_mutex);
    async_bridge_append_locked(self, buf, size);
    if (self->policy == CT_LOG_ASYNC_POLICY_NEWLINE) {
        if (ct_memrchr(buf, '\n', size)) {
            async_bridge_enqueue_locked(self);
            async_bridge_publish_locked(self);
        }
    } else if (self->policy == CT_LOG_ASYNC_POLICY_THRESHOLD && self->producer_size >= self->threshold) {
        async_bridge_publish_locked(self);
    }
    ct_mutex_unlock(&self->producer_mutex);
}

void ct_log_async_bridge_flush(ct_log_async_bridge_t* self) {
    if (!self) { return; }
    ct_threadcache_foreach(tc__pull_from_cache, self, true);

    ct_mutex_lock(&self->producer_mutex);
    async_bridge_enqueue_locked(self);
    async_bridge_publish_locked(self);
    ct_mutex_unlock(&self->producer_mutex);
}

void ct_log_async_bridge_schedule(ct_log_async_bridge_t* self) {
    if (!self) { return; }
    ct_threadcache_foreach(tc__pull_from_cache, self, false);

    if (ct_atomic_flag_test_and_set(&self->consumer_flag)) { return; }

    ct_list_t flush_head[1];
    ct_list_init(flush_head);

    ct_mutex_lock(&self->consumer_mutex);
    ct_list_splice_next(flush_head, self->consumer_head);
    ct_mutex_unlock(&self->consumer_mutex);

    ct_list_foreach_entry_safe(bytes, flush_head, ct_bytes_t, list) {
        const size_t size = ct_bytes_size(bytes);
        self->consume(ct_bytes_buffer(bytes), size, self->consume_ctx);
        ct_atomic_int_sub(&self->pending_bytes, (int)size);
        ct_bytepool_put(self->bytepool, bytes);
    }
}

static void async_bridge_enqueue_locked(ct_log_async_bridge_t* self) {
    if (!self->producer_buffer || ct_bytes_isempty(self->producer_buffer)) { return; }

    const size_t size = ct_bytes_size(self->producer_buffer);
    ct_list_append(self->producer_head, self->producer_buffer->list);
    self->producer_size += size;
    ct_atomic_int_add(&self->pending_bytes, (int)size);
    self->producer_buffer = ct_bytepool_get(self->bytepool);
}

static void async_bridge_append_locked(ct_log_async_bridge_t* self, const char* buf, size_t size) {
    if (size == 0) { return; }
    while (1) {
        const size_t written = ct_bytes_write(self->producer_buffer, buf, size);
        buf += written;
        size -= written;
        if (size == 0) { break; }
        async_bridge_enqueue_locked(self);
    }
}

static void async_bridge_publish_locked(ct_log_async_bridge_t* self) {
    if (ct_list_isempty(self->producer_head)) { return; }

    ct_mutex_lock(&self->consumer_mutex);
    ct_list_splice_next(self->consumer_head, self->producer_head);
    ct_list_init(self->producer_head);
    self->producer_size = 0;
    ct_atomic_flag_clear(&self->consumer_flag);
    ct_mutex_unlock(&self->consumer_mutex);
}

ct_bytes_t* ct_log_async_bridge_acquire(ct_log_async_bridge_t* self) {
    if (!self) { return NULL; }
    return ct_bytepool_get(self->bytepool);
}

ct_bytes_t* ct_log_async_bridge_submit(ct_log_async_bridge_t* self, ct_bytes_t* bytes) {
    if (!self || !bytes) { return NULL; }
    if (ct_bytes_isempty(bytes)) { return bytes; }

    const size_t size = ct_bytes_size(bytes);
    ct_atomic_int_add(&self->pending_bytes, (int)size);

    ct_mutex_lock(&self->consumer_mutex);
    ct_list_append(self->consumer_head, bytes->list);
    ct_atomic_flag_clear(&self->consumer_flag);
    ct_mutex_unlock(&self->consumer_mutex);

    return ct_bytepool_get(self->bytepool);
}

static void tc__cleanup_batch(void* ptr) {
    struct ct_log_tls_batch* batch = (struct ct_log_tls_batch*)ptr;
    if (batch) {
        if (batch->bridge && batch->buffer) {
            ct_bytes_t* next = ct_log_async_bridge_submit(batch->bridge, batch->buffer);
            if (next) { ct_bytepool_put(batch->bridge->bytepool, next); }
        }
        free(batch);
    }
}

static void tc__pull_from_cache(ct_threadcache_t* tc, void* arg, bool force) {
    ct_log_async_bridge_t*   self  = (ct_log_async_bridge_t*)arg;
    struct ct_log_tls_batch* batch = (struct ct_log_tls_batch*)ct_threadcache_get_async_data(tc);
    if (batch && batch->bridge == self) {
        if (force) {
            if (ct_threadcache_trylock(tc) == 0) {
                if (batch->buffer && !ct_bytes_isempty(batch->buffer)) {
                    batch->buffer = ct_log_async_bridge_submit(self, batch->buffer);
                }
                ct_threadcache_unlock(tc);
            }
        }
    }
}

static void tc__strip_bridge(ct_threadcache_t* tc, void* arg, bool force) {
    ct_log_async_bridge_t*   self  = (ct_log_async_bridge_t*)arg;
    struct ct_log_tls_batch* batch = (struct ct_log_tls_batch*)ct_threadcache_get_async_data(tc);
    (void)force;
    if (batch && batch->bridge == self) {
        ct_threadcache_lock(tc);
        if (batch->bridge == self) {
            if (batch->buffer && !ct_bytes_isempty(batch->buffer)) {
                ct_bytes_t* next = ct_log_async_bridge_submit(self, batch->buffer);
                if (next) { ct_bytepool_put(self->bytepool, next); }
            } else if (batch->buffer) {
                ct_bytepool_put(self->bytepool, batch->buffer);
            }
            batch->bridge = NULL;
            batch->buffer = NULL;
        }
        ct_threadcache_unlock(tc);
    }
}
