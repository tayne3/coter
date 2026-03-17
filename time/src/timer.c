/**
 * @file timer.c
 * @brief 定时器机制
 */
#include "coter/time/timer.h"

#include <stdint.h>
#include <stdlib.h>

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/thread/once.h"
#include "timer_core.h"
#include "timer_internal.h"

/**
 * @struct ct_timer_manager
 * @brief 全局任务调度堆与保护锁
 */
static struct ct_timer_manager {
	ct_time64_t (*gettime_cb)(void);  // 时间获取回调

	ct_timer_core_t core;
	ct_mutex_t      lock;  // 全局锁
	ct_cond_t       cv;    // 调度器等待条件变量

	ct_atomic_bool_t shutdown;
	ct_atomic_bool_t running;

	ct_time64_t recent_time;
} g_mgr = {
	.gettime_cb = ct_getuptime_ms,

	.shutdown = CT_ATOMIC_VAR_INIT(0),
	.running  = CT_ATOMIC_VAR_INIT(0),

	.recent_time = 0,
};

static ct_once_t g_mgr_once = CT_ONCE_INIT;

static void timer__dispose_node(ct_timer_node_t *node, void *ctx);
static void timer__init_runtime(void);
static void timer__ensure_runtime(void);

static void timer__init_runtime(void) {
	ct_timer_core_init(&g_mgr.core);
	ct_mutex_init(&g_mgr.lock);
	ct_cond_init(&g_mgr.cv);
}

static void timer__ensure_runtime(void) {
	ct_once_exec(&g_mgr_once, timer__init_runtime);
}

void ct_timer_mgr_run(ct_time64_t (*gettime_cb)(void)) {
	timer__ensure_runtime();
	if (ct_atomic_bool_load(&g_mgr.running)) { return; }
	ct_atomic_bool_store(&g_mgr.running, true);
	ct_atomic_bool_store(&g_mgr.shutdown, false);

	g_mgr.gettime_cb  = gettime_cb ? gettime_cb : ct_getuptime_ms;
	g_mgr.recent_time = 0;

	ct_mutex_lock(&g_mgr.lock);
	while (!ct_atomic_bool_load(&g_mgr.shutdown)) {
		ct_time64_t now = g_mgr.gettime_cb();
		if (ct_timer_core_is_empty(&g_mgr.core)) {
			g_mgr.recent_time = 0;
			ct_cond_wait(&g_mgr.cv, &g_mgr.lock);
			continue;
		}

		ct_time64_t deadline = ct_timer_core_next_deadline(&g_mgr.core);
		if (now < deadline) {
			const ct_time64_t wait_ms    = deadline - now;
			const uint32_t    timeout_ms = wait_ms > UINT32_MAX ? UINT32_MAX : (uint32_t)wait_ms;
			g_mgr.recent_time            = deadline;
			ct_cond_timedwait(&g_mgr.cv, &g_mgr.lock, timeout_ms);
			continue;
		}
		ct_timer_node_t *node = ct_timer_core_pop_expired(&g_mgr.core, now);
		if (!node) { continue; }
		ct_timer_callback_t cb  = node->cb;
		void               *arg = node->arg;
		ct_timer_core_prepare_fire(&g_mgr.core, node);

		if (node->type == CT_TIMER_NODE_TIMEOUT) {
			ct_mutex_unlock(&g_mgr.lock);
			free(node);
			if (cb) { cb(arg); }
			ct_mutex_lock(&g_mgr.lock);
			continue;
		}

		ct_mutex_unlock(&g_mgr.lock);
		if (cb) { cb(arg); }
		ct_mutex_lock(&g_mgr.lock);
	}

	ct_timer_core_clear(&g_mgr.core, timer__dispose_node, NULL);
	ct_mutex_unlock(&g_mgr.lock);
	ct_atomic_bool_store(&g_mgr.running, false);
}

void ct_timer_mgr_close(void) {
	timer__ensure_runtime();
	if (ct_atomic_bool_load(&g_mgr.shutdown)) { return; }
	ct_atomic_bool_store(&g_mgr.shutdown, true);

	ct_mutex_lock(&g_mgr.lock);
	ct_cond_signal(&g_mgr.cv);
	ct_mutex_unlock(&g_mgr.lock);
}

void ct_timer_start(ct_timer_t *timer, ct_time64_t timeout_ms, void (*cb)(void *), void *arg) {
	if (!timer || !cb || ct_atomic_bool_load(&g_mgr.shutdown)) { return; }

	timer__ensure_runtime();
	ct_mutex_lock(&g_mgr.lock);
	ct_timer_core_start_timer(&g_mgr.core, timer, g_mgr.gettime_cb(), timeout_ms, cb, arg);
	if (g_mgr.recent_time > timer->trigger_time || g_mgr.recent_time == 0) { ct_cond_signal(&g_mgr.cv); }
	ct_mutex_unlock(&g_mgr.lock);
}

void ct_timer_reset(ct_timer_t *timer, ct_time64_t timeout_ms) {
	if (!timer || ct_atomic_bool_load(&g_mgr.shutdown)) { return; }

	timer__ensure_runtime();
	ct_mutex_lock(&g_mgr.lock);
	ct_timer_core_reset_timer(&g_mgr.core, timer, g_mgr.gettime_cb(), timeout_ms);
	if (g_mgr.recent_time > timer->trigger_time || g_mgr.recent_time == 0) { ct_cond_signal(&g_mgr.cv); }
	ct_mutex_unlock(&g_mgr.lock);
}

void ct_timer_stop(ct_timer_t *timer) {
	if (!timer) { return; }
	timer__ensure_runtime();
	ct_mutex_lock(&g_mgr.lock);
	ct_timer_core_stop(&g_mgr.core, (ct_timer_node_t *)timer);
	ct_mutex_unlock(&g_mgr.lock);
}

int ct_set_timeout(ct_time64_t timeout_ms, void (*cb)(void *), void *arg) {
	if (!cb || ct_atomic_bool_load(&g_mgr.shutdown)) { return -1; }

	timer__ensure_runtime();
	ct_timer_t *timer = (ct_timer_t *)malloc(sizeof(ct_timer_t));
	if (!timer) { return -1; }

	ct_mutex_lock(&g_mgr.lock);
	ct_timer_core_start_timeout(&g_mgr.core, timer, g_mgr.gettime_cb(), timeout_ms, cb, arg);
	if (g_mgr.recent_time > timer->trigger_time || g_mgr.recent_time == 0) { ct_cond_signal(&g_mgr.cv); }
	ct_mutex_unlock(&g_mgr.lock);
	return 0;
}

void ct_ticker_start(ct_ticker_t *ticker, ct_time64_t interval_ms, void (*cb)(void *), void *arg) {
	if (!ticker || !cb || ct_atomic_bool_load(&g_mgr.shutdown)) { return; }

	timer__ensure_runtime();
	ct_mutex_lock(&g_mgr.lock);
	ct_timer_core_start_ticker(&g_mgr.core, ticker, g_mgr.gettime_cb(), interval_ms, cb, arg);
	if (g_mgr.recent_time > ticker->trigger_time || g_mgr.recent_time == 0) { ct_cond_signal(&g_mgr.cv); }
	ct_mutex_unlock(&g_mgr.lock);
}

void ct_ticker_reset(ct_ticker_t *ticker, ct_time64_t interval_ms) {
	if (!ticker || ct_atomic_bool_load(&g_mgr.shutdown)) { return; }

	timer__ensure_runtime();
	ct_mutex_lock(&g_mgr.lock);
	ct_timer_core_reset_ticker(&g_mgr.core, ticker, g_mgr.gettime_cb(), interval_ms);
	if (g_mgr.recent_time > ticker->trigger_time || g_mgr.recent_time == 0) { ct_cond_signal(&g_mgr.cv); }
	ct_mutex_unlock(&g_mgr.lock);
}

void ct_ticker_stop(ct_ticker_t *ticker) {
	if (!ticker) { return; }
	timer__ensure_runtime();
	ct_mutex_lock(&g_mgr.lock);
	ct_timer_core_stop(&g_mgr.core, (ct_timer_node_t *)ticker);
	ct_mutex_unlock(&g_mgr.lock);
}

static void timer__dispose_node(ct_timer_node_t *node, void *ctx) {
	CT_UNUSED(ctx);
	if (!node) { return; }
	if (node->type == CT_TIMER_NODE_TIMEOUT) { free(node); }
}
