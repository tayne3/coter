/**
 * @file test_thpool.c
 * @brief 线程池测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "base/ct_platform.h"
#include "base/ct_time.h"
#include "ctunit.h"
#include "mech/ct_thpool.h"

#define TEST_DATA_MAX 10000

static bool            test_data[TEST_DATA_MAX] = {0};
static size_t          test_data_size           = 0;
static size_t          test_end_number          = 0;
static pthread_mutex_t test_mutex[1]            = {PTHREAD_MUTEX_INITIALIZER};

static inline void test_data_reset(void);
static inline void test_job_run(void *arg);
static inline void test_thpool_add(size_t data_count);

int main(void) {
	test_thpool_add(1);
	ctunit_trace("Finish! test_thpool_add(1);\n");

	test_thpool_add(10);
	ctunit_trace("Finish! test_thpool_add(10);\n");

	test_thpool_add(100);
	ctunit_trace("Finish! test_thpool_add(100);\n");

	pthread_mutex_destroy(test_mutex);

	ctunit_pass();
}

static inline void test_data_reset(void) {
	for (size_t i = 0; i < test_data_size; i++) {
		test_data[i] = false;
	}
}

static inline void test_job_run(void *arg) {
	const size_t idx = (size_t)(uintptr_t)arg;
	test_data[idx]   = true;

	pthread_mutex_lock(test_mutex);
	test_end_number++;
	pthread_mutex_unlock(test_mutex);
}

static inline void test_thpool_add(size_t data_count) {
	ctunit_assert_uint32(data_count, 0, CTUnit_Greater);
	ctunit_assert_uint32(data_count, TEST_DATA_MAX, CTUnit_LessEqual);

	test_data_size = data_count;
	test_data_reset();
	test_end_number = 0;

	ct_thpool_t *pool = ct_thpool_create(NULL);
	ctunit_assert_not_null(pool);

	int ret = 0;
	for (size_t i = 0; i < test_data_size; i++) {
		ret = ct_thpool_add(pool, NULL, test_job_run, (void *)(uintptr_t)i);
		ctunit_assert_ret(ret, "i = %zu/%zu", i, test_data_size);
	}

	// 等待结束 (最多等待 5s)
	{
		for (int i = 0; i < 500; i++) {
			pthread_mutex_lock(test_mutex);
			if (test_end_number >= test_data_size) {
				pthread_mutex_unlock(test_mutex);
				break;
			} else {
				pthread_mutex_unlock(test_mutex);
				ct_msleep(10);
			}
		}
	}

	ct_thpool_destroy(pool);

	ctunit_assert_uint32(test_end_number, test_data_size, CTUnit_Equal);
}
