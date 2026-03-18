#include "coter/time/cron.h"

#include <catch.hpp>

#include "coter/core/platform.h"
#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"

namespace {

struct test_env {
	ct_atomic_long_t realtime        = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_long_t monotonic       = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t  fired_count     = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t  manager_stopped = CT_ATOMIC_VAR_INIT(0);
	ct_thread_t      manager_thread;
	ct_cron_t        wakeup;
	bool             started = false;

	test_env() {}

	~test_env() {
		if (!started) { return; }
		ct_cron_mgr_close();
		(void)ct_thread_join(manager_thread, nullptr);
	}
};

static test_env g_env;

void do_nothing_cb(void*) {
}

void count_cb(void* arg) {
	ct_atomic_int_t* count = (ct_atomic_int_t*)arg;
	ct_atomic_int_add(count, 1);
}

int cron_thread_run(void* arg) {
	test_env* env = (test_env*)arg;
	ct_cron_mgr_run();
	ct_atomic_int_store(&env->manager_stopped, 1);
	return 0;
}

ct_time64_t mock_realtime_ms() {
	return static_cast<ct_time64_t>(ct_atomic_long_load(&g_env.realtime)) * 1000;
}

ct_time64_t mock_monotonic_ms() {
	return static_cast<ct_time64_t>(ct_atomic_long_load(&g_env.monotonic)) * 1000;
}

ct_time_t make_test_time(int year, int month, int day, int hour, int min, int sec) {
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	tm.tm_year  = year - 1900;
	tm.tm_mon   = month - 1;
	tm.tm_mday  = day;
	tm.tm_hour  = hour;
	tm.tm_min   = min;
	tm.tm_sec   = sec;
	tm.tm_isdst = -1;
	return mktime(&tm);
}

void start() {
	const ct_time_t now = make_test_time(2020, 1, 1, 0, 0, 0);
	ct_atomic_long_store(&g_env.realtime, (long)now);
	ct_atomic_long_store(&g_env.monotonic, 0);

	ct_cron_mgr_init(mock_realtime_ms, mock_monotonic_ms);

	REQUIRE(ct_thread_create(&g_env.manager_thread, nullptr, cron_thread_run, &g_env) == 0);

	ct_cron_init(&g_env.wakeup);
	REQUIRE(ct_cron_start(&g_env.wakeup, -1, -1, -1, -1, -1, do_nothing_cb, nullptr) == 0);
	g_env.started = true;
}

void stop() {
	if (!g_env.started) { return; }
	ct_cron_mgr_close();
	REQUIRE(ct_thread_join(g_env.manager_thread, nullptr) == 0);
	REQUIRE(ct_atomic_int_load(&g_env.manager_stopped) == 1);
	g_env.started = false;
}

void advance_seconds(ct_time_t seconds) {
	ct_atomic_long_add(&g_env.realtime, seconds);
	ct_atomic_long_add(&g_env.monotonic, seconds);
	REQUIRE(ct_cron_reset(&g_env.wakeup, -1, -1, -1, -1, -1) == 0);
}

void advance_seconds_skew(ct_time_t r, ct_time_t m) {
	ct_atomic_long_add(&g_env.realtime, r);
	ct_atomic_long_add(&g_env.monotonic, m);
	REQUIRE(ct_cron_reset(&g_env.wakeup, -1, -1, -1, -1, -1) == 0);
}

bool wait_until_count(ct_atomic_int_t* counter, int expected) {
	for (int i = 0; i < 100; ++i) {
		if (ct_atomic_int_load(counter) >= expected) { return true; }
		ct_msleep(2);
	}
	return ct_atomic_int_load(counter) >= expected;
}

}  // namespace

TEST_CASE("cron_minutely_basic_with_mock_time", "[cron]") {
	start();

	ct_atomic_int_t count = CT_ATOMIC_VAR_INIT(0);

	ct_cron_t cron;
	ct_cron_init(&cron);
	REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, count_cb, (void*)&count) == 0);

	advance_seconds(59);
	REQUIRE(ct_atomic_int_load(&count) == 0);

	advance_seconds(1);
	REQUIRE(wait_until_count(&count, 1));

	advance_seconds(60);
	REQUIRE(wait_until_count(&count, 2));

	REQUIRE(ct_cron_stop(&cron) == 0);
	stop();
}

TEST_CASE("cron_stop_prevents_future_runs", "[cron]") {
	start();

	ct_atomic_int_t count = CT_ATOMIC_VAR_INIT(0);

	ct_cron_t cron;
	ct_cron_init(&cron);
	REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, count_cb, (void*)&count) == 0);

	advance_seconds(60);
	REQUIRE(wait_until_count(&count, 1));

	REQUIRE(ct_cron_stop(&cron) == 0);

	advance_seconds(120);
	ct_msleep(10);
	REQUIRE(ct_atomic_int_load(&count) == 1);

	stop();
}

TEST_CASE("cron_time_jump_reschedules_without_catchup_burst", "[cron]") {
	start();

	ct_atomic_int_t count = CT_ATOMIC_VAR_INIT(0);

	ct_cron_t cron;
	ct_cron_init(&cron);
	REQUIRE(ct_cron_start(&cron, 0, -1, -1, -1, -1, count_cb, (void*)&count) == 0);

	advance_seconds(1800);
	ct_msleep(10);
	REQUIRE(ct_atomic_int_load(&count) == 0);

	advance_seconds_skew(7200, 0);
	ct_msleep(10);
	REQUIRE(ct_atomic_int_load(&count) == 0);

	advance_seconds(1800);
	REQUIRE(wait_until_count(&count, 1));

	REQUIRE(ct_cron_stop(&cron) == 0);
	stop();
}

TEST_CASE("cron_multiple_tasks_due_at_same_now_can_fire_together", "[cron]") {
	start();

	ct_atomic_int_t minute_count = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t hour_count   = CT_ATOMIC_VAR_INIT(0);

	ct_cron_t minute_cron;
	ct_cron_t hour_cron;
	ct_cron_init(&minute_cron);
	ct_cron_init(&hour_cron);

	REQUIRE(ct_cron_start(&minute_cron, -1, -1, -1, -1, -1, count_cb, (void*)&minute_count) == 0);
	REQUIRE(ct_cron_start(&hour_cron, 0, -1, -1, -1, -1, count_cb, (void*)&hour_count) == 0);

	advance_seconds(60);
	REQUIRE(wait_until_count(&minute_count, 1));
	REQUIRE(ct_atomic_int_load(&hour_count) == 0);

	advance_seconds(3540);
	REQUIRE(wait_until_count(&minute_count, 2));
	REQUIRE(wait_until_count(&hour_count, 1));

	REQUIRE(ct_cron_stop(&minute_cron) == 0);
	REQUIRE(ct_cron_stop(&hour_cron) == 0);
	stop();
}
