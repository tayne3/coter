/**
 * @file timer_test.c
 * @brief 定时器测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "ctunit.h"
#include "mech/ct_thpool.h"
#include "mech/ct_timer.h"

static ct_time64_t mock_current_time = 0;

// 定时器调度, 并模拟时间流逝
static inline void timer_schedule_mock(ct_time64_t ms) {
	for (;;) {
		if (ct_timer_mgr_schedule(mock_current_time)) {
			sched_yield();
			continue;
		}
		if (ms <= 0) {
			break;
		}
		if (ms >= 10) {
			mock_current_time += 10;
			ms -= 10;
		} else {
			mock_current_time += ms;
			ms = 0;
		}
	}
}

// 重置模拟时间
static inline void reset_mock_time(void) {
	mock_current_time = 0;
	ct_timer_mgr_schedule(mock_current_time);
}

// 定时器回调函数
static inline void timer_callback(ct_timer_id_t id, void *arg) {
	size_t *count = (size_t *)arg;
	*count += 1;
	return;
	(void)(id);
}

// 测试基本功能
static inline void test_basic_functionality(void) {
	size_t count = 0;
	reset_mock_time();

	// 100毫秒定时器
	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);

	timer_schedule_mock(99);
	ctunit_assert_int32_equal(count, 0);

	timer_schedule_mock(1);
	ctunit_assert_int32_equal(count, 1);

	timer_schedule_mock(100);
	ctunit_assert_int32_equal(count, 2);

	ct_timer_stop(timer_id);
}

// 测试单次定时器
static inline void test_single_timer(void) {
	size_t count = 0;
	reset_mock_time();

	ct_timer_start(100, false, false, timer_callback, &count);
	ctunit_assert_int32_equal(count, 0);

	timer_schedule_mock(100);
	ctunit_assert_int32_equal(count, 1);

	timer_schedule_mock(200);
	ctunit_assert_int32_equal(count, 1);
}

// 测试重复定时器
static inline void test_repeating_timer(void) {
	size_t count = 0;
	reset_mock_time();

	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);

	timer_schedule_mock(250);
	ctunit_assert_int32_equal(count, 2);

	ct_timer_stop(timer_id);
}

// 测试精确定时器
static inline void test_precise_timer(void) {
	size_t count = 0;
	reset_mock_time();

	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);

	timer_schedule_mock(99);
	ctunit_assert_int32_equal(count, 0);

	timer_schedule_mock(1);
	ctunit_assert_int32_equal(count, 1);

	timer_schedule_mock(100);
	ctunit_assert_int32_equal(count, 2);

	ct_timer_stop(timer_id);
}

// 测试边界条件
static inline void test_zero_millisecond_timer(void) {
	size_t count = 0;
	reset_mock_time();

	// 0毫秒定时器
	ct_timer_id_t timer_id = ct_timer_start(0, false, false, timer_callback, &count);
	ctunit_assert_int32_equal(timer_id, CT_TIMER_ID_INVALID);

	timer_schedule_mock(10);
	ctunit_assert_int32_equal(count, 0);
}

// 测试多个定时器同时运行
static inline void test_multiple_timers(void) {
	size_t counts[3] = {0};
	reset_mock_time();

	// 3个定时器
	ct_timer_id_t timer_ids[3];
	timer_ids[0] = ct_timer_start(100, true, false, timer_callback, &counts[0]);
	timer_ids[1] = ct_timer_start(150, true, false, timer_callback, &counts[1]);
	timer_ids[2] = ct_timer_start(200, true, false, timer_callback, &counts[2]);
	timer_schedule_mock(500);

	ctunit_assert_int32_equal(counts[0], 5);
	ctunit_assert_int32_equal(counts[1], 3);
	ctunit_assert_int32_equal(counts[2], 2);

	ct_timer_stop(timer_ids[0]);
	ct_timer_stop(timer_ids[1]);
	ct_timer_stop(timer_ids[2]);
}

// 测试停止后重新启动定时器
static inline void test_restart_timer(void) {
	size_t count = 0;
	reset_mock_time();

	// 100毫秒定时器
	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);
	timer_schedule_mock(250);
	ctunit_assert_int32_equal(count, 2);

	// 停止定时器
	ct_timer_stop(timer_id);

	// 重新启动定时器
	timer_schedule_mock(100);
	ctunit_assert_int32_equal(count, 2);

	// 100毫秒定时器
	timer_id = ct_timer_start(100, true, false, timer_callback, &count);
	timer_schedule_mock(150);
	ctunit_assert_int32_equal(count, 3);
	ct_timer_stop(timer_id);
}

int main(void) {
	// 创建线程池
	ct_thpool_t *thpool = ct_thpool_create(2, NULL);
	ctunit_assert_not_null(thpool);

	// 初始化定时器中枢
	ct_timer_mgr_init(mock_current_time, thpool);

	test_basic_functionality();
	ctunit_trace("Finish! test_basic_functionality()\n");

	test_single_timer();
	ctunit_trace("Finish! test_single_timer()\n");

	test_repeating_timer();
	ctunit_trace("Finish! test_repeating_timer()\n");

	test_precise_timer();
	ctunit_trace("Finish! test_precise_timer()\n");

	test_zero_millisecond_timer();
	ctunit_trace("Finish! test_zero_millisecond_timer()\n");

	test_multiple_timers();
	ctunit_trace("Finish! test_multiple_timers()\n");

	test_restart_timer();
	ctunit_trace("Finish! test_restart_timer()\n");

	// 销毁线程池
	ct_thpool_destroy(thpool);

	ctunit_pass();
}
