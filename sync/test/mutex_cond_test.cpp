#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"

#include <catch.hpp>

namespace {
struct cond_env {
	ct_mutex_t      mutex;
	ct_cond_t       cond;
	ct_atomic_int_t waiter_ready = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t should_exit  = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t awakened     = CT_ATOMIC_VAR_INIT(0);

	cond_env() {
		ct_mutex_init(&mutex);
		ct_cond_init(&cond);
	}

	~cond_env() {
		ct_cond_destroy(&cond);
		ct_mutex_destroy(&mutex);
	}
};

static int cond_waiter(void* arg) {
	cond_env* env = (cond_env*)arg;
	ct_mutex_lock(&env->mutex);
	ct_atomic_int_store(&env->waiter_ready, 1);
	while (!ct_atomic_int_load(&env->should_exit)) {
		ct_cond_wait(&env->cond, &env->mutex);
	}
	ct_atomic_int_add(&env->awakened, 1);
	ct_mutex_unlock(&env->mutex);
	return 0;
}

struct mutex_env {
	ct_mutex_t      mutex;
	ct_atomic_int_t entered = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t release = CT_ATOMIC_VAR_INIT(0);

	mutex_env() {
		ct_mutex_init(&mutex);
	}

	~mutex_env() {
		ct_mutex_destroy(&mutex);
	}
};

static int mutex_waiter(void* arg) {
	mutex_env* env = (mutex_env*)arg;
	ct_mutex_lock(&env->mutex);
	ct_atomic_int_store(&env->entered, 1);
	while (!ct_atomic_int_load(&env->release)) {
		ct_msleep(1);
	}
	ct_mutex_unlock(&env->mutex);
	return 0;
}
}  // namespace

TEST_CASE("mutex_blocks_other_threads", "[sync][mutex]") {
	mutex_env   env;
	ct_thread_t thread;

	REQUIRE(ct_mutex_lock(&env.mutex) == 0);
	REQUIRE(ct_thread_create(&thread, NULL, mutex_waiter, &env) == 0);

	for (int i = 0; i < 10; ++i) {
		REQUIRE(ct_atomic_int_load(&env.entered) == 0);
		ct_msleep(5);
	}

	REQUIRE(ct_mutex_unlock(&env.mutex) == 0);

	for (int i = 0; i < 20 && ct_atomic_int_load(&env.entered) == 0; ++i) {
		ct_msleep(5);
	}

	REQUIRE(ct_atomic_int_load(&env.entered) == 1);
	ct_atomic_int_store(&env.release, 1);
	REQUIRE(ct_thread_join(thread, NULL) == 0);
}

TEST_CASE("mutex_trylock_reports_busy_when_already_locked", "[sync][mutex]") {
	mutex_env env;

	REQUIRE(ct_mutex_lock(&env.mutex) == 0);
	REQUIRE(ct_mutex_trylock(&env.mutex) == EBUSY);
	REQUIRE(ct_mutex_unlock(&env.mutex) == 0);
}

TEST_CASE("cond_signal_wakes_waiter", "[sync][cond]") {
	cond_env    env;
	ct_thread_t thread;

	REQUIRE(ct_thread_create(&thread, NULL, cond_waiter, &env) == 0);

	for (int i = 0; i < 20 && ct_atomic_int_load(&env.waiter_ready) == 0; ++i) {
		ct_msleep(5);
	}

	REQUIRE(ct_atomic_int_load(&env.waiter_ready) == 1);
	REQUIRE(ct_mutex_lock(&env.mutex) == 0);
	ct_atomic_int_store(&env.should_exit, 1);
	REQUIRE(ct_cond_signal(&env.cond) == 0);
	REQUIRE(ct_mutex_unlock(&env.mutex) == 0);
	REQUIRE(ct_thread_join(thread, NULL) == 0);
	REQUIRE(ct_atomic_int_load(&env.awakened) == 1);
}

TEST_CASE("cond_timedwait_times_out", "[sync][cond]") {
	cond_env env;

	REQUIRE(ct_mutex_lock(&env.mutex) == 0);
	REQUIRE(ct_cond_timedwait(&env.cond, &env.mutex, 20) == ETIMEDOUT);
	REQUIRE(ct_mutex_unlock(&env.mutex) == 0);
}
