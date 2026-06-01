/**
 * @file dispatcher.c
 * @brief Internal log dispatcher.
 */
#include <stdlib.h>
#include <string.h>

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/msgqueue.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"
#include "log_internal.h"

#define CT_LOG_BATCH_MAX 1024

// -------------------------[BLOCK & POOL INTERNAL]-------------------------

struct ct_log_block_pool {
    ct_msgqueue_t queue;
    void*         q_buf;
    uint32_t      block_capacity;
};

static ct_log_block_t* block__create(uint32_t capacity) {
    ct_log_block_t* self = (ct_log_block_t*)malloc(sizeof(ct_log_block_t) + capacity);
    if (!self) { return NULL; }
    ct_list_init(&self->node);
    self->capacity  = capacity;
    self->used      = 0;
    self->rec_count = 0;
    return self;
}

static void block__destroy(ct_log_block_t* block) {
    if (block) { free(block); }
}

static void block__clear(ct_log_block_t* block) {
    if (block) {
        block->used      = 0;
        block->rec_count = 0;
    }
}

static ct_log_block_pool_t* block__pool_create(uint32_t max_blocks, uint32_t block_capacity) {
    ct_log_block_pool_t* self = (ct_log_block_pool_t*)calloc(1, sizeof(ct_log_block_pool_t));
    if (!self) { return NULL; }
    self->block_capacity = block_capacity;

    self->q_buf = malloc(sizeof(ct_log_block_t*) * max_blocks);
    if (!self->q_buf) {
        free(self);
        return NULL;
    }

    ct_msgqueue_init(&self->queue, self->q_buf, sizeof(ct_log_block_t*), max_blocks);
    return self;
}

static void block__pool_destroy(ct_log_block_pool_t* pool) {
    if (!pool) { return; }
    ct_log_block_t* block;
    while (ct_msgqueue_try_pop(&pool->queue, &block) == 0) { block__destroy(block); }
    ct_msgqueue_destroy(&pool->queue);
    free(pool->q_buf);
    free(pool);
}

ct_log_block_t* ct_log_block_pool_acquire(ct_log_block_pool_t* pool) {
    if (!pool) { return NULL; }
    ct_log_block_t* block;
    if (ct_msgqueue_try_pop(&pool->queue, &block) == 0) { return block; }
    return block__create(pool->block_capacity);
}

static void block__pool_release(ct_log_block_pool_t* pool, ct_log_block_t* block) {
    if (!pool || !block) { return; }
    block__clear(block);
    if (ct_msgqueue_try_push(&pool->queue, &block) != 0) { block__destroy(block); }
}

// -------------------------[DISPATCHER INTERNAL]-------------------------

typedef struct {
    ct_logger_t*    logger;
    ct_log_block_t* block;
} ct_log_job_t;

struct ct_log_dispatcher {
    ct_thread_t          thread;
    ct_msgqueue_t        queue;
    void*                q_buf;
    ct_log_block_pool_t* pool;
    ct_atomic_int_t      pending_jobs;
    ct_mutex_t           flush_mutex;
    ct_cond_t            flush_cond;
    ct_log_record_t      batch_records[CT_LOG_BATCH_MAX];  // Fixed pre-allocated buffer
    bool                 running;
};

static inline void dispatcher__do_heartbeat(ct_time64_t* last_heartbeat) {
    ct_time64_t now = ct_getuptime_ms();
    if (now - *last_heartbeat >= 1000) {
        ct_log_harvest();
        ct_log_flush_handlers();
        *last_heartbeat = now;
    }
}

static int dispatcher__routine(void* arg) {
    ct_log_dispatcher_t* self = (ct_log_dispatcher_t*)arg;
    ct_log_job_t         job;
    ct_time64_t          last_heartbeat = ct_getuptime_ms();

    while (self->running || ct_atomic_int_load(&self->pending_jobs) > 0) {
        if (ct_msgqueue_pop_for(&self->queue, &job, 100) == 0) {
            if (job.block && job.block->rec_count > 0) {
                uint32_t total_count = job.block->rec_count;
                uint32_t processed   = 0;
                char*    p           = job.block->data;

                while (processed < total_count) {
                    uint32_t batch_size = total_count - processed;
                    if (batch_size > CT_LOG_BATCH_MAX) { batch_size = CT_LOG_BATCH_MAX; }

                    for (uint32_t i = 0; i < batch_size; ++i) {
                        ct_log_record_header_t* header = (ct_log_record_header_t*)p;
                        p += sizeof(ct_log_record_header_t);
                        self->batch_records[i].time  = header->time;
                        self->batch_records[i].level = header->level;
                        self->batch_records[i].data  = p;
                        self->batch_records[i].size  = header->size;
                        p += header->size;
                    }

                    ct_list_foreach_entry(handler, &job.logger->handlers, ct_log_handler_t, node) {
                        if (handler->vtable && handler->vtable->write_batch) {
                            handler->vtable->write_batch(handler, self->batch_records, batch_size);
                        }
                    }
                    processed += batch_size;
                }
            }

            if (job.block) { block__pool_release(self->pool, job.block); }

            ct_mutex_lock(&self->flush_mutex);
            if (ct_atomic_int_sub(&self->pending_jobs, 1) == 1) { ct_cond_broadcast(&self->flush_cond); }
            ct_mutex_unlock(&self->flush_mutex);
        }
        dispatcher__do_heartbeat(&last_heartbeat);
    }
    return 0;
}

ct_log_dispatcher_t* ct_log_dispatcher_create(void) {
    ct_log_dispatcher_t* self = (ct_log_dispatcher_t*)calloc(1, sizeof(ct_log_dispatcher_t));
    if (!self) { return NULL; }

    self->pool = block__pool_create(256, 8192);
    if (!self->pool) {
        free(self);
        return NULL;
    }

    self->q_buf = malloc(sizeof(ct_log_job_t) * 1024);
    if (!self->q_buf) {
        block__pool_destroy(self->pool);
        free(self);
        return NULL;
    }
    ct_msgqueue_init(&self->queue, self->q_buf, sizeof(ct_log_job_t), 1024);
    self->pending_jobs = CT_ATOMIC_VAR_INIT(0);
    ct_mutex_init(&self->flush_mutex);
    ct_cond_init(&self->flush_cond);

    self->running = true;
    if (ct_thread_create(&self->thread, NULL, dispatcher__routine, self) != 0) {
        ct_mutex_destroy(&self->flush_mutex);
        ct_cond_destroy(&self->flush_cond);
        ct_msgqueue_destroy(&self->queue);
        free(self->q_buf);
        block__pool_destroy(self->pool);
        free(self);
        return NULL;
    }

    return self;
}

void ct_log_dispatcher_destroy(ct_log_dispatcher_t* self) {
    if (!self) { return; }
    self->running = false;
    ct_thread_join(self->thread, NULL);

    ct_mutex_destroy(&self->flush_mutex);
    ct_cond_destroy(&self->flush_cond);

    ct_msgqueue_destroy(&self->queue);
    free(self->q_buf);

    block__pool_destroy(self->pool);
    free(self);
}

void ct_log_dispatcher_push_block(ct_log_dispatcher_t* self, ct_logger_t* logger, ct_log_block_t* block) {
    if (!self || !logger || !block || !self->running) {
        if (block) block__pool_release(self->pool, block);
        return;
    }
    if (block->rec_count == 0) {
        block__pool_release(self->pool, block);
        return;
    }

    ct_log_job_t job = {
        .logger = logger,
        .block  = block,
    };
    ct_atomic_int_add(&self->pending_jobs, 1);
    if (ct_msgqueue_push(&self->queue, &job) != 0) {
        ct_atomic_int_sub(&self->pending_jobs, 1);
        block__pool_release(self->pool, block);
    }
}

ct_log_block_pool_t* ct_log_dispatcher_get_pool(ct_log_dispatcher_t* self) {
    return self ? self->pool : NULL;
}

void ct_log_dispatcher_flush(ct_log_dispatcher_t* self) {
    if (!self) { return; }
    ct_mutex_lock(&self->flush_mutex);
    while (ct_atomic_int_load(&self->pending_jobs) > 0) { ct_cond_wait(&self->flush_cond, &self->flush_mutex); }
    ct_mutex_unlock(&self->flush_mutex);
}
