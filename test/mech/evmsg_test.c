/**
 * @file evmsg_test.c
 * @brief 事件消息测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "base/ct_platform.h"
#include "ctunit.h"
#include "mech/ct_evmsg.h"
#include "mech/ct_thpool.h"

#define TEST_THREAD_NUMBER 3
#define TEST_DATA_NUMBER   10000

static ct_evmsg_center_ptr_t test_center                                       = NULL;
static bool                  test_result[TEST_THREAD_NUMBER][TEST_DATA_NUMBER] = {{0}};
static bool                  is_exit[TEST_THREAD_NUMBER]                       = {0};

static inline bool test_evmsg_handler(ct_evmsg_t *msg, void *userdata) {
	ctunit_assert_not_null(msg);
	ctunit_assert_not_null(msg->data);
	ctunit_assert_uint8(msg->id, 0, CTUnit_GreaterEqual);
	ctunit_assert_uint8(msg->id, TEST_THREAD_NUMBER, CTUnit_Less);

	const int result_index = *(int *)msg->data;
	ctunit_assert_false(test_result[msg->id][result_index]);
	test_result[msg->id][result_index] = true;
	return false;
	(void)(userdata);
}

static inline void *test_evmsg_publish(void *arg) {
	const uint8_t id = (uint8_t)(uintptr_t)arg;
	// 模拟事件数据
	ct_evmsg_t msg = CT_EVMSG_MSG_INIT(1, id, NULL, 0);
	// 发布事件
	for (int i = 0; i < TEST_DATA_NUMBER; i++) {
		msg.data = &i;
		msg.size = sizeof(int);
		ct_evmsg_publish(test_center, &msg);
		sched_yield();
	}

	is_exit[id] = true;
	return NULL;
}

int main(void) {
	bool      is_ok;
	pthread_t threads[TEST_THREAD_NUMBER];

	// 创建任务池
	ct_thpool_t *thpool = ct_thpool_create(64, NULL);
	ctunit_assert_not_null(thpool);

	// 初始化事件消息中枢
	test_center = ct_evmsg_center_create(thpool);
	// 订阅事件
	ct_evmsg_subscribe(test_center, 1, test_evmsg_handler, NULL);

	// 创建线程
	for (int i = 0; i < TEST_THREAD_NUMBER; i++) {
		is_ok = pthread_create(&threads[i], NULL, test_evmsg_publish, (void *)(uintptr_t)i) == 0;
		ctunit_assert_true(is_ok, "id = %d/%d", i, TEST_THREAD_NUMBER);
	}

	// 调度事件管理
	for (int count = 0, n = 0; n < 1000; n++) {
		ct_msleep(10);
		ct_evmsg_center_schedule(test_center);  // 事件消息调度
		if (is_exit[count]) {
			if (++count == TEST_THREAD_NUMBER) {
				break;
			}
		}
	}

	ct_evmsg_center_schedule(test_center);  // 事件消息中枢调度
	ct_evmsg_center_destroy(test_center);   // 销毁事件消息中枢
	test_center = NULL;

	// 等待线程结束
	for (int i = 0; i < TEST_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	// 检查结果
	for (int id = 0; id < TEST_THREAD_NUMBER; id++) {
		for (int i = 0; i < TEST_DATA_NUMBER; i++) {
			ctunit_assert_true(test_result[id][i], "id = %ld, i = %ld", id, i);
		}
		sched_yield();
	}

	// 销毁任务池
	ct_thpool_destroy(thpool);

	ctunit_pass();
}
