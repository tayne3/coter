/**
 * @file bytes_concurrent_test.c
 * @brief 字节数组并发操作测试
 */
#include "coter/container/bytes.h"
#include "cunit.h"

/**
 * @brief 测试参数枚举
 */
enum {
	NUM_PRODUCER_THREADS  = 4,   /**< 生产者线程数量 */
	BUFFER_SIZE           = 100, /**< 缓冲区大小 */
	CHUNK_SIZE            = 9,   /**< 数据块大小 */
	ITERATIONS_PER_THREAD = 1000 /**< 每个线程的迭代次数 */
};

/**
 * @brief 缓冲池结构体
 */
typedef struct {
	ct_list_t       free_buffers;   /**< 空闲缓冲区列表 */
	ct_list_t       filled_buffers; /**< 已填充缓冲区列表 */
	pthread_mutex_t mutex;          /**< 互斥锁 */
	pthread_cond_t  cond;           /**< 条件变量 */
	size_t          total_chunks;   /**< 总数据块数 */
	size_t          idx;            /**< 当前索引 */
} buffer_pool_t;

/**
 * @brief 测试上下文结构体
 */
typedef struct {
	buffer_pool_t*  free_pool;               /**< 空闲缓冲池 */
	buffer_pool_t*  filled_pool;             /**< 已填充缓冲池 */
	ct_bytes_t*     current_buffer;          /**< 当前缓冲区 */
	pthread_mutex_t mutex;                   /**< 互斥锁 */
	char            sample_data[CHUNK_SIZE]; /**< 样本数据 */
	bool            test_complete;           /**< 测试完成标志 */
} test_context_t;

/**
 * @brief 初始化缓冲池
 * @param pool 待初始化的缓冲池
 */
static void init_buffer_pool(buffer_pool_t* pool) {
	ct_list_init(&pool->free_buffers);
	ct_list_init(&pool->filled_buffers);
	pthread_mutex_init(&pool->mutex, NULL);
	pthread_cond_init(&pool->cond, NULL);
	pool->total_chunks = 0;
	pool->idx          = 0;
}

/**
 * @brief 销毁缓冲池
 * @param pool 待销毁的缓冲池
 */
static void destroy_buffer_pool(buffer_pool_t* pool) {
	pthread_mutex_destroy(&pool->mutex);
	pthread_cond_destroy(&pool->cond);
}

/**
 * @brief 从空闲池中获取一个缓冲区
 * @param pool 缓冲池
 * @return 获取的缓冲区
 */
static ct_bytes_t* get_free_buffer(buffer_pool_t* pool) {
	pthread_mutex_lock(&pool->mutex);
	ct_bytes_t* buffer;
	if (ct_list_isempty(&pool->free_buffers)) {
		buffer = ct_bytes_create(BUFFER_SIZE);
	} else {
		buffer = ct_list_first_entry(&pool->free_buffers, ct_bytes_t, list);
		ct_list_remove(buffer->list);
	}
	pthread_mutex_unlock(&pool->mutex);
	return buffer;
}

/**
 * @brief 将填满的缓冲区返回到已填充池
 * @param pool 缓冲池
 * @param buffer 已填充的缓冲区
 */
static void return_filled_buffer(buffer_pool_t* pool, ct_bytes_t* buffer) {
	pthread_mutex_lock(&pool->mutex);
	ct_list_append(&pool->filled_buffers, buffer->list);
	pool->total_chunks++;
	pthread_cond_signal(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);
}

/**
 * @brief 消费数据块
 * @param ctx 测试上下文
 */
static void consume_chunks(test_context_t* ctx) {
	pthread_mutex_lock(&ctx->filled_pool->mutex);
	while (ct_list_isempty(&ctx->filled_pool->filled_buffers) && !ctx->test_complete) {
		pthread_cond_wait(&ctx->filled_pool->cond, &ctx->filled_pool->mutex);
	}

	if (ct_list_isempty(&ctx->filled_pool->filled_buffers) && ctx->test_complete) {
		pthread_mutex_unlock(&ctx->filled_pool->mutex);
		return;
	}

	ct_bytes_t* buffer = ct_list_first_entry(&ctx->filled_pool->filled_buffers, ct_bytes_t, list);
	ct_list_remove(buffer->list);
	pthread_mutex_unlock(&ctx->filled_pool->mutex);

	uint8_t* buffer_data = (uint8_t*)ct_bytes_buffer(buffer);
	size_t   buffer_size = ct_bytes_size(buffer);

	for (size_t i = 0; i < buffer_size; i++) {
		assert_uint8_eq(buffer_data[i], 0x31 + ctx->filled_pool->idx);
		if (++ctx->filled_pool->idx >= CHUNK_SIZE) {
			ctx->filled_pool->idx = 0;
		}
	}

	ct_bytes_clear(buffer);

	pthread_mutex_lock(&ctx->free_pool->mutex);
	ct_list_append(&ctx->free_pool->free_buffers, buffer->list);
	ctx->free_pool->total_chunks++;
	pthread_mutex_unlock(&ctx->free_pool->mutex);
}

/**
 * @brief 生产一个数据块
 * @param ctx 测试上下文
 */
static void produce_chunk(test_context_t* ctx) {
	const char* write_ptr = ctx->sample_data;
	size_t      remaining = CHUNK_SIZE;

	pthread_mutex_lock(&ctx->mutex);
	while (remaining > 0) {
		size_t written =
			ct_bytes_write(ctx->current_buffer, write_ptr, CT_MIN(remaining, ct_bytes_available(ctx->current_buffer)));
		write_ptr += written;
		remaining -= written;

		if (ct_bytes_isfull(ctx->current_buffer)) {
			uint8_t* buffer_data = (uint8_t*)ct_bytes_buffer(ctx->current_buffer);
			size_t   buffer_size = ct_bytes_size(ctx->current_buffer);

			// 检查数据
			for (size_t i = 0; i < buffer_size; i++) {
				assert_uint8_eq(buffer_data[i], 0x31 + ctx->free_pool->idx);
				if (++ctx->free_pool->idx >= CHUNK_SIZE) {
					ctx->free_pool->idx = 0;
				}
			}

			return_filled_buffer(ctx->filled_pool, ctx->current_buffer);
			ctx->current_buffer = get_free_buffer(ctx->free_pool);
		}
	}
	pthread_mutex_unlock(&ctx->mutex);
}

/**
 * @brief 生产者线程函数
 * @param arg 测试上下文
 * @return NULL
 */
static void* producer_thread_func(void* arg) {
	test_context_t* ctx = (test_context_t*)arg;
	for (int i = 0; i < ITERATIONS_PER_THREAD; i++) {
		produce_chunk(ctx);
		sched_yield();
	}
	return NULL;
}

/**
 * @brief 消费者线程函数
 * @param arg 测试上下文
 * @return NULL
 */
static void* consumer_thread_func(void* arg) {
	test_context_t* ctx = (test_context_t*)arg;
	while (!ctx->test_complete) {
		consume_chunks(ctx);
		sched_yield();
	}
	return NULL;
}

/**
 * @brief 并发字节数组测试
 */
static void test_bytes_concurrent(void) {
	test_context_t ctx;
	buffer_pool_t  free_pool, filled_pool;

	init_buffer_pool(&free_pool);
	init_buffer_pool(&filled_pool);

	// 初始化测试上下文
	ctx.free_pool      = &free_pool;
	ctx.filled_pool    = &filled_pool;
	ctx.current_buffer = ct_bytes_create(BUFFER_SIZE);
	pthread_mutex_init(&ctx.mutex, NULL);
	ctx.test_complete = false;

	// 初始化sample_data
	for (int i = 0; i < CHUNK_SIZE; i++) {
		ctx.sample_data[i] = 0x31 + (char)(i % 26);
	}

	// 创建并运行线程
	pthread_t producer_threads[NUM_PRODUCER_THREADS];
	pthread_t consumer_thread;

	for (int i = 0; i < NUM_PRODUCER_THREADS; i++) {
		int ret = pthread_create(&producer_threads[i], NULL, producer_thread_func, &ctx);
		assert_int_eq(ret, 0);
	}
	{
		int ret = pthread_create(&consumer_thread, NULL, consumer_thread_func, &ctx);
		assert_int_eq(ret, 0);
	}

	// 等待生产者线程完成
	for (int i = 0; i < NUM_PRODUCER_THREADS; i++) {
		int ret = pthread_join(producer_threads[i], NULL);
		assert_int_eq(ret, 0);
	}
	// 通知消费者线程结束
	{
		ctx.test_complete = true;
		pthread_cond_signal(&filled_pool.cond);
		int ret = pthread_join(consumer_thread, NULL);
		assert_int_eq(ret, 0);
	}
	// 等待所有数据被消费
	while (!ct_list_isempty(&filled_pool.filled_buffers)) {
		consume_chunks(&ctx);
	}

	// 清理资源
	{
		ct_bytes_destroy(ctx.current_buffer);
		pthread_mutex_destroy(&ctx.mutex);
		destroy_buffer_pool(&free_pool);
		destroy_buffer_pool(&filled_pool);
	}

	// 验证结果
	size_t total_bytes     = NUM_PRODUCER_THREADS * ITERATIONS_PER_THREAD * CHUNK_SIZE;
	size_t expected_chunks = (total_bytes / BUFFER_SIZE) + (total_bytes % BUFFER_SIZE ? 1 : 0);
	assert_uint32_eq(filled_pool.total_chunks, expected_chunks);
	assert_uint32_eq(free_pool.total_chunks, expected_chunks);
}

int main(void) {
	test_bytes_concurrent();
	cunit_println("Finish! test_bytes_concurrent();");

	cunit_pass();
}
