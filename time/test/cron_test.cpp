#include "coter/time/cron.h"

#include <catch.hpp>

#include "coter/thread/thpool.h"
#include "coter/thread/thread.h"

static ct_time_t mock_current_time = 0;

static inline void cron_schedule_mock(ct_time_t seconds) {
	for (;;) {
		if (ct_cron_mgr_schedule(mock_current_time)) {
			ct_thread_yield();
			continue;
		}
		if (seconds <= 0) { break; }
		mock_current_time++;
		seconds--;
	}
}

static inline void reset_mock_time(void) {
	mock_current_time = 0;
	ct_cron_mgr_schedule(mock_current_time);
}

static inline void cron_callback(void *arg) {
	size_t *count = (size_t *)arg;
	(*count)++;
}

TEST_CASE("cron_basic_functionality", "[cron]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_cron_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(59);
	REQUIRE(count == 0);

	cron_schedule_mock(1);
	REQUIRE(count == 1);

	cron_schedule_mock(60);
	REQUIRE(count == 2);

	ct_cron_stop(cron_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("cron_every_minute", "[cron]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_cron_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(180);
	REQUIRE(count == 3);

	ct_cron_stop(cron_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("cron_hourly", "[cron]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_cron_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, -1, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(3600 * 3);
	REQUIRE(count == 3);

	ct_cron_stop(cron_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("cron_daily", "[cron]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_cron_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, -1, -1, -1, cron_callback, &count);

	cron_schedule_mock(86400 * 3);
	REQUIRE(count == 3);

	ct_cron_stop(cron_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("cron_weekly", "[cron]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_cron_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, -1, 0, -1, cron_callback, &count);

	cron_schedule_mock(86400 * 7 * 3);
	REQUIRE(count == 3);

	ct_cron_stop(cron_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("cron_monthly", "[cron]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_cron_mgr_init(mock_current_time, thpool);

	size_t count = 0;
	reset_mock_time();

	ct_cron_id_t cron_id = ct_cron_start(0, 0, 1, -1, -1, cron_callback, &count);

	cron_schedule_mock(86400 * 31 * 3);
	REQUIRE(count == 3);

	ct_cron_stop(cron_id);
	ct_thpool_destroy(thpool);
}

TEST_CASE("cron_multiple_crons", "[cron]") {
	ct_thpool_t *thpool = ct_thpool_create(2, nullptr);
	REQUIRE(thpool != nullptr);
	ct_cron_mgr_init(mock_current_time, thpool);

	size_t counts[3] = {0};
	reset_mock_time();

	ct_cron_id_t cron_ids[3];
	cron_ids[0] = ct_cron_start(-1, -1, -1, -1, -1, cron_callback, &counts[0]);
	cron_ids[1] = ct_cron_start(0, -1, -1, -1, -1, cron_callback, &counts[1]);
	cron_ids[2] = ct_cron_start(0, 0, -1, -1, -1, cron_callback, &counts[2]);

	cron_schedule_mock(86400 * 2);

	REQUIRE(counts[0] == 2880);
	REQUIRE(counts[1] == 48);
	REQUIRE(counts[2] == 2);

	ct_cron_stop(cron_ids[0]);
	ct_cron_stop(cron_ids[1]);
	ct_cron_stop(cron_ids[2]);
	ct_thpool_destroy(thpool);
}
