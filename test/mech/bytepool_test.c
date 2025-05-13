/**
 * @file bytepool_test.c
 * @brief 字节池测试
 * @author tayne3@dingtalk.com
 * @date 2024.2.10
 */
#include "base/ct_time.h"
#include "ctunit.h"
#include "mech/ct_bytepool.h"

#define TEST_THREADS    24
#define TEST_ITERATIONS 20480
#define MIN_BUFFER_SIZE 1
#define MAX_BUFFER_SIZE 1024

#define CONCURRENCY_THREADS    10
#define CONCURRENCY_ITERATIONS 1000
#define MEMORY_LEAK_ITERATIONS 10000

// 基本功能测试
static void test_bytepool_basic_operations(void) {
	ct_bytepool_t* pool = ct_bytepool_create(10, 100);
	ctunit_assert_not_null(pool);

	// 测试获取和放回字节
	for (int i = 0; i < 15; i++) {
		ct_bytes_t* bytes = ct_bytepool_get(pool);
		ctunit_assert_not_null(bytes);
		ctunit_assert_uint32_equal(ct_bytes_capacity(bytes), 100);
		ctunit_assert_uint32_equal(ct_bytes_size(bytes), 0);
		ctunit_assert_uint32_equal(ct_bytes_available(bytes), 100);

		// 模拟使用字节
		for (int j = 0; j < 50; j++) {
			ct_bytes_write(bytes, "A", 1);
		}

		ct_bytepool_put(pool, bytes);
	}

	// 测试池容量
	ct_bytes_t* bytes_array[20];
	int         count = 0;
	for (int i = 0; i < 20; i++) {
		bytes_array[i] = ct_bytepool_get(pool);
		if (bytes_array[i] != NULL) {
			count++;
		}
	}
	ctunit_assert_int32_equal(count, 20);

	for (int i = 0; i < count; i++) {
		ct_bytepool_put(pool, bytes_array[i]);
	}

	ct_bytepool_destroy(pool);
}

// 边界情况测试
static void test_bytepool_edge_cases(void) {
	// 测试创建最小池
	ct_bytepool_t* small_pool = ct_bytepool_create(1, MIN_BUFFER_SIZE);
	ctunit_assert_not_null(small_pool);

	ct_bytes_t* small_bytes = ct_bytepool_get(small_pool);
	ctunit_assert_not_null(small_bytes);
	ctunit_assert_uint32_equal(small_bytes->cap, MIN_BUFFER_SIZE);

	ct_bytepool_put(small_pool, small_bytes);
	ct_bytepool_destroy(small_pool);

	// 测试创建大池
	ct_bytepool_t* large_pool = ct_bytepool_create(1000, MAX_BUFFER_SIZE);
	ctunit_assert_not_null(large_pool);

	ct_bytes_t* large_bytes = ct_bytepool_get(large_pool);
	ctunit_assert_not_null(large_bytes);
	ctunit_assert_uint32_equal(large_bytes->cap, MAX_BUFFER_SIZE);

	ct_bytepool_put(large_pool, large_bytes);
	ct_bytepool_destroy(large_pool);
}

// 辅助函数
static void* thread_func_with_pool(void* arg) {
	ct_bytepool_t* pool = (ct_bytepool_t*)arg;
	for (int i = 0; i < TEST_ITERATIONS; i++) {
		ct_bytes_t* bytes = ct_bytepool_get(pool);
		ctunit_assert_not_null(bytes);

		sched_yield();
		// ct_msleep(2);
		ct_bytepool_put(pool, bytes);
	}
	return NULL;
}

// 辅助函数
static void* thread_func_without_pool(void* arg) {
	for (int i = 0; i < TEST_ITERATIONS; i++) {
		char* buffer = (char*)malloc(MAX_BUFFER_SIZE);
		ctunit_assert_not_null(buffer);

		sched_yield();
		// ct_msleep(2);
		free(buffer);
	}
	return NULL;
	(void)arg;
}

// 并发测试
static void test_bytepool_concurrency(void) {
	ct_bytepool_t* pool = ct_bytepool_create(CONCURRENCY_THREADS * 2, MAX_BUFFER_SIZE);
	ctunit_assert_not_null(pool);

	pthread_t threads[CONCURRENCY_THREADS];
	for (int i = 0; i < CONCURRENCY_THREADS; i++) {
		int result = pthread_create(&threads[i], NULL, thread_func_with_pool, pool);
		ctunit_assert_int32_equal(result, 0);
	}

	for (int i = 0; i < CONCURRENCY_THREADS; i++) {
		int result = pthread_join(threads[i], NULL);
		ctunit_assert_int32_equal(result, 0);
	}

	// 验证池的状态
	int available_bytes = 0;
	for (int i = 0; i < CONCURRENCY_THREADS * 2; i++) {
		ct_bytes_t* bytes = ct_bytepool_get(pool);
		if (bytes != NULL) {
			available_bytes++;
			ct_bytepool_put(pool, bytes);
		}
	}
	ctunit_assert_int32_equal(available_bytes, CONCURRENCY_THREADS * 2);

	ct_bytepool_destroy(pool);
}

// 性能对比测试
static void test_bytepool_performance(void) {
	pthread_t   threads[TEST_THREADS];
	ct_time64_t start, end;
	bool        is_ok;

	// 不使用字节池的情况
	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, thread_func_without_pool, NULL);
		ctunit_assert_true(is_ok);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		ctunit_assert_true(is_ok);
	}
	end                                 = ct_getuptime_ms();
	const ct_time64_t time_without_pool = end - start;

	// 使用字节池的情况
	// ct_bytepool_t* pool = ct_bytepool_create(TEST_THREADS * 2, MAX_BUFFER_SIZE);
	ct_bytepool_t* pool = ct_bytepool_create(1024, MAX_BUFFER_SIZE);
	start               = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, thread_func_with_pool, pool);
		ctunit_assert_true(is_ok);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		ctunit_assert_true(is_ok);
	}
	end                              = ct_getuptime_ms();
	const ct_time64_t time_with_pool = end - start;
	ct_bytepool_destroy(pool);

	ctunit_trace("Performance comparison: With byte pool %" PRIu64 "ms, Without byte pool %" PRIu64 "ms\n",
				 time_with_pool, time_without_pool);
	// ctunit_assert_true(time_with_pool < time_without_pool);
}

int main(void) {
	srand((unsigned int)time(NULL));  // 初始化随机数生成器

	test_bytepool_basic_operations();
	ctunit_trace("Finish! test_bytepool_basic_operations();\n");

	test_bytepool_edge_cases();
	ctunit_trace("Finish! test_bytepool_edge_cases();\n");

	test_bytepool_concurrency();
	ctunit_trace("Finish! test_bytepool_concurrency();\n");

	test_bytepool_performance();
	ctunit_trace("Finish! test_bytepool_performance();\n");

	ctunit_pass();
}
