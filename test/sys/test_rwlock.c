/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2024.2.10
 */
#include "common/ct_random.h"
#include "common/ct_time.h"
#include "ctunit.h"
#include "sys/ct_cond.h"
#include "sys/ct_mutex.h"
#include "sys/ct_rwlock.h"
#include "sys/ct_thread.h"

#define TEST_THREAD_NUMBER 8
#define TEST_DATA_SIZE     100000

static ct_rwlock_t test_rwlock                = CT_RWLOCK_INITIALIZATION;
static uint64_t    test_datas[TEST_DATA_SIZE] = {0};

static inline void *test_reader_thread(void *arg);
static inline void *test_writer_thread(void *arg);

static inline void test_1(void);

int main(void)
{
	test_1();

	ctunit_pass();
}

static inline void *test_reader_thread(void *arg)
{
	const size_t idx = (size_t)(uint64_t)arg;
	for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
		ct_rwlock_rlock(&test_rwlock);
		test_datas[idx]++;
		ct_rwlock_unlock(&test_rwlock);
	}
	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
}

static inline void *test_writer_thread(void *arg)
{
	for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
		ct_rwlock_wlock(&test_rwlock);
		for (size_t idx = 0; idx < TEST_THREAD_NUMBER; idx++) {
			test_datas[idx]++;
		}
		ct_rwlock_unlock(&test_rwlock);
	}
	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
	ct_unused(arg);
}

static inline void test_1(void)
{
	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		test_datas[i] = 0;
	}

	ct_thread_t rthreads[TEST_THREAD_NUMBER];
	ct_thread_t wthreads[TEST_THREAD_NUMBER];

	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ct_thread_create(&rthreads[i], test_reader_thread, (void *)(uint64_t)i);
		ct_thread_create(&wthreads[i], test_writer_thread, ct_nullptr);
	}

	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ct_thread_join(rthreads[i], ct_nullptr);
		ct_thread_join(wthreads[i], ct_nullptr);
	}

	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ctunit_assert_uint64(test_datas[i], (TEST_THREAD_NUMBER + 1) * TEST_DATA_SIZE, CTUnit_Equal);
		test_datas[i] = 0;
	}
}
