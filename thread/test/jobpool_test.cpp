/**
 * @file jobpool_test.cpp
 * @brief 任务池测试
 */
#include "coter/thread/jobpool.h"

#include <catch.hpp>

#include "coter/core/platform.h"

#define TEST_DATA_MAX 10000

static struct {
	bool            data[TEST_DATA_MAX];
	size_t          data_size;
	size_t          end_number;
	pthread_mutex_t mutex;
} test_data = {
	{0},
	0,
	0,
	PTHREAD_MUTEX_INITIALIZER,
};

static void test_job_routine(void *arg) {
	const size_t idx    = (size_t)(uintptr_t)arg;
	test_data.data[idx] = true;

	pthread_mutex_lock(&test_data.mutex);
	test_data.end_number++;
	pthread_mutex_unlock(&test_data.mutex);
}

static void test_data_reset(void) {
	for (size_t i = 0; i < test_data.data_size; ++i) { test_data.data[i] = false; }
}

static void *test_job_publish(void *arg) {
	ct_jobpool_t *jobpool = (ct_jobpool_t *)arg;
	for (size_t i = 0; i < test_data.data_size; ++i) { REQUIRE(ct_jobpool_submit(jobpool, test_job_routine, (void *)(uintptr_t)i) == 0); }

	return nullptr;
}

static void setup(void) {
	test_data_reset();
	test_data.data_size  = 0;
	test_data.end_number = 0;
}

static void test_jobpool_add(size_t data_count, size_t task_count, size_t job_count) {
	setup();
	REQUIRE(data_count > 0);
	REQUIRE(data_count <= TEST_DATA_MAX);
	REQUIRE(task_count > 0);
	REQUIRE(job_count > 0);

	test_data.data_size   = data_count;
	ct_jobpool_t *jobpool = ct_jobpool_create(task_count, job_count);
	REQUIRE(jobpool != nullptr);

	pthread_t thread;
	REQUIRE(pthread_create(&thread, nullptr, test_job_publish, jobpool) == 0);

	// 等待结束 (超时时长: 5s)
	bool is_end = false;
	for (int i = 0; i < 1000; ++i) {
		pthread_mutex_lock(&test_data.mutex);
		is_end = test_data.end_number >= test_data.data_size;
		pthread_mutex_unlock(&test_data.mutex);
		if (is_end) { break; }
		ct_msleep(5);
	}

	REQUIRE(pthread_join(thread, nullptr) == 0);

	pthread_mutex_lock(&test_data.mutex);
	REQUIRE(test_data.end_number == test_data.data_size);
	pthread_mutex_unlock(&test_data.mutex);

	ct_jobpool_destroy(jobpool);
	REQUIRE(test_data.end_number == test_data.data_size);
}

TEST_CASE("jobpool_add_10_1_10", "[jobpool]") {
	test_jobpool_add(10, 1, 10);
}

TEST_CASE("jobpool_add_10_10_1", "[jobpool]") {
	test_jobpool_add(10, 10, 1);
}

TEST_CASE("jobpool_add_500_50_50", "[jobpool]") {
	test_jobpool_add(500, 10, 1);
}
