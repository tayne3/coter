#include "coter/time/timer.h"

#include <catch.hpp>

#include "coter/thread/thpool.h"

static ct_time64_t mock_current_time = 0;

static inline void timer_schedule_mock(ct_time64_t ms) {
	for (;;) {
		if (ct_timer_mgr_schedule(mock_current_time)) {
			CT_PAUSE();
			continue;
		}
		if (ms <= 0) { break; }
		if (ms >= 10) {
			mock_current_time += 10;
			ms -= 10;
		} else {
			mock_current_time += ms;
			ms = 0;
		}
	}
}

static inline void reset_mock_time(void) {
	mock_current_time = 0;
	ct_timer_mgr_schedule(mock_current_time);
}

static inline void timer_callback(void *arg) {
	size_t *count = (size_t *)arg;
	*count += 1;
}

TEST_CASE("timer_basic_functionality", "[timer]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_timer_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);

	timer_schedule_mock(99);
	REQUIRE(count == 0);

	timer_schedule_mock(1);
	REQUIRE(count == 1);

	timer_schedule_mock(100);
	REQUIRE(count == 2);

	ct_timer_stop(timer_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("timer_single", "[timer]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_timer_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_timer_start(100, false, false, timer_callback, &count);
	REQUIRE(count == 0);

	timer_schedule_mock(100);
	REQUIRE(count == 1);

	timer_schedule_mock(200);
	REQUIRE(count == 1);

	ct_thpool_destroy(thpool);
}

TEST_CASE("timer_repeating", "[timer]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_timer_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);

	timer_schedule_mock(250);
	REQUIRE(count == 2);

	ct_timer_stop(timer_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("timer_precise", "[timer]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_timer_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);

	timer_schedule_mock(99);
	REQUIRE(count == 0);

	timer_schedule_mock(1);
	REQUIRE(count == 1);

	timer_schedule_mock(100);
	REQUIRE(count == 2);

	ct_timer_stop(timer_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("timer_zero_millisecond", "[timer]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_timer_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_timer_id_t timer_id = ct_timer_start(0, false, false, timer_callback, &count);
	REQUIRE(timer_id == CT_TIMER_ID_INVALID);

	timer_schedule_mock(10);
	REQUIRE(count == 0);

	ct_thpool_destroy(thpool);
}

TEST_CASE("timer_multiple_timers", "[timer]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_timer_mgr_init(mock_current_time, thpool);

	size_t counts[3] = {0};
	reset_mock_time();

	ct_timer_id_t timer_ids[3];
	timer_ids[0] = ct_timer_start(100, true, false, timer_callback, &counts[0]);
	timer_ids[1] = ct_timer_start(150, true, false, timer_callback, &counts[1]);
	timer_ids[2] = ct_timer_start(200, true, false, timer_callback, &counts[2]);
	timer_schedule_mock(500);

	REQUIRE(counts[0] == 5);
	REQUIRE(counts[1] == 3);
	REQUIRE(counts[2] == 2);

	ct_timer_stop(timer_ids[0]);
	ct_timer_stop(timer_ids[1]);
	ct_timer_stop(timer_ids[2]);
	ct_thpool_destroy(thpool);
}

TEST_CASE("timer_restart", "[timer]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_timer_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_timer_id_t timer_id = ct_timer_start(100, true, false, timer_callback, &count);
	timer_schedule_mock(250);
	REQUIRE(count == 2);

	ct_timer_stop(timer_id);

	timer_schedule_mock(100);
	REQUIRE(count == 2);

	timer_id = ct_timer_start(100, true, false, timer_callback, &count);
	timer_schedule_mock(150);
	REQUIRE(count == 3);
	ct_timer_stop(timer_id);

	ct_thpool_destroy(thpool);
}
