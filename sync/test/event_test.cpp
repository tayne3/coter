#include "coter/sync/event.h"

#include <catch.hpp>

#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"

namespace {
struct event_env {
	ct_event_t      event;
	ct_atomic_int_t waiter_done = CT_ATOMIC_VAR_INIT(0);

	event_env() { ct_event_init(&event); }

	~event_env() { ct_event_destroy(&event); }
};

struct timed_wait_env {
	ct_event_t      event;
	ct_atomic_int_t ready_count   = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t success_count = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t timeout_count = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t error_count   = CT_ATOMIC_VAR_INIT(0);
	uint32_t        timeout_ms    = 0;

	timed_wait_env() { ct_event_init(&event); }

	~timed_wait_env() { ct_event_destroy(&event); }
};

static int timed_wait_worker(void* arg) {
	timed_wait_env* env = (timed_wait_env*)arg;
	ct_atomic_int_add(&env->ready_count, 1);

	int result = ct_event_timedwait(&env->event, env->timeout_ms);
	if (result == 0) {
		ct_atomic_int_add(&env->success_count, 1);
	} else if (result == ETIMEDOUT) {
		ct_atomic_int_add(&env->timeout_count, 1);
	} else {
		ct_atomic_int_add(&env->error_count, 1);
	}
	return result;
}
}  // namespace

TEST_CASE("event_signal_then_wait_consumes_signal", "[sync][event]") {
	event_env env;

	REQUIRE(ct_event_signal(&env.event) == 0);
	REQUIRE(ct_event_wait(&env.event) == 0);
	REQUIRE(ct_event_timedwait(&env.event, 20) == ETIMEDOUT);
}

TEST_CASE("event_multiple_signals_do_not_accumulate", "[sync][event]") {
	event_env env;

	REQUIRE(ct_event_signal(&env.event) == 0);
	REQUIRE(ct_event_signal(&env.event) == 0);
	REQUIRE(ct_event_wait(&env.event) == 0);
	REQUIRE(ct_event_timedwait(&env.event, 20) == ETIMEDOUT);
}

TEST_CASE("event_wait_blocks_until_signal", "[sync][event]") {
	event_env   env;
	ct_thread_t thread;

	auto wait_worker = [](void* arg) -> int {
		event_env* env = (event_env*)arg;
		if (ct_event_wait(&env->event) == 0) { ct_atomic_int_store(&env->waiter_done, 1); }
		return 0;
	};
	REQUIRE(ct_thread_create(&thread, NULL, wait_worker, &env) == 0);

	for (int i = 0; i < 10; ++i) {
		REQUIRE(ct_atomic_int_load(&env.waiter_done) == 0);
		ct_msleep(5);
	}

	REQUIRE(ct_event_signal(&env.event) == 0);
	REQUIRE(ct_thread_join(thread, NULL) == 0);
	REQUIRE(ct_atomic_int_load(&env.waiter_done) == 1);
}

TEST_CASE("event_timedwait_times_out_without_signal", "[sync][event]") {
	event_env env;

	REQUIRE(ct_event_timedwait(&env.event, 20) == ETIMEDOUT);
}

TEST_CASE("event_signal_wakes_only_one_waiter", "[sync][event]") {
	timed_wait_env env;
	ct_thread_t    threads[2];

	env.timeout_ms = 200;

	for (int i = 0; i < 2; ++i) { REQUIRE(ct_thread_create(&threads[i], NULL, timed_wait_worker, &env) == 0); }

	for (int i = 0; i < 40 && ct_atomic_int_load(&env.ready_count) != 2; ++i) { ct_msleep(5); }

	REQUIRE(ct_atomic_int_load(&env.ready_count) == 2);
	REQUIRE(ct_event_signal(&env.event) == 0);

	for (int i = 0; i < 2; ++i) { REQUIRE(ct_thread_join(threads[i], NULL) == 0); }

	REQUIRE(ct_atomic_int_load(&env.success_count) == 1);
	REQUIRE(ct_atomic_int_load(&env.timeout_count) == 1);
	REQUIRE(ct_atomic_int_load(&env.error_count) == 0);
}

TEST_CASE("event_two_signals_wake_two_waiters_when_signaled_separately", "[sync][event]") {
	timed_wait_env env;
	ct_thread_t    threads[2];

	env.timeout_ms = 500;

	for (int i = 0; i < 2; ++i) { REQUIRE(ct_thread_create(&threads[i], NULL, timed_wait_worker, &env) == 0); }

	for (int i = 0; i < 40 && ct_atomic_int_load(&env.ready_count) != 2; ++i) { ct_msleep(5); }

	REQUIRE(ct_atomic_int_load(&env.ready_count) == 2);
	REQUIRE(ct_event_signal(&env.event) == 0);
	for (int i = 0; i < 40 && ct_atomic_int_load(&env.success_count) != 1; ++i) { ct_msleep(5); }
	REQUIRE(ct_atomic_int_load(&env.success_count) == 1);
	REQUIRE(ct_event_signal(&env.event) == 0);

	for (int i = 0; i < 2; ++i) { REQUIRE(ct_thread_join(threads[i], NULL) == 0); }

	REQUIRE(ct_atomic_int_load(&env.success_count) == 2);
	REQUIRE(ct_atomic_int_load(&env.timeout_count) == 0);
	REQUIRE(ct_atomic_int_load(&env.error_count) == 0);
}

TEST_CASE("event_reset_clears_pending_signal", "[sync][event]") {
	event_env env;

	REQUIRE(ct_event_signal(&env.event) == 0);
	REQUIRE(ct_event_reset(&env.event) == 0);
	REQUIRE(ct_event_timedwait(&env.event, 20) == ETIMEDOUT);
}
