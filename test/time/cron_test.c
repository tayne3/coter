/**
 * @file cron_test.c
 * @brief cron任务相关测试
 */
#include "coter/time/cron.h"

#include "coter/thread/thpool.h"
#include "cunit.h"

static ct_time_t mock_current_time = 0;

// cron任务调度，并模拟时间流逝
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

// 重置模拟时间
static inline void reset_mock_time(void) {
	mock_current_time = 0;
	ct_cron_mgr_schedule(mock_current_time);
}

// cron任务回调函数
static inline void cron_callback(void *arg) {
	size_t *count = (size_t *)arg;
	(*count)++;
}

// 测试基本功能
static inline void test_basic_functionality(void) {
	size_t count = 0;
	reset_mock_time();

	// 每分钟执行一次的cron任务
	ct_cron_id_t cron_id = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(59);
	assert_int32_eq(count, 0);

	cron_schedule_mock(1);
	assert_int32_eq(count, 1);

	cron_schedule_mock(60);
	assert_int32_eq(count, 2);

	ct_cron_stop(cron_id);
}

// 测试每分钟执行的cron任务
static inline void test_every_minute_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(180);  // 3分钟
	assert_int32_eq(count, 3);

	ct_cron_stop(cron_id);
}

// 测试每小时执行的cron任务
static inline void test_hourly_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, -1, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(3600 * 3);  // 3小时
	assert_int32_eq(count, 3);

	ct_cron_stop(cron_id);
}

// 测试每天执行的cron任务
static inline void test_daily_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(86400 * 3);  // 3天
	assert_int32_eq(count, 3);

	ct_cron_stop(cron_id);
}

// 测试每周执行的cron任务
static inline void test_weekly_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, -1, 0, -1, cron_callback, &count);

	cron_schedule_mock(86400 * 7 * 3);  // 3周
	assert_int32_eq(count, 3);

	ct_cron_stop(cron_id);
}

// 测试每月执行的cron任务
static inline void test_monthly_cron(void) {
	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, 1, -1, -1, cron_callback, &count);

	cron_schedule_mock(86400 * 31 * 3);  // 假设3个月
	assert_int32_eq(count, 3);

	ct_cron_stop(cron_id);
}

// 测试多个cron任务同时运行
static inline void test_multiple_crons(void) {
	size_t counts[3] = {0};
	reset_mock_time();

	// 3个cron任务
	ct_cron_id_t cron_ids[3];
	cron_ids[0] = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, &counts[0]);  // 每分钟
	cron_ids[1] = ct_cron_start(0, -1, -1, -1, -1, cron_callback, &counts[1]);   // 每小时
	cron_ids[2] = ct_cron_start(0, 0, -1, -1, -1, cron_callback, &counts[2]);    // 每天

	cron_schedule_mock(86400 * 2);  // 2天

	assert_int32_eq(counts[0], 2880);  // 2天 * 24小时 * 60分钟
	assert_int32_eq(counts[1], 48);    // 2天 * 24小时
	assert_int32_eq(counts[2], 2);     // 2天

	ct_cron_stop(cron_ids[0]);
	ct_cron_stop(cron_ids[1]);
	ct_cron_stop(cron_ids[2]);
}

int main(void) {
	// 创建线程池
	ct_thpool_t *thpool = ct_thpool_create(2, NULL);
	assert_not_null(thpool);

	// 初始化cron任务管理
	ct_cron_mgr_init(mock_current_time, thpool);

	cunit_init();

	CUNIT_SUITE_BEGIN("cron", NULL, NULL)
	CUNIT_TEST("basic_functionality", test_basic_functionality)
	CUNIT_TEST("every_minute_cron", test_every_minute_cron)
	CUNIT_TEST("hourly_cron", test_hourly_cron)
	CUNIT_TEST("daily_cron", test_daily_cron)
	CUNIT_TEST("weekly_cron", test_weekly_cron)
	CUNIT_TEST("monthly_cron", test_monthly_cron)
	CUNIT_TEST("multiple_crons", test_multiple_crons)
	CUNIT_SUITE_END()

	const int ret = cunit_run();

	// 销毁线程池
	ct_thpool_destroy(thpool);

	return ret;
}
