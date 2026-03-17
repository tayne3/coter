#include "coter/thread/thread.h"

#include <catch.hpp>

#include "coter/sync/atomic.h"
#include "coter/thread/once.h"
#include "coter/thread/tls.h"

namespace {
ct_once_t       g_once         = CT_ONCE_INIT;
ct_atomic_int_t g_once_counter = CT_ATOMIC_VAR_INIT(0);
ct_tls_key_t    g_tls_key;
ct_atomic_int_t g_tls_destructor_count = CT_ATOMIC_VAR_INIT(0);

static void thread_once_init(void) {
	ct_atomic_int_add(&g_once_counter, 1);
}

static void tls_destructor(void* value) {
	if (value) { ct_atomic_int_add(&g_tls_destructor_count, 1); }
}

static int return_worker(void* arg) {
	return (int)(intptr_t)arg;
}

static int once_worker(void* arg) {
	CT_UNUSED(arg);
	ct_once_exec(&g_once, thread_once_init);
	return 0;
}

static int tls_worker(void* arg) {
	CT_UNUSED(arg);
	if (ct_tls_set(g_tls_key, (void*)0x1) != 0) { return 1; }
	if (ct_tls_get(g_tls_key) != (void*)0x1) { return 2; }
	return 0;
}

struct detach_env {
	ct_atomic_int_t done = CT_ATOMIC_VAR_INIT(0);
};

static int detach_worker(void* arg) {
	detach_env* env = (detach_env*)arg;
	ct_msleep(20);
	ct_atomic_int_store(&env->done, 1);
	return 0;
}

}  // namespace

TEST_CASE("thread_create_and_join_returns_value", "[thread]") {
	ct_thread_t thread;
	int         result = 0;

	REQUIRE(ct_thread_create(&thread, NULL, return_worker, (void*)0x1234) == 0);
	REQUIRE(ct_thread_join(thread, &result) == 0);
	REQUIRE(result == 0x1234);
}

TEST_CASE("thread_once_runs_once", "[thread]") {
	ct_thread_t threads[4];

	ct_atomic_int_store(&g_once_counter, 0);
	for (int i = 0; i < 4; ++i) { REQUIRE(ct_thread_create(&threads[i], NULL, once_worker, NULL) == 0); }
	for (int i = 0; i < 4; ++i) { REQUIRE(ct_thread_join(threads[i], NULL) == 0); }
	REQUIRE(ct_atomic_int_load(&g_once_counter) == 1);
}

TEST_CASE("thread_tls_runs_destructor_on_exit", "[thread]") {
	ct_thread_t thread;
	int         result = -1;

	ct_atomic_int_store(&g_tls_destructor_count, 0);
	REQUIRE(ct_tls_create(&g_tls_key, tls_destructor) == 0);
	REQUIRE(ct_thread_create(&thread, NULL, tls_worker, NULL) == 0);
	REQUIRE(ct_thread_join(thread, &result) == 0);
	REQUIRE(result == 0);
	REQUIRE(ct_atomic_int_load(&g_tls_destructor_count) == 1);
	REQUIRE(ct_tls_destroy(g_tls_key) == 0);
}

TEST_CASE("thread_detach_allows_background_completion", "[thread]") {
	ct_thread_t thread;
	detach_env  env;

	REQUIRE(ct_thread_create(&thread, NULL, detach_worker, &env) == 0);
	REQUIRE(ct_thread_detach(thread) == 0);

	for (int i = 0; i < 20 && ct_atomic_int_load(&env.done) == 0; ++i) { ct_msleep(5); }

	REQUIRE(ct_atomic_int_load(&env.done) == 1);
}

TEST_CASE("thread_self_identity_helpers", "[thread]") {
	ct_thread_t self = ct_thread_self();

	REQUIRE(ct_thread_equal(self, ct_thread_self()) != 0);
	REQUIRE(ct_thread_is_self(self) != 0);
	REQUIRE(ct_thread_equal(self, self) != 0);
	REQUIRE(ct_thread_get_id(self) == ct_thread_current_id());
}
