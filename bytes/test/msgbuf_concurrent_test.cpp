/**
 * @file msgbuf_concurrent_test.cpp
 * @brief Dynamic message buffer concurrency tests.
 */
#include <thread>

#include "coter/bytes/msgbuf.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/testing/doctest.h"


static constexpr int num_producer_threads  = 4;
static constexpr int buffer_size           = 100;
static constexpr int chunk_size            = 9;
static constexpr int iterations_per_thread = 1000;

typedef struct {
    ct_list_t  free_buffers_;
    ct_list_t  filled_buffers_;
    ct_mutex_t mutex_;
    ct_cond_t  cond_;
    size_t     total_chunks_;
    size_t     idx_;
} buffer_pool_t;

typedef struct {
    buffer_pool_t* free_pool_;
    buffer_pool_t* filled_pool_;
    ct_msgbuf_t*   current_buffer_;
    ct_mutex_t     mutex_;
    char           sample_data_[chunk_size];
    bool           test_complete_;
} test_context_t;

static void msgbuf_init_buffer_pool(buffer_pool_t* pool) {
    ct_list_init(&pool->free_buffers_);
    ct_list_init(&pool->filled_buffers_);
    ct_mutex_init(&pool->mutex_);
    ct_cond_init(&pool->cond_);
    pool->total_chunks_ = 0;
    pool->idx_          = 0;
}

static void msgbuf_destroy_buffer_pool(buffer_pool_t* pool) {
    ct_mutex_destroy(&pool->mutex_);
    ct_cond_destroy(&pool->cond_);
}

static ct_msgbuf_t* msgbuf_get_free_buffer(buffer_pool_t* pool) {
    ct_mutex_lock(&pool->mutex_);
    ct_msgbuf_t* buffer;
    if (ct_list_isempty(&pool->free_buffers_)) {
        buffer = ct_msgbuf_create(buffer_size);
    } else {
        buffer = ct_list_first_entry(&pool->free_buffers_, ct_msgbuf_t, list_);
        ct_list_remove(buffer->list_);
    }
    ct_mutex_unlock(&pool->mutex_);
    return buffer;
}

static void msgbuf_return_filled_buffer(buffer_pool_t* pool, ct_msgbuf_t* buffer) {
    ct_mutex_lock(&pool->mutex_);
    ct_list_append(&pool->filled_buffers_, buffer->list_);
    ++pool->total_chunks_;
    ct_cond_signal(&pool->cond_);
    ct_mutex_unlock(&pool->mutex_);
}

static void msgbuf_consume_chunks(test_context_t* ctx) {
    ct_mutex_lock(&ctx->filled_pool_->mutex_);
    while (ct_list_isempty(&ctx->filled_pool_->filled_buffers_) && !ctx->test_complete_) {
        ct_cond_wait(&ctx->filled_pool_->cond_, &ctx->filled_pool_->mutex_);
    }
    if (ct_list_isempty(&ctx->filled_pool_->filled_buffers_) && ctx->test_complete_) {
        ct_mutex_unlock(&ctx->filled_pool_->mutex_);
        return;
    }
    ct_msgbuf_t* buffer = ct_list_first_entry(&ctx->filled_pool_->filled_buffers_, ct_msgbuf_t, list_);
    ct_list_remove(buffer->list_);
    ct_mutex_unlock(&ctx->filled_pool_->mutex_);
    uint8_t* buffer_data = (uint8_t*)ct_msgbuf_buffer(buffer);
    size_t   buffer_size = ct_msgbuf_size(buffer);
    for (size_t i = 0; i < buffer_size; ++i) {
        REQUIRE(buffer_data[i] == (uint8_t)(0x31 + ctx->filled_pool_->idx_));
        if (++ctx->filled_pool_->idx_ >= chunk_size) { ctx->filled_pool_->idx_ = 0; }
    }
    ct_msgbuf_clear(buffer);
    ct_mutex_lock(&ctx->free_pool_->mutex_);
    ct_list_append(&ctx->free_pool_->free_buffers_, buffer->list_);
    ++ctx->free_pool_->total_chunks_;
    ct_mutex_unlock(&ctx->free_pool_->mutex_);
}

static void msgbuf_produce_chunk(test_context_t* ctx) {
    const char* write_ptr = ctx->sample_data_;
    size_t      remaining = chunk_size;
    ct_mutex_lock(&ctx->mutex_);
    while (remaining > 0) {
        size_t written = ct_msgbuf_write(ctx->current_buffer_, write_ptr,
                                         CT_MIN(remaining, ct_msgbuf_available(ctx->current_buffer_)));
        write_ptr += written;
        remaining -= written;
        if (ct_msgbuf_isfull(ctx->current_buffer_)) {
            uint8_t* buffer_data = (uint8_t*)ct_msgbuf_buffer(ctx->current_buffer_);
            size_t   buffer_size = ct_msgbuf_size(ctx->current_buffer_);
            for (size_t i = 0; i < buffer_size; ++i) {
                REQUIRE(buffer_data[i] == (uint8_t)(0x31 + ctx->free_pool_->idx_));
                if (++ctx->free_pool_->idx_ >= chunk_size) { ctx->free_pool_->idx_ = 0; }
            }
            msgbuf_return_filled_buffer(ctx->filled_pool_, ctx->current_buffer_);
            ctx->current_buffer_ = msgbuf_get_free_buffer(ctx->free_pool_);
        }
    }
    ct_mutex_unlock(&ctx->mutex_);
}

static int msgbuf_producer_thread_func(void* arg) {
    test_context_t* ctx = (test_context_t*)arg;
    for (int i = 0; i < iterations_per_thread; ++i) {
        msgbuf_produce_chunk(ctx);
        std::this_thread::yield();
    }
    return 0;
}

static int msgbuf_consumer_thread_func(void* arg) {
    test_context_t* ctx = (test_context_t*)arg;
    while (!ctx->test_complete_) {
        msgbuf_consume_chunks(ctx);
        std::this_thread::yield();
    }
    return 0;
}

TEST_CASE("msgbuf concurrent" * doctest::test_suite("msgbuf") * doctest::test_suite("concurrency")) {
    test_context_t ctx;
    buffer_pool_t  free_pool, filled_pool;
    msgbuf_init_buffer_pool(&free_pool);
    msgbuf_init_buffer_pool(&filled_pool);
    ctx.free_pool_      = &free_pool;
    ctx.filled_pool_    = &filled_pool;
    ctx.current_buffer_ = ct_msgbuf_create(buffer_size);
    ct_mutex_init(&ctx.mutex_);
    ctx.test_complete_ = false;
    for (int i = 0; i < chunk_size; ++i) { ctx.sample_data_[i] = (char)(0x31 + (i % 26)); }
    std::thread producer_threads[num_producer_threads];
    for (int i = 0; i < num_producer_threads; ++i) { producer_threads[i] = std::thread(msgbuf_producer_thread_func, &ctx); }
    std::thread consumer_thread(msgbuf_consumer_thread_func, &ctx);
    for (int i = 0; i < num_producer_threads; ++i) { producer_threads[i].join(); }
    {
        ctx.test_complete_ = true;
        ct_cond_signal(&filled_pool.cond_);
        consumer_thread.join();
    }
    while (!ct_list_isempty(&filled_pool.filled_buffers_)) { msgbuf_consume_chunks(&ctx); }
    ct_msgbuf_destroy(ctx.current_buffer_);
    ct_mutex_destroy(&ctx.mutex_);
    msgbuf_destroy_buffer_pool(&free_pool);
    msgbuf_destroy_buffer_pool(&filled_pool);
    size_t total_bytes     = num_producer_threads * iterations_per_thread * chunk_size;
    size_t expected_chunks = (total_bytes / buffer_size) + (total_bytes % buffer_size ? 1 : 0);
    REQUIRE(filled_pool.total_chunks_ == expected_chunks);
    REQUIRE(free_pool.total_chunks_ == expected_chunks);
}
