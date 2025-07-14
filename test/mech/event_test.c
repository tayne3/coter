/**
 * @file event_test.c
 * @brief 事件测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "base/ct_platform.h"
#include "cunit.h"
#include "mech/ct_event.h"

#define TEST_THREAD_COUNT 1
#define TEST_EVENT_COUNT  1000

static int        test_result[TEST_THREAD_COUNT] = {0};
static ct_event_t event_a                        = CT_EVENT_INITIALIZER;
static ct_event_t event_b                        = CT_EVENT_INITIALIZER;

// 辅助函数:发送事件
static inline void* send_events(void* arg) {
	const ct_event_id_t id_send = (ct_event_id_t)(uintptr_t)arg + 1;
	assert_true(ct_event_id_isvalid(id_send));

	ct_event_id_t id_recv;
	ct_any_t      arg_recv;

	for (int i = 0; i < TEST_EVENT_COUNT; i++) {
		ct_event_send(&event_b, id_send, CT_ANY_INT(i));

		id_recv = ct_event_receive_single(&event_a, id_send);
		assert_uint8_eq(id_send, id_recv);

		arg_recv = ct_event_arg_get(&event_b, id_recv);
		assert_int32_eq(ct_any_value_int(arg_recv), i);
	}

	return NULL;
}

// 测试基本功能
static inline void test_basic_functionality(void) {
	ct_event_t event = CT_EVENT_INITIALIZER;

	// 测试发送和接收单个事件
	ct_event_send(&event, CT_EVENT_ID_MIN, CT_ANY_INT(42));
	ct_event_id_t received_id = ct_event_receive(&event);
	assert_uint8_eq(received_id, CT_EVENT_ID_MIN);
	ct_any_t arg = ct_event_arg_get(&event, CT_EVENT_ID_MIN);
	assert_int32_eq(ct_any_value_int(arg), 42);

	// 测试清除所有事件
	ct_event_send(&event, CT_EVENT_ID_MIN + 1, CT_ANY_INT(100));
	ct_event_clear(&event);
	received_id = ct_event_try_receive(&event);
	assert_uint8_eq(received_id, CT_EVENT_ID_INVALID);
}

// 测试边界条件
static inline void test_boundary_conditions(void) {
	ct_event_t event = CT_EVENT_INITIALIZER;

	// 测试最小和最大有效事件ID
	{
		ct_event_send(&event, CT_EVENT_ID_MIN, ct_any_null);
		ct_event_send(&event, CT_EVENT_ID_MAX - 1, ct_any_null);

		ct_event_id_t id1 = ct_event_receive(&event);
		assert_true(ct_event_id_isvalid(id1));
		assert_uint8_eq(id1, CT_EVENT_ID_MIN);

		ct_event_id_t id2 = ct_event_receive(&event);
		assert_true(ct_event_id_isvalid(id2));
		assert_uint8_eq(id2, CT_EVENT_ID_MAX - 1);
	}
	{
		ct_event_send(&event, CT_EVENT_ID_MAX - 1, ct_any_null);
		ct_event_send(&event, CT_EVENT_ID_MIN, ct_any_null);

		ct_event_id_t id1 = ct_event_receive(&event);
		ct_event_id_t id2 = ct_event_receive(&event);
		assert_true(ct_event_id_isvalid(id1));
		assert_true(ct_event_id_isvalid(id2));

		assert_uint8_eq(id1, CT_EVENT_ID_MIN);
		assert_uint8_eq(id2, CT_EVENT_ID_MAX - 1);
	}
	{
		ct_event_send(&event, CT_EVENT_ID_MIN, ct_any_null);
		ct_event_send(&event, CT_EVENT_ID_MAX - 1, ct_any_null);

		ct_event_id_t id1 = ct_event_receive(&event);
		ct_event_id_t id2 = ct_event_receive(&event);
		assert_true(ct_event_id_isvalid(id1));
		assert_true(ct_event_id_isvalid(id2));

		assert_uint8_eq(id1, CT_EVENT_ID_MIN);
		assert_uint8_eq(id2, CT_EVENT_ID_MAX - 1);
	}

	// 测试无效的事件ID
	{
		ct_event_send(&event, CT_EVENT_ID_INVALID, ct_any_null);
		ct_event_send(&event, CT_EVENT_ID_MAX, ct_any_null);

		ct_event_id_t id1 = ct_event_try_receive(&event);
		assert_false(ct_event_id_isvalid(id1));
	}
}

// 测试并发
static inline void test_concurrency(void) {
	pthread_t threads[TEST_THREAD_COUNT];

	bool is_ok;

	// 创建线程
	for (int i = 0; i < TEST_THREAD_COUNT; i++) {
		is_ok = pthread_create(&threads[i], NULL, send_events, (void*)(uintptr_t)i) == 0;
		assert_true(is_ok, "id = %d/%d", i, TEST_THREAD_COUNT);
	}

	ct_event_id_t id;
	ct_any_t      arg;

	for (int thread_counter = 0;;) {
		id = ct_event_receive(&event_b);
		assert_true(ct_event_id_isvalid(id));

		arg = ct_event_arg_get(&event_b, id);
		assert_int32_eq(ct_any_value_int(arg), test_result[id - 1]);
		test_result[id - 1]++;

		ct_event_send(&event_a, id, arg);

		if (test_result[id - 1] >= TEST_EVENT_COUNT) {
			if (++thread_counter == TEST_THREAD_COUNT) {
				break;
			}
		}
	}

	// 等待线程结束
	for (int i = 0; i < TEST_THREAD_COUNT; i++) {
		pthread_join(threads[i], NULL);
	}
}

// 测试非阻塞行为
static inline void test_non_blocking(void) {
	ct_event_t event = CT_EVENT_INITIALIZER;

	ct_event_id_t id = ct_event_try_receive(&event);
	assert_uint8_eq(id, CT_EVENT_ID_INVALID);

	ct_event_send(&event, CT_EVENT_ID_MIN, ct_any_null);
	id = ct_event_try_receive(&event);
	assert_uint8_eq(id, CT_EVENT_ID_MIN);
}

int main(void) {
	test_basic_functionality();
	cunit_println("Finish! test_basic_functionality();");

	test_boundary_conditions();
	cunit_println("Finish! test_boundary_conditions();");

	test_concurrency();
	cunit_println("Finish! test_concurrency();");

	test_non_blocking();
	cunit_println("Finish! test_non_blocking();");

	cunit_pass();
}
