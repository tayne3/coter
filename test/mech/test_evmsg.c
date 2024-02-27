/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include <stdbool.h>
#include <stdio.h>

#include "ct_app.h"
#include "ctunit.h"
#include "mech/ct_evmsg.h"
#include "sys/ct_thread.h"

#define TEST_THREAD_NUMBER 3
#define TEST_DATA_NUMBER   10000

static bool test_result[TEST_THREAD_NUMBER][TEST_DATA_NUMBER] = {{0}};
static bool is_exit[TEST_THREAD_NUMBER]                       = {0};

// 模拟事件处理函数
static inline bool test_evmsg_handler(ct_evmsg_msg_t *msg, void *userdata);
// 测试事件发布
static inline void *test_evmsg_publish(void *arg);

int main(void)
{
	ct_thread_buf_t threads[TEST_THREAD_NUMBER];

	ct_app_create();

	// 订阅事件
	ct_evmsg_subscribe(1, test_evmsg_handler, ct_nullptr);

	// 创建线程
	for (uint8_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ct_thread_create(threads[i], test_evmsg_publish, (void *)(uint64_t)i);
	}

	// 调度事件管理
	{
		for (size_t count = 0; count < TEST_THREAD_NUMBER;) {
			for (size_t n = 0; n < 1000 && count < TEST_THREAD_NUMBER; n++) {
				ct_evmsg_center_schedule();
				for (count = 0; count < TEST_THREAD_NUMBER && is_exit[count]; count++) {}
			}
			ct_thread_msleep(5);
		}
		ct_evmsg_center_schedule();
		ct_evmsg_center_destroy();
	}

	// 等待线程结束
	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		ct_thread_join(*threads[i], ct_nullptr);
	}

	for (size_t id = 0; id < TEST_THREAD_NUMBER; id++) {
		for (size_t i = 0; i < TEST_DATA_NUMBER; i++) {
			ctunit_assert_true(test_result[id][i], "id = %ld, i = %ld", id, i);
		}
		ct_thread_msleep(1);
	}

	ctunit_pass();
}

static inline bool test_evmsg_handler(ct_evmsg_msg_t *msg, void *userdata)
{
	ctunit_assert_not_null(msg);
	ctunit_assert_not_null(msg->data);
	ctunit_assert_uint8(msg->id, 0, CTUnit_GreaterEqual);
	ctunit_assert_uint8(msg->id, TEST_THREAD_NUMBER, CTUnit_Less);

	size_t i = *(size_t *)msg->data;

	ctunit_assert_false(test_result[msg->id][i]);

	test_result[msg->id][i] = true;

	return false;
	ct_unused(userdata);
}

static inline void *test_evmsg_publish(void *arg)
{
	const uint8_t id = (uint8_t)(uint64_t)arg;

	// 模拟事件数据
	ct_evmsg_msg_buf_t msg = {{.type = 1, .id = id, .data = ct_nullptr, .size = 0}};
	// 发布事件
	for (size_t i = 0; i < TEST_DATA_NUMBER; i++) {
		msg->data = &i;
		msg->size = sizeof(size_t);
		ct_evmsg_publish(msg);
	}

	is_exit[id] = true;

	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
}
