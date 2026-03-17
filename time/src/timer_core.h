#ifndef COTER_TIME_TIMER_CORE_H
#define COTER_TIME_TIMER_CORE_H

#include "coter/container/heap.h"
#include "coter/core/platform.h"
#include "coter/time/timer.h"
#include "coter/time/ticker.h"
#include "timer_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_timer_core {
	ct_heap_t nodes;
} ct_timer_core_t;

void ct_timer_core_init(ct_timer_core_t* core);
bool ct_timer_core_is_empty(const ct_timer_core_t* core);
ct_time64_t ct_timer_core_next_deadline(const ct_timer_core_t* core);

void ct_timer_core_start_timer(ct_timer_core_t* core, ct_timer_t* timer, ct_time64_t now, ct_time64_t timeout_ms,
							   ct_timer_callback_t cb, void* arg);
void ct_timer_core_reset_timer(ct_timer_core_t* core, ct_timer_t* timer, ct_time64_t now, ct_time64_t timeout_ms);
void ct_timer_core_start_timeout(ct_timer_core_t* core, ct_timer_t* timer, ct_time64_t now, ct_time64_t timeout_ms,
								 ct_timer_callback_t cb, void* arg);
void ct_timer_core_start_ticker(ct_timer_core_t* core, ct_ticker_t* ticker, ct_time64_t now, ct_time64_t interval_ms,
								ct_timer_callback_t cb, void* arg);
void ct_timer_core_reset_ticker(ct_timer_core_t* core, ct_ticker_t* ticker, ct_time64_t now, ct_time64_t interval_ms);
void ct_timer_core_stop(ct_timer_core_t* core, ct_timer_node_t* node);

ct_timer_node_t* ct_timer_core_pop_expired(ct_timer_core_t* core, ct_time64_t now);
void ct_timer_core_prepare_fire(ct_timer_core_t* core, ct_timer_node_t* node);
void ct_timer_core_clear(ct_timer_core_t* core, void (*dispose)(ct_timer_node_t* node, void* ctx), void* ctx);

#ifdef __cplusplus
}
#endif

#endif  // COTER_TIME_TIMER_CORE_H
