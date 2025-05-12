/**
 * @file msgqueue_test.c
 * @brief 消息队列测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "base/ct_platform.h"
#include "base/ct_random.h"
#include "cunit.h"
#include "mech/ct_msgqueue.h"

static size_t test_data_size = 0;
static int   *test_data;

static inline void *test_task_enqueue(void *arg) {
	ct_msgqueue_t *msgqueue = (ct_msgqueue_t *)arg;
	for (size_t i = 0; i < test_data_size;) {
		for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
			ct_msgqueue_enqueue(msgqueue, &test_data[i]);
		}
		sched_yield();
	}

	return NULL;
}

static inline void test_msgqueue(size_t data_size, size_t buffer_size) {
	cunit_assert_uint32_greater(data_size, 0);
	cunit_assert_uint32_greater(buffer_size, 0);

	test_data_size   = data_size;
	test_data        = (int *)calloc(test_data_size, sizeof(int));
	int *test_buffer = (int *)calloc(buffer_size, sizeof(int));
	cunit_assert_not_null(test_data);
	cunit_assert_not_null(test_buffer);

	// 初始化测试数据
	{
		ct_random_buf_t ctrand;
		ct_random_init(ctrand);
		for (size_t i = 0; i < test_data_size; i++) {
			test_data[i] = ct_random_int32(ctrand, INT32_MIN, INT32_MAX);
		}
	}

	// 初始化队列
	ct_msgqueue_buf_t msgqueue;
	ct_msgqueue_init(msgqueue, test_buffer, sizeof(int), buffer_size);
	cunit_assert_true(ct_msgqueue_isempty(msgqueue));
	cunit_assert_false(ct_msgqueue_isfull(msgqueue));

	// 测试 enqueue 和 dequeue
	{
		bool      is_ok;
		pthread_t thread;
		is_ok = pthread_create(&thread, NULL, test_task_enqueue, msgqueue) == 0;
		cunit_assert_true(is_ok);

		int item = 0;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				is_ok = ct_msgqueue_dequeue(msgqueue, &item);
				cunit_assert_true(is_ok);
				cunit_assert_int32_equal(item, test_data[i], "buffer_size=%d, data_size=%d", buffer_size, data_size);
			}
			sched_yield();
		}

		is_ok = pthread_join(thread, NULL) == 0;
		cunit_assert_true(is_ok);

		cunit_assert_true(ct_msgqueue_isempty(msgqueue));
		cunit_assert_false(ct_msgqueue_isfull(msgqueue));
	}

	// 测试 try_enqueue 和 try_dequeue
	{
		bool is_ok;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				is_ok = ct_msgqueue_try_enqueue(msgqueue, &test_buffer[i]);
				if (i < buffer_size) {
					cunit_assert_true(is_ok);
				} else {
					cunit_assert_false(is_ok);
				}
			}
			sched_yield();
		}

		cunit_assert_false(ct_msgqueue_isempty(msgqueue));
		cunit_assert_bool(ct_msgqueue_isfull(msgqueue), buffer_size <= test_data_size);

		int item = 0;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				is_ok = ct_msgqueue_try_dequeue(msgqueue, &item);

				if (i < buffer_size) {
					cunit_assert_true(is_ok);
					cunit_assert_int32_equal(item, test_buffer[i]);
				} else {
					cunit_assert_false(is_ok);
				}
			}
			sched_yield();
		}

		cunit_assert_true(ct_msgqueue_isempty(msgqueue));
		cunit_assert_false(ct_msgqueue_isfull(msgqueue));
	}

	// 关闭队列
	ct_msgqueue_close(msgqueue);
	// 销毁队列
	ct_msgqueue_destroy(msgqueue);

	free(test_data);
	free(test_buffer);
}

int main(void) {
	test_msgqueue(10, 1);
	cunit_println("Finish! test_msgqueue(10, 1);\n");

	test_msgqueue(1, 10);
	cunit_println("Finish! test_msgqueue(1, 10);\n");

	test_msgqueue(500, 10);
	cunit_println("Finish! test_msgqueue(500, 10);\n");

	test_msgqueue(500, 500);
	cunit_println("Finish! test_msgqueue(500, 500);\n");

	cunit_pass();
}
