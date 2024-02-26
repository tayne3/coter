/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.12.29
 */
#include "common/ct_random.h"
#include "common/ct_time.h"
#include "ctunit.h"
#include "sys/ct_cond.h"
#include "sys/ct_mutex.h"
#include "sys/ct_spinlock.h"
#include "sys/ct_thread.h"

#define TEST_THREAD_NUMBER 8
#define TEST_DATA_SIZE     100000

static ct_spinlock_buf_t test_spinlock              = {CT_SPINLOCK_INITIALIZATION};
static uint64_t          test_datas[TEST_DATA_SIZE] = {0};

static ct_mutex_buf_t start_mutex = {CT_MUTEX_INITIALIZATION};
static ct_cond_buf_t  start_cond  = {CT_COND_INITIALIZATION};
static bool           start_flag  = false;

static ct_spinlock_t spinlock;
static ct_mutex_t    mutex;
static int64_t       count         = 0;
static size_t        target_number = 0;

static inline void *test_spinlock_thread(void *arg);
static inline void  test_spinlock_test(void);

static inline void *test_thread_1(void *arg);
static inline void *test_thread_2(void *arg);
static inline void  test_spinlock_1(size_t thread_number, size_t number);
static inline void  test_spinlock_2(size_t number);

int main(void)
{
	test_spinlock_test();

	test_spinlock_1(1, 10000);
	test_spinlock_1(4, 10000);
	test_spinlock_1(8, 10000);
	test_spinlock_1(16, 10000);

	test_spinlock_2(10000);
	test_spinlock_2(30000);
	test_spinlock_2(50000);

	test_spinlock_2(100000);
	test_spinlock_2(300000);
	test_spinlock_2(500000);

	ctunit_pass();
}

static inline void *test_spinlock_thread(void *arg)
{
	for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
		ct_spinlock_lock(test_spinlock);
		for (size_t idx = 0; idx < TEST_THREAD_NUMBER; idx++) {
			test_datas[idx]++;
		}
		ct_spinlock_unlock(test_spinlock);
	}
	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
	ct_unused(arg);
}

static inline void test_spinlock_test(void)
{
	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		test_datas[i] = 0;
	}

	ct_thread_t threads[TEST_THREAD_NUMBER];

	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ct_thread_create(&threads[i], test_spinlock_thread, ct_nullptr);
	}

	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ct_thread_join(&threads[i], ct_nullptr);
	}

	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ctunit_assert_uint64(test_datas[i], TEST_THREAD_NUMBER * TEST_DATA_SIZE, CTUnit_Equal);
		test_datas[i] = 0;
	}
}

static inline void test_spinlock_1(size_t thread_number, const size_t number)
{
	ct_thread_t threads[thread_number];

	ct_random_t random;
	ct_random_init(&random);
	const int64_t init_value = ct_random_int64(&random, 0, 100);
	count                    = init_value;

	target_number = number;

	{
		// 初始化锁
		ct_spinlock_init(&spinlock);

		for (size_t i = 0; i < ct_arrsize(threads); i++) {
			start_flag = false;

			// 创建线程
			ct_thread_create(&threads[i], test_thread_1, (void *)(uint64_t)1);

			// 等待启动完毕
			for (; !start_flag;) {
				ct_thread_usleep(10);
			}
		}

		const ct_timeval_t start_time = ct_current_timeval();

		// 加锁
		ct_mutex_lock(start_mutex);
		// 通知所有线程
		ct_cond_notify_all(start_cond);
		// 解锁
		ct_mutex_unlock(start_mutex);

		// 等待线程结束
		for (size_t i = 0; i < ct_arrsize(threads); i++) {
			ct_thread_join(&threads[i], ct_nullptr);
			ct_thread_usleep(10);
		}

		const ct_timeval_t end_time = ct_current_timeval();

		const ct_timeval_t total_time = ct_timeval_calculate_diff(&end_time, &start_time);

		// 打印执行时间
		ctunit_print("001 - threads(%u) - number(%u) - spinlock time: %03ldms %03ldus" STR_NEWLINE, thread_number,
					 target_number, total_time.tv_sec * 1000 + total_time.tv_usec / 1000, total_time.tv_usec % 1000);

		// 销毁锁
		ct_spinlock_destroy(&spinlock);

		// 检查结果
		ctunit_assert_int32(count, init_value, CTUnit_Equal);
	}

	{
		// 初始化锁
		ct_mutex_init(&mutex);

		for (size_t i = 0; i < ct_arrsize(threads); i++) {
			start_flag = false;

			// 创建线程
			ct_thread_create(&threads[i], test_thread_1, (void *)(uint64_t)0);

			// 等待启动完毕
			ct_forever {
				if (start_flag) {
					break;
				}
				ct_thread_usleep(10);
			}
		}

		const ct_timeval_t start_time = ct_current_timeval();

		// 加锁
		ct_mutex_lock(start_mutex);
		// 通知所有线程
		ct_cond_notify_all(start_cond);
		// 解锁
		ct_mutex_unlock(start_mutex);

		// 等待线程结束
		for (size_t i = 0; i < ct_arrsize(threads); i++) {
			ct_thread_join(&threads[i], ct_nullptr);
			ct_thread_usleep(10);
		}

		const ct_timeval_t end_time = ct_current_timeval();

		const ct_timeval_t total_time = ct_timeval_calculate_diff(&end_time, &start_time);

		// 打印执行时间
		ctunit_print("001 - threads(%u) - number(%u) - mutex time   : %03ldms %03ldus" STR_NEWLINE, thread_number,
					 target_number, total_time.tv_sec * 1000 + total_time.tv_usec / 1000, total_time.tv_usec % 1000);

		// 销毁锁
		ct_mutex_destroy(&mutex);

		// 检查结果
		ctunit_assert_int32(count, init_value, CTUnit_Equal);
	}
}

static inline void test_spinlock_2(size_t number)
{
	target_number = number;
	count         = 0;

	{
		// 初始化锁
		ct_spinlock_init(&spinlock);

		const ct_timeval_t start_time = ct_current_timeval();

		test_thread_2((void *)(uint64_t)1);

		const ct_timeval_t end_time = ct_current_timeval();

		const ct_timeval_t total_time = ct_timeval_calculate_diff(&end_time, &start_time);

		// 打印执行时间
		ctunit_print("002 - number(%u) - spinlock time: %03ldms %03ldus" STR_NEWLINE, target_number,
					 total_time.tv_sec * 1000 + total_time.tv_usec / 1000, total_time.tv_usec % 1000);

		// 销毁锁
		ct_spinlock_destroy(&spinlock);

		// 检查结果
		ctunit_assert_int32(count, 0, CTUnit_Equal);
	}

	{
		// 初始化锁
		ct_mutex_init(&mutex);

		const ct_timeval_t start_time = ct_current_timeval();

		test_thread_2((void *)(uint64_t)0);

		const ct_timeval_t end_time = ct_current_timeval();

		const ct_timeval_t total_time = ct_timeval_calculate_diff(&end_time, &start_time);

		// 打印执行时间
		ctunit_print("002 - number(%u) - mutex time   : %03ldms %03ldus" STR_NEWLINE, target_number,
					 total_time.tv_sec * 1000 + total_time.tv_usec / 1000, total_time.tv_usec % 1000);

		// 销毁锁
		ct_mutex_destroy(&mutex);

		// 检查结果
		ctunit_assert_int32(count, 0, CTUnit_Equal);
	}
}

static inline void *test_thread_1(void *arg)
{
	const uint64_t flag = (uint64_t)arg;

	// 加锁
	ct_mutex_lock(start_mutex);
	// 标记启动
	start_flag = true;
	// 等待条件
	ct_cond_wait(start_cond, start_mutex);
	// 恢复运行
	ct_mutex_unlock(start_mutex);

#define TEST_SPINLOCK_HANDLE           \
	do {                               \
		ct_spinlock_lock(&spinlock);   \
		count++;                       \
		ct_spinlock_unlock(&spinlock); \
		ct_spinlock_lock(&spinlock);   \
		count--;                       \
		ct_spinlock_unlock(&spinlock); \
	} while (0);

#define TEST_MUTEX_HANDLE        \
	do {                         \
		ct_mutex_lock(&mutex);   \
		count++;                 \
		ct_mutex_unlock(&mutex); \
		ct_mutex_lock(&mutex);   \
		count--;                 \
		ct_mutex_unlock(&mutex); \
	} while (0);

	if (flag) {
		for (size_t i = 0; i < target_number; i++) {
			TEST_SPINLOCK_HANDLE
			// TEST_SPINLOCK_HANDLE
			// TEST_SPINLOCK_HANDLE
			// TEST_SPINLOCK_HANDLE
			// TEST_SPINLOCK_HANDLE
		}
	} else {
		for (size_t i = 0; i < target_number; i++) {
			TEST_MUTEX_HANDLE
			// TEST_MUTEX_HANDLE
			// TEST_MUTEX_HANDLE
			// TEST_MUTEX_HANDLE
			// TEST_MUTEX_HANDLE
		}
	}

	return ct_nullptr;
	ct_unused(arg);
}

static inline void *test_thread_2(void *arg)
{
	const uint64_t flag = (uint64_t)arg;

	if (flag) {
		for (; count < (int64_t)target_number;) {
			if (ct_spinlock_try_lock(&spinlock)) {
				count++;
				ct_spinlock_unlock(&spinlock);
				continue;
			}
		}
		for (; count > 0;) {
			if (ct_spinlock_try_lock(&spinlock)) {
				count--;
				ct_spinlock_unlock(&spinlock);
				continue;
			}
		}
	} else {
		for (; count < (int64_t)target_number;) {
			if (ct_mutex_try_lock(&mutex)) {
				count++;
				ct_mutex_unlock(&mutex);
				continue;
			}
		}
		for (; count > 0;) {
			if (ct_mutex_try_lock(&mutex)) {
				count--;
				ct_mutex_unlock(&mutex);
				continue;
			}
		}
	}

	return ct_nullptr;
	ct_unused(arg);
}
