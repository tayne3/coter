/**
 * @file jobpool_test.c
 * @brief 任务池测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "base/ct_platform.h"
#include "base/ct_time.h"
#include "ctunit.h"
#include "mech/ct_jobpool.h"

#define TEST_DATA_MAX 10000

static bool            test_data[TEST_DATA_MAX] = {0};
static size_t          test_data_size           = 0;
static size_t          test_end_number          = 0;
static pthread_mutex_t test_mutex[1]            = {PTHREAD_MUTEX_INITIALIZER};

static inline void test_job_routine(void *arg) {
	const size_t idx = (size_t)(uintptr_t)arg;
	test_data[idx]   = true;

	pthread_mutex_lock(test_mutex);
	test_end_number++;
	pthread_mutex_unlock(test_mutex);
}

static inline void test_data_reset(void) {
	for (size_t i = 0; i < test_data_size; i++) {
		test_data[i] = false;
	}
}

static inline void *test_job_publish(void *arg) {
	ct_jobpool_t *jobpool = (ct_jobpool_t *)arg;
	for (size_t i = 0; i < test_data_size; i++) {
		ct_jobpool_submit(jobpool, test_job_routine, (void *)(uintptr_t)i);
	}

	return NULL;
}

static inline void test_jobpool_add(size_t data_count, size_t task_count, size_t job_count) {
	ctunit_assert_uint32(data_count, 0, CTUnit_Greater);
	ctunit_assert_uint32(data_count, TEST_DATA_MAX, CTUnit_LessEqual);
	ctunit_assert_uint32(task_count, 0, CTUnit_Greater);
	ctunit_assert_uint32(job_count, 0, CTUnit_Greater);

	test_data_size = data_count;
	test_data_reset();
	test_end_number = 0;

	ct_jobpool_t *jobpool = ct_jobpool_create(task_count, job_count);
	ctunit_assert_not_null(jobpool);

	bool      is_ok;
	pthread_t thread;
	is_ok = pthread_create(&thread, NULL, test_job_publish, jobpool) == 0;
	ctunit_assert_true(is_ok);

	// 等待结束 (超时时长: 10s)
	bool is_timeout = false;
	for (int i = 0; i < 1000; i++) {
		pthread_mutex_lock(test_mutex);
		is_timeout = test_end_number >= test_data_size;
		pthread_mutex_unlock(test_mutex);
		if (is_timeout) {
			break;
		}
		ct_msleep(10);
	}

	is_ok = pthread_join(thread, NULL) == 0;
	ctunit_assert_true(is_ok);

	pthread_mutex_lock(test_mutex);
	ctunit_assert_uint32(test_end_number, test_data_size, CTUnit_Equal);
	pthread_mutex_unlock(test_mutex);

	ct_jobpool_destroy(jobpool);

	ctunit_assert_uint32(test_end_number, test_data_size, CTUnit_Equal);
}

int main(void) {
	test_jobpool_add(10, 1, 10);
	ctunit_trace("Finish! test_jobpool_add(10, 1, 10);\n");

	test_jobpool_add(10, 10, 1);
	ctunit_trace("Finish! test_jobpool_add(10, 10, 1);\n");

	test_jobpool_add(500, 50, 50);
	ctunit_trace("Finish! test_jobpool_add(500, 50, 50);\n");

	pthread_mutex_destroy(test_mutex);
	ctunit_pass();
}
