/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include <stdio.h>

#include "common/ct_random.h"
#include "ctunit.h"
#include "mech/ct_msgqueue.h"
#include "sys/ct_thread.h"

static size_t test_data_size = 0;
static int   *test_data;

static inline void  test_msgqueue(size_t data_size, size_t buffer_size);
static inline void *test_task_enqueue(void *arg);

int main(void)
{
	test_msgqueue(10, 1);
	test_msgqueue(1, 10);
	test_msgqueue(500, 10);
	test_msgqueue(500, 500);

	ctunit_pass();
}

static inline void test_msgqueue(size_t data_size, size_t buffer_size)
{
	ctunit_assert_uint32(data_size, 0, CTUnit_Greater);
	ctunit_assert_uint32(buffer_size, 0, CTUnit_Greater);

	test_data_size   = data_size;
	test_data        = (int *)calloc(test_data_size, sizeof(int));
	int *test_buffer = (int *)calloc(buffer_size, sizeof(int));
	ctunit_assert_not_null(test_data);
	ctunit_assert_not_null(test_buffer);

	// 初始化测试数据
	{
		ct_random_buf_t random;
		ct_random_init(random);
		for (size_t i = 0; i < test_data_size; i++) {
			test_data[i] = ct_random_int32(random, INT32_MIN, INT32_MAX);
		}
	}

	// 初始化队列
	ct_msgqueue_buf_t msgqueue;
	ct_msgqueue_init(msgqueue, test_buffer, sizeof(int), buffer_size);
	ctunit_assert_true(ct_msgqueue_isempty(msgqueue));
	ctunit_assert_false(ct_msgqueue_isfull(msgqueue));
	ctunit_assert_false(ct_msgqueue_isshut(msgqueue));

	// 测试 enqueue 和 dequeue
	{
		bool isok = false;

		ct_thread_t thread;
		isok = ct_thread_create(&thread, test_task_enqueue, msgqueue);
		ctunit_assert_true(isok);

		int item = 0;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				isok = ct_msgqueue_dequeue(msgqueue, &item);
				ctunit_assert_true(isok);
				ctunit_assert_int(item, test_data[i], CTUnit_Equal, "buffer_size=%d, data_size=%d", buffer_size,
								  data_size);
			}
			ct_thread_msleep(10);
		}

		isok = ct_thread_join(thread, ct_nullptr);
		ctunit_assert_true(isok);

		ctunit_assert_true(ct_msgqueue_isempty(msgqueue));
		ctunit_assert_false(ct_msgqueue_isfull(msgqueue));
		ctunit_assert_false(ct_msgqueue_isshut(msgqueue));
	}

	// 测试 try_enqueue 和 try_dequeue
	{
		bool isok = false;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				isok = ct_msgqueue_try_enqueue(msgqueue, &test_buffer[i]);
				if (i < buffer_size) {
					ctunit_assert_true(isok);
				} else {
					ctunit_assert_false(isok);
				}
			}
			ct_thread_msleep(10);
		}

		ctunit_assert_false(ct_msgqueue_isempty(msgqueue));
		ctunit_assert_bool(ct_msgqueue_isfull(msgqueue), buffer_size <= test_data_size);
		ctunit_assert_false(ct_msgqueue_isshut(msgqueue));

		int item = 0;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				isok = ct_msgqueue_try_dequeue(msgqueue, &item);

				if (i < buffer_size) {
					ctunit_assert_true(isok);
					ctunit_assert_int(item, test_buffer[i], CTUnit_Equal);
				} else {
					ctunit_assert_false(isok);
				}
			}
			ct_thread_msleep(10);
		}

		ctunit_assert_true(ct_msgqueue_isempty(msgqueue));
		ctunit_assert_false(ct_msgqueue_isfull(msgqueue));
		ctunit_assert_false(ct_msgqueue_isshut(msgqueue));
	}

	// 销毁队列
	ct_msgqueue_destroy(msgqueue);
	ctunit_assert_true(ct_msgqueue_isshut(msgqueue));

	free(test_data);
	free(test_buffer);
}

static inline void *test_task_enqueue(void *arg)
{
	ct_msgqueue_t *msgqueue = (ct_msgqueue_t *)arg;
	for (size_t i = 0; i < test_data_size;) {
		for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
			ct_msgqueue_enqueue(msgqueue, &test_data[i]);
		}
		ct_thread_msleep(10);
	}

	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
}
