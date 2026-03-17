#ifndef COTER_TIME_TIMER_INTERNAL_H
#define COTER_TIME_TIMER_INTERNAL_H

#include "coter/time/ticker.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ct_timer_node_type {
	CT_TIMER_NODE_TIMER = 0,
	CT_TIMER_NODE_TIMEOUT,
	CT_TIMER_NODE_TICKER,
};

typedef struct ct_timer_node {
	CT_TIMER_BASE
} ct_timer_node_t;

#ifdef __cplusplus
}
#endif

#endif  // COTER_TIME_TIMER_INTERNAL_H
