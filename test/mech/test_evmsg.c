/**
 * @file test_evmsg.c
 * @brief 事件消息测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include <stdbool.h>
#include <stdio.h>

#include "base/ct_platform.h"
#include "ct_app.h"
#include "ctunit.h"
#include "mech/ct_evmsg.h"
#include "mech/ct_thpool.h"
#include "sched.h"

#define TEST_THREAD_NUMBER 3
#define TEST_DATA_NUMBER   10000

static bool test_result[TEST_THREAD_NUMBER][TEST_DATA_NUMBER] = {{0}};
static bool is_exit[TEST_THREAD_NUMBER]                       = {0};

// 模拟事件处理函数
static inline bool test_evmsg_handler(ct_evmsg_msg_t *msg, void *userdata);
// 测试事件发布
static inline void *test_evmsg_publish(void *arg);

int main(void) {
	pthread_t threads[TEST_THREAD_NUMBER];

	// 创建线程池
	ct_thpool_ptr_t thpool = ct_thpool_global_create(16, 50, NULL);
	// 初始化事件消息中枢
	ct_evmsg_mgr_init();
	// 订阅事件
	ct_evmsg_subscribe(1, test_evmsg_handler, ct_nullptr);

	// 创建线程
	for (uint8_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], NULL, test_evmsg_publish, (void *)(uint64_t)i);
	}

	// 调度事件管理
	{
		size_t count = 0;
		for (size_t n = 0; n < 1000 && count < TEST_THREAD_NUMBER; n++) {
			ct_msleep(10);
			ct_evmsg_mgr_schedule();  // 事件消息中枢调度
			if (is_exit[count]) {
				count++;
			}
		}
		ct_evmsg_mgr_schedule();  // 事件消息中枢调度
		ct_evmsg_mgr_destroy();   // 销毁事件消息中枢
	}

	// 等待线程结束
	for (size_t i = 0; i < TEST_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	// 检查结果
	for (size_t id = 0; id < TEST_THREAD_NUMBER; id++) {
		for (size_t i = 0; i < TEST_DATA_NUMBER; i++) {
			ctunit_assert_true(test_result[id][i], "id = %ld, i = %ld", id, i);
		}
		sched_yield();
	}

	// 销毁线程池
	// ct_thpool_destroy(thpool);

	ctunit_pass();
}

static inline bool test_evmsg_handler(ct_evmsg_msg_t *msg, void *userdata) {
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

static inline void *test_evmsg_publish(void *arg) {
	const uint8_t id = (uint8_t)(uint64_t)arg;

	// 模拟事件数据
	ct_evmsg_msg_buf_t msg = {CT_EVMSG_MSG_INIT(1, id, ct_nullptr, 0)};

	// 发布事件
	for (size_t i = 0; i < TEST_DATA_NUMBER; i++) {
		msg->data = &i;
		msg->size = sizeof(size_t);
		ct_evmsg_publish(msg);
		sched_yield();
	}

	is_exit[id] = true;

	pthread_exit(NULL);
	return NULL;
}
