/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include <stdio.h>

#include "ctunit.h"
#include "mech/ct_thpool.h"
#include "sys/ct_mutex.h"
#include "sys/ct_thread.h"

#define TEST_DATA_MAX 10000

static bool           test_data[TEST_DATA_MAX] = {0};
static size_t         test_data_size           = 0;
static size_t         test_end_number          = 0;
static ct_mutex_buf_t test_mutex               = {CT_MUTEX_INITIALIZATION};

static inline void test_data_reset(void);
static inline void test_job_run(void *arg);
static inline void test_thpool_add_job(size_t data_count, size_t task_count, size_t job_count);
static inline void test_thpool_add_task(size_t data_count);

int main(void)
{
	test_thpool_add_job(10, 1, 10);
	test_thpool_add_job(10, 10, 1);
	test_thpool_add_job(5000, 10, 50);

	test_thpool_add_task(1);
	test_thpool_add_task(10);
	test_thpool_add_task(100);

	ctunit_pass();
}

static inline void test_data_reset(void)
{
	for (size_t i = 0; i < test_data_size; i++) {
		test_data[i] = false;
	}
}

static inline void test_job_run(void *arg)
{
	const size_t idx = (size_t)(uint64_t)arg;
	test_data[idx]   = true;

	ct_mutex_lock(test_mutex);
	test_end_number++;
	ct_mutex_unlock(test_mutex);
}

static inline void test_thpool_add_job(size_t data_count, size_t task_count, size_t job_count)
{
	ctunit_assert_uint32(data_count, 0, CTUnit_Greater);
	ctunit_assert_uint32(data_count, TEST_DATA_MAX, CTUnit_LessEqual);
	ctunit_assert_uint32(task_count, 0, CTUnit_Greater);
	ctunit_assert_uint32(job_count, 0, CTUnit_Greater);

	test_data_size = data_count;
	test_data_reset();
	test_end_number = 0;

	ct_thpool_ptr_t pool = ct_thpool_create(task_count, job_count);
	ctunit_assert_not_null(pool);

	for (size_t i = 0; i < test_data_size;) {
		for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
			ct_thpool_add_job(pool, test_job_run, (void *)(uint64_t)i);
		}
		ct_thread_msleep(5);
	}

	// 等待结束 (最多等待 5s)
	{
		ct_mutex_lock(test_mutex);
		for (size_t i = 0; i < 2000 && test_end_number < test_data_size; i++) {
			ct_mutex_unlock(test_mutex);
			ct_thread_msleep(5);
			ct_mutex_lock(test_mutex);
		}
		ct_mutex_unlock(test_mutex);
	}

	ct_thpool_destroy(pool);

	ctunit_assert_uint32(test_end_number, test_data_size, CTUnit_Equal);
}

static inline void test_thpool_add_task(size_t data_count)
{
	ctunit_assert_uint32(data_count, 0, CTUnit_Greater);
	ctunit_assert_uint32(data_count, TEST_DATA_MAX, CTUnit_LessEqual);

	test_data_size = data_count;
	test_data_reset();
	test_end_number = 0;

	ct_thpool_ptr_t pool = ct_thpool_create(1, 1);
	ctunit_assert_not_null(pool);

	for (size_t i = 0; i < test_data_size; i++) {
		ct_thpool_add_task(pool, test_job_run, (void *)(uint64_t)i);
		ct_thread_msleep(5);
	}

	// 等待结束 (最多等待 1s)
	{
		ct_mutex_lock(test_mutex);
		for (size_t i = 0; i < 2000 && test_end_number < test_data_size; i++) {
			ct_mutex_unlock(test_mutex);
			ct_thread_msleep(5);
			ct_mutex_lock(test_mutex);
		}
		ct_mutex_unlock(test_mutex);
	}

	ct_thpool_destroy(pool);

	ctunit_assert_uint32(test_end_number, test_data_size, CTUnit_Equal);
}
