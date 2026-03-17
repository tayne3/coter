#include <catch.hpp>

#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"
#include "timer_core.h"

namespace {
struct capture {
	int    value;
	int    fired_count;
	int    order[8];
	size_t order_size;
};

static void capture_cb(void* arg) {
	capture* cap = (capture*)arg;
	if (!cap) { return; }
	cap->fired_count++;
	if (cap->order_size < CT_ARRSIZE(cap->order)) { cap->order[cap->order_size++] = cap->value; }
}

struct manager_env {
	ct_atomic_int_t stop  = CT_ATOMIC_VAR_INIT(0);
	ct_atomic_int_t fired = CT_ATOMIC_VAR_INIT(0);
};

static void manager_cb(void* arg) {
	manager_env* env = (manager_env*)arg;
	ct_atomic_int_add(&env->fired, 1);
}

static int manager_thread(void* arg) {
	manager_env* env = (manager_env*)arg;
	ct_timer_mgr_run(NULL);
	ct_atomic_int_store(&env->stop, 1);
	return 0;
}
}  // namespace

TEST_CASE("timer_core_orders_deadlines", "[timer][core]") {
	ct_timer_core_t core;
	ct_timer_core_init(&core);

	ct_timer_t a{};
	ct_timer_t b{};
	ct_timer_t c{};
	capture    ca{1, 0, {0}, 0};
	capture    cb{2, 0, {0}, 0};
	capture    cc{3, 0, {0}, 0};

	ct_timer_core_start_timer(&core, &a, 100, 30, capture_cb, &ca);
	ct_timer_core_start_timer(&core, &b, 100, 10, capture_cb, &cb);
	ct_timer_core_start_timer(&core, &c, 100, 20, capture_cb, &cc);

	REQUIRE(ct_timer_core_next_deadline(&core) == 110);

	ct_timer_node_t* first = ct_timer_core_pop_expired(&core, 109);
	REQUIRE(first == nullptr);

	first = ct_timer_core_pop_expired(&core, 110);
	REQUIRE(first == reinterpret_cast<ct_timer_node_t*>(&b));
	ct_timer_core_prepare_fire(&core, first);

	ct_timer_node_t* second = ct_timer_core_pop_expired(&core, 120);
	REQUIRE(second == reinterpret_cast<ct_timer_node_t*>(&c));
	ct_timer_core_prepare_fire(&core, second);

	ct_timer_node_t* third = ct_timer_core_pop_expired(&core, 130);
	REQUIRE(third == reinterpret_cast<ct_timer_node_t*>(&a));
	ct_timer_core_prepare_fire(&core, third);

	REQUIRE_FALSE(a.is_active);
	REQUIRE_FALSE(b.is_active);
	REQUIRE_FALSE(c.is_active);
	REQUIRE(ct_timer_core_is_empty(&core));
}

TEST_CASE("timer_core_reset_and_stop", "[timer][core]") {
	ct_timer_core_t core;
	ct_timer_core_init(&core);

	ct_timer_t timer{};
	capture    cap{7, 0, {0}, 0};

	ct_timer_core_start_timer(&core, &timer, 100, 20, capture_cb, &cap);
	REQUIRE(ct_timer_core_next_deadline(&core) == 120);

	ct_timer_core_reset_timer(&core, &timer, 105, 50);
	REQUIRE(ct_timer_core_next_deadline(&core) == 155);

	REQUIRE(ct_timer_core_pop_expired(&core, 154) == nullptr);
	REQUIRE(ct_timer_core_pop_expired(&core, 155) == reinterpret_cast<ct_timer_node_t*>(&timer));

	ct_timer_core_start_timer(&core, &timer, 200, 20, capture_cb, &cap);
	ct_timer_core_stop(&core, reinterpret_cast<ct_timer_node_t*>(&timer));
	REQUIRE_FALSE(timer.is_active);
	REQUIRE(ct_timer_core_is_empty(&core));
}

TEST_CASE("ticker_core_requeues_after_fire", "[ticker][core]") {
	ct_timer_core_t core;
	ct_timer_core_init(&core);

	ct_ticker_t ticker{};
	capture     cap{9, 0, {0}, 0};

	ct_timer_core_start_ticker(&core, &ticker, 100, 25, capture_cb, &cap);
	REQUIRE(ct_timer_core_next_deadline(&core) == 125);

	ct_timer_node_t* node = ct_timer_core_pop_expired(&core, 125);
	REQUIRE(node == reinterpret_cast<ct_timer_node_t*>(&ticker));
	ct_timer_core_prepare_fire(&core, node);
	REQUIRE(ticker.is_active);
	REQUIRE(ct_timer_core_next_deadline(&core) == 150);

	ct_timer_core_reset_ticker(&core, &ticker, 130, 40);
	REQUIRE(ct_timer_core_next_deadline(&core) == 170);

	ct_timer_core_stop(&core, reinterpret_cast<ct_timer_node_t*>(&ticker));
	REQUIRE(ct_timer_core_is_empty(&core));
}

TEST_CASE("timer_core_clear_disposes_dynamic_timeouts", "[timer][core]") {
	ct_timer_core_t core;
	ct_timer_core_init(&core);

	ct_timer_t* dynamic_timer = (ct_timer_t*)calloc(1, sizeof(ct_timer_t));
	REQUIRE(dynamic_timer != nullptr);
	capture cap{4, 0, {0}, 0};
	int     disposed = 0;

	ct_timer_core_start_timeout(&core, dynamic_timer, 0, 10, capture_cb, &cap);
	REQUIRE_FALSE(ct_timer_core_is_empty(&core));

	ct_timer_core_clear(
		&core,
		[](ct_timer_node_t* node, void* ctx) {
			int* count = (int*)ctx;
			if (node->type == CT_TIMER_NODE_TIMEOUT) {
				(*count)++;
				free(node);
			}
		},
		&disposed);

	REQUIRE(disposed == 1);
	REQUIRE(ct_timer_core_is_empty(&core));
}

TEST_CASE("timer_manager_smoke_timeout_and_ticker", "[timer][mgr]") {
	manager_env env;
	ct_thread_t tid;

	REQUIRE(ct_thread_create(&tid, nullptr, manager_thread, &env) == 0);

	ct_set_timeout(20, manager_cb, &env);

	ct_ticker_t ticker{};
	ct_ticker_start(&ticker, 10, manager_cb, &env);

	for (int i = 0; i < 50 && ct_atomic_int_load(&env.fired) < 3; ++i) { ct_msleep(10); }

	ct_ticker_stop(&ticker);
	ct_timer_mgr_close();
	REQUIRE(ct_thread_join(tid, nullptr) == 0);
	REQUIRE(ct_atomic_int_load(&env.fired) >= 2);
	REQUIRE(ct_atomic_int_load(&env.stop) == 1);
}
