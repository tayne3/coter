/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "ctunit.h"
#include "sys/ct_cond.h"
#include "sys/ct_mutex.h"
#include "sys/ct_once.h"
#include "sys/ct_thread.h"

#define TEST_THREAD_MAX 100

static ct_mutex_buf_t start_mutex = {CT_MUTEX_INITIALIZATION};
static ct_cond_buf_t  start_cond  = {CT_COND_INITIALIZATION};
static bool           start_flag  = false;

static int           result = 0;
static ct_once_buf_t once   = {CT_ONCE_INITIALIZATION};

static inline void  test_routine(void);
static inline void *test_call(void *arg);
static inline void  test_once(void);

int main(void)
{
	test_once();

	ctunit_pass();
}

static inline void test_routine(void)
{
	result++;
}

static inline void *test_call(void *arg)
{
	// 标记启动
	start_flag = true;
	// 加锁
	ct_mutex_lock(start_mutex);
	// 等待条件
	ct_cond_wait(start_cond, start_mutex);
	// 恢复运行
	ct_mutex_unlock(start_mutex);
	// 单次执行
	ct_once_exec(once, test_routine);
	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
	ct_unused(arg);
}

static inline void test_once(void)
{
	ct_thread_t threads[TEST_THREAD_MAX];

	// 创建线程
	for (size_t i = 0; i < ct_arrsize(threads); i++) {
		start_flag = false;
		// 创建线程
		ct_thread_create(&threads[i], test_call, ct_nullptr);
		// 等待启动完毕
		ct_forever {
			if (start_flag) {
				break;
			}
			ct_thread_usleep(10);
		}
	}

	// 加锁
	ct_mutex_lock(start_mutex);
	// 通知所有线程
	ct_cond_notify_all(start_cond);
	// 解锁
	ct_mutex_unlock(start_mutex);

	// 等待线程结束
	for (size_t i = 0; i < ct_arrsize(threads); i++) {
		ct_thread_join(threads[i], ct_nullptr);
		ct_thread_usleep(10);
	}

	// 检查结果
	ctunit_assert_int(result, 1, CTUnit_Equal);
}
