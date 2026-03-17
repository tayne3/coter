#include "timer_core.h"

static int timer_core__cmp(const ct_heap_node_t* a, const ct_heap_node_t* b) {
	const ct_timer_node_t* ta = (const ct_timer_node_t*)a;
	const ct_timer_node_t* tb = (const ct_timer_node_t*)b;
	if (ta->trigger_time < tb->trigger_time) { return -1; }
	if (ta->trigger_time > tb->trigger_time) { return 1; }
	return 0;
}

static void timer_core__detach_if_active(ct_timer_core_t* core, ct_timer_node_t* node) {
	if (!core || !node || !node->is_active) { return; }
	ct_heap_remove(&core->nodes, &node->node);
	node->is_active = 0;
}

void ct_timer_core_init(ct_timer_core_t* core) {
	if (!core) { return; }
	ct_heap_init(&core->nodes, timer_core__cmp);
}

bool ct_timer_core_is_empty(const ct_timer_core_t* core) {
	return !core || ct_heap_is_empty(&core->nodes);
}

ct_time64_t ct_timer_core_next_deadline(const ct_timer_core_t* core) {
	if (!core) { return -1; }
	ct_timer_node_t* node = (ct_timer_node_t*)ct_heap_top(&core->nodes);
	return node ? node->trigger_time : -1;
}

void ct_timer_core_start_timer(ct_timer_core_t* core, ct_timer_t* timer, ct_time64_t now, ct_time64_t timeout_ms,
							   ct_timer_callback_t cb, void* arg) {
	if (!core || !timer || !cb) { return; }
	timer_core__detach_if_active(core, (ct_timer_node_t*)timer);
	timer->trigger_time = now + timeout_ms;
	timer->cb           = cb;
	timer->arg          = arg;
	timer->type         = CT_TIMER_NODE_TIMER;
	timer->is_active    = 1;
	ct_heap_insert(&core->nodes, &timer->node);
}

void ct_timer_core_reset_timer(ct_timer_core_t* core, ct_timer_t* timer, ct_time64_t now, ct_time64_t timeout_ms) {
	if (!core || !timer) { return; }
	timer_core__detach_if_active(core, (ct_timer_node_t*)timer);
	timer->trigger_time = now + timeout_ms;
	timer->type         = CT_TIMER_NODE_TIMER;
	timer->is_active    = 1;
	ct_heap_insert(&core->nodes, &timer->node);
}

void ct_timer_core_start_timeout(ct_timer_core_t* core, ct_timer_t* timer, ct_time64_t now, ct_time64_t timeout_ms,
								 ct_timer_callback_t cb, void* arg) {
	if (!core || !timer || !cb) { return; }
	timer->trigger_time = now + timeout_ms;
	timer->cb           = cb;
	timer->arg          = arg;
	timer->type         = CT_TIMER_NODE_TIMEOUT;
	timer->is_active    = 1;
	ct_heap_insert(&core->nodes, &timer->node);
}

void ct_timer_core_start_ticker(ct_timer_core_t* core, ct_ticker_t* ticker, ct_time64_t now, ct_time64_t interval_ms,
								ct_timer_callback_t cb, void* arg) {
	if (!core || !ticker || !cb) { return; }
	timer_core__detach_if_active(core, (ct_timer_node_t*)ticker);
	ticker->interval     = interval_ms;
	ticker->trigger_time = now + interval_ms;
	ticker->cb           = cb;
	ticker->arg          = arg;
	ticker->type         = CT_TIMER_NODE_TICKER;
	ticker->is_active    = 1;
	ct_heap_insert(&core->nodes, &ticker->node);
}

void ct_timer_core_reset_ticker(ct_timer_core_t* core, ct_ticker_t* ticker, ct_time64_t now, ct_time64_t interval_ms) {
	if (!core || !ticker) { return; }
	timer_core__detach_if_active(core, (ct_timer_node_t*)ticker);
	ticker->interval     = interval_ms;
	ticker->trigger_time = now + interval_ms;
	ticker->type         = CT_TIMER_NODE_TICKER;
	ticker->is_active    = 1;
	ct_heap_insert(&core->nodes, &ticker->node);
}

void ct_timer_core_stop(ct_timer_core_t* core, ct_timer_node_t* node) {
	if (!core || !node || !node->is_active) { return; }
	ct_heap_remove(&core->nodes, &node->node);
	node->is_active = 0;
}

ct_timer_node_t* ct_timer_core_pop_expired(ct_timer_core_t* core, ct_time64_t now) {
	if (!core) { return NULL; }
	ct_timer_node_t* node = (ct_timer_node_t*)ct_heap_top(&core->nodes);
	if (!node || node->trigger_time > now) { return NULL; }
	return (ct_timer_node_t*)ct_heap_pop(&core->nodes);
}

void ct_timer_core_prepare_fire(ct_timer_core_t* core, ct_timer_node_t* node) {
	if (!core || !node || !node->is_active) { return; }
	switch (node->type) {
		case CT_TIMER_NODE_TIMER:
		case CT_TIMER_NODE_TIMEOUT:
			node->is_active = 0;
			break;
		case CT_TIMER_NODE_TICKER: {
			ct_ticker_t* ticker = (ct_ticker_t*)node;
			node->trigger_time += ticker->interval;
			ct_heap_insert(&core->nodes, &node->node);
		} break;
		default: break;
	}
}

void ct_timer_core_clear(ct_timer_core_t* core, void (*dispose)(ct_timer_node_t* node, void* ctx), void* ctx) {
	if (!core) { return; }
	while (!ct_heap_is_empty(&core->nodes)) {
		ct_timer_node_t* node = (ct_timer_node_t*)ct_heap_pop(&core->nodes);
		node->is_active = 0;
		if (dispose) { dispose(node, ctx); }
	}
}
