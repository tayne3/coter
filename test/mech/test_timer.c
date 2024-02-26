/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include <stdbool.h>
#include <stdio.h>

#include "ct_app.h"
#include "ctunit.h"
#include "mech/ct_timer.h"
#include "sys/ct_thread.h"

static bool test_isexit = false;

// 定时器执行
static inline void *test_timer_exec(void *arg);
// 定时器回调函数
static inline void timer_callback(ct_timer_id_t id, const ct_any_buf_t arg);
// 测试定时器
static inline void test_timer_1(void);

int main(void)
{
	ct_app_create();
	// 创建线程
	ct_thread_buf_t threads[1];
	ct_thread_create(threads[0], test_timer_exec, ct_nullptr);

	test_timer_1();

	// 等待线程结束
	test_isexit = true;
	ct_thread_join(threads[0], ct_nullptr);
	ctunit_pass();
}

static inline void *test_timer_exec(void *arg)
{
	for (; !test_isexit;) {
		ct_timer_center_schedule();
		ct_thread_msleep(5);
	}
	ct_thread_exit(ct_nullptr);
	return arg;
}

static inline void timer_callback(ct_timer_id_t id, const ct_any_buf_t arg)
{
	*(size_t *)ct_any_value_pointer(*arg) += 1;
	return;
	ct_unused(id);
}

static inline void test_timer_1(void)
{
	size_t        onoff_count[1]     = {0};
	size_t        periodic_count[2]  = {0};
	size_t        schedule_count[1]  = {0};
	size_t        precision_count[3] = {0};
	ct_timer_id_t timer_ids[5]       = {0};

	// 创建一次性定时器
	{
		ct_timer_start_oneoff(1, timer_callback, CT_ANY_POINTER(&onoff_count[0]));
	}
	// 创建周期性定时器
	{
		timer_ids[0] = ct_timer_start_periodic(1, true, timer_callback, CT_ANY_POINTER(&periodic_count[0]));
		timer_ids[1] = ct_timer_start_periodic(1, false, timer_callback, CT_ANY_POINTER(&periodic_count[1]));
	}
	// 创建精确定时器
	{
		timer_ids[2] =
			ct_timer_start_precision(1000, true, true, timer_callback, CT_ANY_POINTER(&precision_count[0]));
		timer_ids[3] =
			ct_timer_start_precision(300, false, true, timer_callback, CT_ANY_POINTER(&precision_count[1]));
		timer_ids[4] =
			ct_timer_start_precision(100, false, true, timer_callback, CT_ANY_POINTER(&precision_count[2]));
	}
	// 创建定时器 (指定日期时间)
	{
		const ct_datetime_t current_time = ct_current_datetime();
		const ct_datetime_t target_time  = ct_timestamp_to_datetime(ct_datetime_to_timestamp(&current_time) + 1);
		ct_timer_start_schedule(target_time, timer_callback, CT_ANY_POINTER(&schedule_count[0]));
	}

	ct_thread_msleep(1100);
	ct_timer_stop(timer_ids[0]);
	ct_timer_stop(timer_ids[1]);
	ct_timer_stop(timer_ids[2]);
	ct_timer_stop(timer_ids[3]);
	ct_timer_stop(timer_ids[4]);

	ctunit_assert_int(onoff_count[0], 1, CTUnit_Equal);
	ctunit_assert_int(periodic_count[0], 2, CTUnit_Equal);
	ctunit_assert_int(periodic_count[1], 1, CTUnit_Equal);
	ctunit_assert_int(precision_count[0], 2, CTUnit_Equal);
	ctunit_assert_int(precision_count[1], 3, CTUnit_Equal);
	ctunit_assert_int(precision_count[2], 10, CTUnit_Equal);
	ctunit_assert_int(schedule_count[0], 1, CTUnit_Equal);
}
