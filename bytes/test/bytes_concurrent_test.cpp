#include <catch.hpp>

#include "coter/bytes/bytes.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"

static constexpr int NUM_PRODUCER_THREADS  = 4;
static constexpr int BUFFER_SIZE           = 100;
static constexpr int CHUNK_SIZE            = 9;
static constexpr int ITERATIONS_PER_THREAD = 1000;

typedef struct {
    ct_list_t  free_buffers;
    ct_list_t  filled_buffers;
    ct_mutex_t mutex;
    ct_cond_t  cond;
    size_t     total_chunks;
    size_t     idx;
} buffer_pool_t;

typedef struct {
    buffer_pool_t* free_pool;
    buffer_pool_t* filled_pool;
    ct_bytes_t*    current_buffer;
    ct_mutex_t     mutex;
    char           sample_data[CHUNK_SIZE];
    bool           test_complete;
} test_context_t;

static void init_buffer_pool(buffer_pool_t* pool) {
    ct_list_init(&pool->free_buffers);
    ct_list_init(&pool->filled_buffers);
    ct_mutex_init(&pool->mutex);
    ct_cond_init(&pool->cond);
    pool->total_chunks = 0;
    pool->idx          = 0;
}

static void destroy_buffer_pool(buffer_pool_t* pool) {
    ct_mutex_destroy(&pool->mutex);
    ct_cond_destroy(&pool->cond);
}

static ct_bytes_t* get_free_buffer(buffer_pool_t* pool) {
    ct_mutex_lock(&pool->mutex);
    ct_bytes_t* buffer;
    if (ct_list_isempty(&pool->free_buffers)) {
        buffer = ct_bytes_create(BUFFER_SIZE);
    } else {
        buffer = ct_list_first_entry(&pool->free_buffers, ct_bytes_t, list);
        ct_list_remove(buffer->list);
    }
    ct_mutex_unlock(&pool->mutex);
    return buffer;
}

static void return_filled_buffer(buffer_pool_t* pool, ct_bytes_t* buffer) {
    ct_mutex_lock(&pool->mutex);
    ct_list_append(&pool->filled_buffers, buffer->list);
    pool->total_chunks++;
    ct_cond_signal(&pool->cond);
    ct_mutex_unlock(&pool->mutex);
}

static void consume_chunks(test_context_t* ctx) {
    ct_mutex_lock(&ctx->filled_pool->mutex);
    while (ct_list_isempty(&ctx->filled_pool->filled_buffers) && !ctx->test_complete) {
        ct_cond_wait(&ctx->filled_pool->cond, &ctx->filled_pool->mutex);
    }
    if (ct_list_isempty(&ctx->filled_pool->filled_buffers) && ctx->test_complete) {
        ct_mutex_unlock(&ctx->filled_pool->mutex);
        return;
    }
    ct_bytes_t* buffer = ct_list_first_entry(&ctx->filled_pool->filled_buffers, ct_bytes_t, list);
    ct_list_remove(buffer->list);
    ct_mutex_unlock(&ctx->filled_pool->mutex);
    uint8_t* buffer_data = (uint8_t*)ct_bytes_buffer(buffer);
    size_t   buffer_size = ct_bytes_size(buffer);
    for (size_t i = 0; i < buffer_size; ++i) {
        REQUIRE(buffer_data[i] == (uint8_t)(0x31 + ctx->filled_pool->idx));
        if (++ctx->filled_pool->idx >= CHUNK_SIZE) { ctx->filled_pool->idx = 0; }
    }
    ct_bytes_clear(buffer);
    ct_mutex_lock(&ctx->free_pool->mutex);
    ct_list_append(&ctx->free_pool->free_buffers, buffer->list);
    ctx->free_pool->total_chunks++;
    ct_mutex_unlock(&ctx->free_pool->mutex);
}

static void produce_chunk(test_context_t* ctx) {
    const char* write_ptr = ctx->sample_data;
    size_t      remaining = CHUNK_SIZE;
    ct_mutex_lock(&ctx->mutex);
    while (remaining > 0) {
        size_t written =
            ct_bytes_write(ctx->current_buffer, write_ptr, CT_MIN(remaining, ct_bytes_available(ctx->current_buffer)));
        write_ptr += written;
        remaining -= written;
        if (ct_bytes_isfull(ctx->current_buffer)) {
            uint8_t* buffer_data = (uint8_t*)ct_bytes_buffer(ctx->current_buffer);
            size_t   buffer_size = ct_bytes_size(ctx->current_buffer);
            for (size_t i = 0; i < buffer_size; ++i) {
                REQUIRE(buffer_data[i] == (uint8_t)(0x31 + ctx->free_pool->idx));
                if (++ctx->free_pool->idx >= CHUNK_SIZE) { ctx->free_pool->idx = 0; }
            }
            return_filled_buffer(ctx->filled_pool, ctx->current_buffer);
            ctx->current_buffer = get_free_buffer(ctx->free_pool);
        }
    }
    ct_mutex_unlock(&ctx->mutex);
}

static int producer_thread_func(void* arg) {
    test_context_t* ctx = (test_context_t*)arg;
    for (int i = 0; i < ITERATIONS_PER_THREAD; ++i) {
        produce_chunk(ctx);
        ct_thread_yield();
    }
    return 0;
}

static int consumer_thread_func(void* arg) {
    test_context_t* ctx = (test_context_t*)arg;
    while (!ctx->test_complete) {
        consume_chunks(ctx);
        ct_thread_yield();
    }
    return 0;
}

TEST_CASE("bytes concurrent", "[bytes][concurrency]") {
    test_context_t ctx;
    buffer_pool_t  free_pool, filled_pool;
    init_buffer_pool(&free_pool);
    init_buffer_pool(&filled_pool);
    ctx.free_pool      = &free_pool;
    ctx.filled_pool    = &filled_pool;
    ctx.current_buffer = ct_bytes_create(BUFFER_SIZE);
    ct_mutex_init(&ctx.mutex);
    ctx.test_complete = false;
    for (int i = 0; i < CHUNK_SIZE; ++i) { ctx.sample_data[i] = (char)(0x31 + (i % 26)); }
    ct_thread_t producer_threads[NUM_PRODUCER_THREADS];
    ct_thread_t consumer_thread;
    for (int i = 0; i < NUM_PRODUCER_THREADS; ++i) {
        int ret = ct_thread_create(&producer_threads[i], nullptr, producer_thread_func, &ctx);
        REQUIRE(ret == 0);
    }
    {
        int ret = ct_thread_create(&consumer_thread, nullptr, consumer_thread_func, &ctx);
        REQUIRE(ret == 0);
    }
    for (int i = 0; i < NUM_PRODUCER_THREADS; ++i) {
        int ret = ct_thread_join(producer_threads[i], nullptr);
        REQUIRE(ret == 0);
    }
    {
        ctx.test_complete = true;
        ct_cond_signal(&filled_pool.cond);
        int ret = ct_thread_join(consumer_thread, nullptr);
        REQUIRE(ret == 0);
    }
    while (!ct_list_isempty(&filled_pool.filled_buffers)) { consume_chunks(&ctx); }
    ct_bytes_destroy(ctx.current_buffer);
    ct_mutex_destroy(&ctx.mutex);
    destroy_buffer_pool(&free_pool);
    destroy_buffer_pool(&filled_pool);
    size_t total_bytes     = NUM_PRODUCER_THREADS * ITERATIONS_PER_THREAD * CHUNK_SIZE;
    size_t expected_chunks = (total_bytes / BUFFER_SIZE) + (total_bytes % BUFFER_SIZE ? 1 : 0);
    REQUIRE(filled_pool.total_chunks == expected_chunks);
    REQUIRE(free_pool.total_chunks == expected_chunks);
}
