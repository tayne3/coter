#include "coter/bytes/pool.h"

#include <catch.hpp>

#include "coter/time/time.h"

static constexpr int TEST_THREADS    = 24;
static constexpr int TEST_ITERATIONS = 20480;
static constexpr int MIN_BUFFER_SIZE = 1;
static constexpr int MAX_BUFFER_SIZE = 1024;

static constexpr int CONCURRENCY_THREADS    = 10;

static void test_bytepool_basic_operations(void) {
	ct_bytepool_t* pool = ct_bytepool_create(10, 100);
	REQUIRE(pool != nullptr);

	for (int i = 0; i < 15; i++) {
		ct_bytes_t* bytes = ct_bytepool_get(pool);
		REQUIRE(bytes != nullptr);
		REQUIRE(ct_bytes_capacity(bytes) == 100);
		REQUIRE(ct_bytes_size(bytes) == 0);
		REQUIRE(ct_bytes_available(bytes) == 100);

		for (int j = 0; j < 50; j++) { ct_bytes_write(bytes, "A", 1); }

		ct_bytepool_put(pool, bytes);
	}

	ct_bytes_t* bytes_array[20];
	int         count = 0;
	for (int i = 0; i < 20; i++) {
		bytes_array[i] = ct_bytepool_get(pool);
		if (bytes_array[i] != nullptr) { count++; }
	}
	REQUIRE(count == 20);

	for (int i = 0; i < count; i++) { ct_bytepool_put(pool, bytes_array[i]); }

	ct_bytepool_destroy(pool);
}

static void test_bytepool_edge_cases(void) {
	ct_bytepool_t* small_pool = ct_bytepool_create(1, MIN_BUFFER_SIZE);
	REQUIRE(small_pool != nullptr);

	ct_bytes_t* small_bytes = ct_bytepool_get(small_pool);
	REQUIRE(small_bytes != nullptr);
	REQUIRE(small_bytes->cap == MIN_BUFFER_SIZE);

	ct_bytepool_put(small_pool, small_bytes);
	ct_bytepool_destroy(small_pool);

	ct_bytepool_t* large_pool = ct_bytepool_create(1000, MAX_BUFFER_SIZE);
	REQUIRE(large_pool != nullptr);

	ct_bytes_t* large_bytes = ct_bytepool_get(large_pool);
	REQUIRE(large_bytes != nullptr);
	REQUIRE(large_bytes->cap == MAX_BUFFER_SIZE);

	ct_bytepool_put(large_pool, large_bytes);
	ct_bytepool_destroy(large_pool);
}

static void* thread_func_with_pool(void* arg) {
	ct_bytepool_t* pool = (ct_bytepool_t*)arg;
	for (int i = 0; i < TEST_ITERATIONS; i++) {
		ct_bytes_t* bytes = ct_bytepool_get(pool);
		REQUIRE(bytes != nullptr);
		sched_yield();
		ct_bytepool_put(pool, bytes);
	}
	return nullptr;
}

static void* thread_func_without_pool(void* arg) {
	for (int i = 0; i < TEST_ITERATIONS; i++) {
		char* buffer = (char*)malloc(MAX_BUFFER_SIZE);
		REQUIRE(buffer != nullptr);
		sched_yield();
		free(buffer);
	}
	return nullptr;
	(void)arg;
}

static void test_bytepool_concurrency(void) {
	ct_bytepool_t* pool = ct_bytepool_create(CONCURRENCY_THREADS * 2, MAX_BUFFER_SIZE);
	REQUIRE(pool != nullptr);

	pthread_t threads[CONCURRENCY_THREADS];
	for (int i = 0; i < CONCURRENCY_THREADS; i++) {
		int result = pthread_create(&threads[i], nullptr, thread_func_with_pool, pool);
		REQUIRE(result == 0);
	}

	for (int i = 0; i < CONCURRENCY_THREADS; i++) {
		int result = pthread_join(threads[i], NULL);
		REQUIRE(result == 0);
	}

	int available_bytes = 0;
	for (int i = 0; i < CONCURRENCY_THREADS * 2; i++) {
		ct_bytes_t* bytes = ct_bytepool_get(pool);
		if (bytes != nullptr) {
			available_bytes++;
			ct_bytepool_put(pool, bytes);
		}
	}
	REQUIRE(available_bytes == CONCURRENCY_THREADS * 2);

	ct_bytepool_destroy(pool);
}

static void test_bytepool_performance(void) {
	pthread_t   threads[TEST_THREADS];
	ct_time64_t start, end;
	bool        is_ok;

	start = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, thread_func_without_pool, NULL);
		REQUIRE(is_ok);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		REQUIRE(is_ok);
	}
	end                                 = ct_getuptime_ms();
	const ct_time64_t time_without_pool = end - start;

	ct_bytepool_t* pool = ct_bytepool_create(1024, MAX_BUFFER_SIZE);
	start               = ct_getuptime_ms();
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_create(&threads[i], NULL, thread_func_with_pool, pool);
		REQUIRE(is_ok);
	}
	for (int i = 0; i < TEST_THREADS; i++) {
		is_ok = 0 == pthread_join(threads[i], NULL);
		REQUIRE(is_ok);
	}
	end                              = ct_getuptime_ms();
	const ct_time64_t time_with_pool = end - start;
	ct_bytepool_destroy(pool);

	INFO("Performance comparison: With byte pool " << time_with_pool << "ms, Without byte pool " << time_without_pool << "ms");
}

TEST_CASE("bytepool basic_operations", "[bytes][pool][basic]") {
	test_bytepool_basic_operations();
}

TEST_CASE("bytepool edge_cases", "[bytes][pool][edge]") {
	test_bytepool_edge_cases();
}

TEST_CASE("bytepool concurrency", "[bytes][pool][concurrency]") {
	test_bytepool_concurrency();
}

TEST_CASE("bytepool performance", "[bytes][pool][perf]") {
	test_bytepool_performance();
}
