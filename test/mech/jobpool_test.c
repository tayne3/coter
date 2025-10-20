/**
 * @file jobpool_test.c
 * @brief 任务池测试
 */
#include "coter/mech/jobpool.h"

#include "coter/base/platform.h"
#include "coter/base/time.h"
#include "cunit.h"

#define TEST_DATA_MAX 10000

static struct {
	bool            data[TEST_DATA_MAX];
	size_t          data_size;
	size_t          end_number;
	pthread_mutex_t mutex;
} test_data = {
	.data       = {0},
	.data_size  = 0,
	.end_number = 0,
	.mutex      = PTHREAD_MUTEX_INITIALIZER,
};

static inline void test_job_routine(void *arg) {
	const size_t idx    = (size_t)(uintptr_t)arg;
	test_data.data[idx] = true;

	pthread_mutex_lock(&test_data.mutex);
	test_data.end_number++;
	pthread_mutex_unlock(&test_data.mutex);
}

static inline void test_data_reset(void) {
	for (size_t i = 0; i < test_data.data_size; i++) {
		test_data.data[i] = false;
	}
}

static inline void *test_job_publish(void *arg) {
	ct_jobpool_t *jobpool = (ct_jobpool_t *)arg;
	for (size_t i = 0; i < test_data.data_size; i++) {
		assert_int32_eq(0, ct_jobpool_submit(jobpool, test_job_routine, (void *)(uintptr_t)i));
	}

	return NULL;
}

static void setup(void) {
	test_data_reset();
	test_data.data_size  = 0;
	test_data.end_number = 0;
}

static inline void test_jobpool_add(size_t data_count, size_t task_count, size_t job_count) {
	assert_uint32_gt(data_count, 0);
	assert_uint32_le(data_count, TEST_DATA_MAX);
	assert_uint32_gt(task_count, 0);
	assert_uint32_gt(job_count, 0);

	test_data.data_size = data_count;
	ct_jobpool_t *jobpool = ct_jobpool_create(task_count, job_count);
	assert_not_null(jobpool);

	pthread_t thread;
	assert_int32_eq(0, pthread_create(&thread, NULL, test_job_publish, jobpool));

	// 等待结束 (超时时长: 5s)
	bool is_end = false;
	for (int i = 0; i < 1000; ++i) {
		pthread_mutex_lock(&test_data.mutex);
		is_end = test_data.end_number >= test_data.data_size;
		pthread_mutex_unlock(&test_data.mutex);
		if (is_end) {
			break;
		}
		ct_msleep(5);
	}

	assert_int32_eq(0, pthread_join(thread, NULL));

	pthread_mutex_lock(&test_data.mutex);
	assert_uint32_eq(test_data.end_number, test_data.data_size);
	pthread_mutex_unlock(&test_data.mutex);

	ct_jobpool_destroy(jobpool);
	assert_uint32_eq(test_data.end_number, test_data.data_size);
}

void test_jobpool_add_10_1_10(void) {
	test_jobpool_add(10, 1, 10);
}

void test_jobpool_add_10_10_1(void) {
	test_jobpool_add(10, 10, 1);
}

void test_jobpool_add_500_50_50(void) {
	test_jobpool_add(500, 10, 1);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("jobpool", setup, NULL)
	CUNIT_TEST("add_10_1_10", test_jobpool_add_10_1_10)
	CUNIT_TEST("add_10_10_1", test_jobpool_add_10_10_1)
	CUNIT_TEST("add_500_50_50", test_jobpool_add_500_50_50)
	CUNIT_SUITE_END()

	return cunit_run();
}
