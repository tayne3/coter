/**
 * @file test_cron.c
 * @brief cron任务相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "ctunit.h"
#include "mech/ct_cron.h"
#include "mech/ct_thpool.h"

static ct_time_t mock_current_time = 0;

// cron任务调度，并模拟时间流逝
static inline void cron_schedule_mock(ct_time_t seconds);
// 重置模拟时间
static inline void reset_mock_time(void);
// cron任务回调函数
static inline void cron_callback(ct_cron_id_t id, const ct_any_buf_t arg);

// 测试基本功能
static inline void test_basic_functionality(void);
// 测试每分钟执行的cron任务
static inline void test_every_minute_cron(void);
// 测试每小时执行的cron任务
static inline void test_hourly_cron(void);
// 测试每天执行的cron任务
static inline void test_daily_cron(void);
// 测试每周执行的cron任务
static inline void test_weekly_cron(void);
// 测试每月执行的cron任务
static inline void test_monthly_cron(void);
// 测试多个cron任务同时运行
static inline void test_multiple_crons(void);

int main(void) {
	// 创建任务池
	ct_thpool_t *thpool = ct_thpool_create(64, NULL);
	ctunit_assert_not_null(thpool);

	// 初始化cron任务管理
	ct_cron_mgr_init(mock_current_time, thpool);

	test_basic_functionality();
	ctunit_trace("Finish! test_basic_functionality()\n");

	test_every_minute_cron();
	ctunit_trace("Finish! test_every_minute_cron()\n");

	test_hourly_cron();
	ctunit_trace("Finish! test_hourly_cron()\n");

	test_daily_cron();
	ctunit_trace("Finish! test_daily_cron()\n");

	test_weekly_cron();
	ctunit_trace("Finish! test_weekly_cron()\n");

	test_monthly_cron();
	ctunit_trace("Finish! test_monthly_cron()\n");

	test_multiple_crons();
	ctunit_trace("Finish! test_multiple_crons()\n");

	// 销毁任务池
	ct_thpool_destroy(thpool);

	ctunit_pass();
}

static inline void cron_schedule_mock(ct_time_t seconds) {
	for (;;) {
		if (ct_cron_mgr_schedule(mock_current_time)) {
			sched_yield();
			continue;
		}
		if (seconds <= 0) {
			break;
		}
		mock_current_time++;
		seconds--;
	}
}

static inline void reset_mock_time(void) {
	mock_current_time = 0;
	ct_cron_mgr_schedule(mock_current_time);
}

// cron任务回调函数
static inline void cron_callback(ct_cron_id_t id, const ct_any_buf_t arg) {
	size_t *count = (size_t *)ct_any_value_pointer(*arg);
	(*count)++;
	(void)(id);
}

// 测试基本功能
static inline void test_basic_functionality(void) {
	size_t count = 0;
	reset_mock_time();

	// 每分钟执行一次的cron任务
	ct_cron_id_t cron_id = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, CT_ANY_POINTER(&count));

	cron_schedule_mock(59);
	ctunit_assert_int(count, 0, CTUnit_Equal);

	cron_schedule_mock(1);
	ctunit_assert_int(count, 1, CTUnit_Equal);

	cron_schedule_mock(60);
	ctunit_assert_int(count, 2, CTUnit_Equal);

	ct_cron_stop(cron_id);
}

// 测试每分钟执行的cron任务
static inline void test_every_minute_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, CT_ANY_POINTER(&count));

	cron_schedule_mock(180);  // 3分钟
	ctunit_assert_int(count, 3, CTUnit_Equal);

	ct_cron_stop(cron_id);
}

// 测试每小时执行的cron任务
static inline void test_hourly_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, -1, -1, -1, -1, cron_callback, CT_ANY_POINTER(&count));

	cron_schedule_mock(3600 * 3);  // 3小时
	ctunit_assert_int(count, 3, CTUnit_Equal);

	ct_cron_stop(cron_id);
}

// 测试每天执行的cron任务
static inline void test_daily_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, -1, -1, -1, cron_callback, CT_ANY_POINTER(&count));

	cron_schedule_mock(86400 * 3);  // 3天
	ctunit_assert_int(count, 3, CTUnit_Equal);

	ct_cron_stop(cron_id);
}

// 测试每周执行的cron任务
static inline void test_weekly_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, -1, 0, -1, cron_callback, CT_ANY_POINTER(&count));

	cron_schedule_mock(86400 * 7 * 3);  // 3周
	ctunit_assert_int(count, 3, CTUnit_Equal);

	ct_cron_stop(cron_id);
}

// 测试每月执行的cron任务
static inline void test_monthly_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, 1, -1, -1, cron_callback, CT_ANY_POINTER(&count));

	cron_schedule_mock(86400 * 31 * 3);  // 假设3个月
	ctunit_assert_int(count, 3, CTUnit_Equal);

	ct_cron_stop(cron_id);
}

// 测试多个cron任务同时运行
static inline void test_multiple_crons(void) {
	size_t counts[3] = {0};
	reset_mock_time();

	// 3个cron任务
	ct_cron_id_t cron_ids[3];
	cron_ids[0] = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, CT_ANY_POINTER(&counts[0]));  // 每分钟
	cron_ids[1] = ct_cron_start(0, -1, -1, -1, -1, cron_callback, CT_ANY_POINTER(&counts[1]));   // 每小时
	cron_ids[2] = ct_cron_start(0, 0, -1, -1, -1, cron_callback, CT_ANY_POINTER(&counts[2]));    // 每天

	cron_schedule_mock(86400 * 2);  // 2天

	ctunit_assert_int(counts[0], 2880, CTUnit_Equal);  // 2天 * 24小时 * 60分钟
	ctunit_assert_int(counts[1], 48, CTUnit_Equal);    // 2天 * 24小时
	ctunit_assert_int(counts[2], 2, CTUnit_Equal);     // 2天

	ct_cron_stop(cron_ids[0]);
	ct_cron_stop(cron_ids[1]);
	ct_cron_stop(cron_ids[2]);
}
