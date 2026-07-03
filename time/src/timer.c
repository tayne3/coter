/**
 * @file timer.c
 * @brief 定时器机制
 */
#include "coter/time/timer.h"

#include <stdlib.h>

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

typedef enum node_type {
    NODE_TYPE_TIMER = 0,
    NODE_TYPE_TIMEOUT,
    NODE_TYPE_TICKER,
} node_type_t;

typedef struct timer_node {
    CT_TIMER_BASE
} node_t;

static int timer__compare(const ct_heap_node_t* a, const ct_heap_node_t* b);

static struct {
    ct_heap_t  nodes;
    ct_mutex_t lock;
    ct_cond_t  cv;

    ct_atomic_bool_t is_running;
    bool             stop_requested;
    ct_time64_t      recent;
} g_mgr = {
    .nodes = CT_HEAP_INIT(timer__compare),
    .lock  = CT_MUTEX_INITIALIZER,
    .cv    = CT_COND_INITIALIZER,

    .is_running     = CT_ATOMIC_VAR_INIT(false),
    .stop_requested = false,
    .recent         = 0,
};

static int  timer__ops_start(node_t* node, enum node_type type, ct_time64_t interval_ms, ct_timer_callback_t cb,
                             void* arg);
static int  timer__ops_stop(node_t* node);
static void timer__reuse(node_t* node);
static void timer__clear(void);

void ct_timer_init(ct_timer_t* self) {
    if (!self) { return; }
    self->is_active = self->is_queued = 0;
    self->cb                          = NULL;
}

int ct_timer_start(ct_timer_t* self, ct_time64_t timeout_ms, void (*cb)(void*), void* arg) {
    if (!self || !cb) { return -1; }
    return timer__ops_start((node_t*)self, NODE_TYPE_TIMER, timeout_ms, cb, arg);
}

int ct_timer_reset(ct_timer_t* self, ct_time64_t timeout_ms) {
    if (!self || !self->cb) { return -1; }
    return timer__ops_start((node_t*)self, NODE_TYPE_TIMER, timeout_ms, self->cb, self->arg);
}

int ct_timer_stop(ct_timer_t* self) {
    if (!self) { return -1; }
    return timer__ops_stop((node_t*)self);
}

int ct_set_timeout(ct_time64_t timeout_ms, void (*cb)(void*), void* arg) {
    if (!cb) { return -1; }

    ct_timer_t* self = (ct_timer_t*)calloc(1, sizeof(ct_timer_t));
    if (!self) { return -1; }

    int rc;
    if ((rc = timer__ops_start((node_t*)self, NODE_TYPE_TIMEOUT, timeout_ms, cb, arg)) != 0) { free(self); }
    return rc;
}

int ct_ticker_start(ct_ticker_t* ticker, ct_time64_t interval_ms, ct_ticker_callback_t cb, void* arg) {
    if (!ticker || !cb) { return -1; }
    return timer__ops_start((node_t*)ticker, NODE_TYPE_TICKER, interval_ms, cb, arg);
}

int ct_ticker_reset(ct_ticker_t* ticker, ct_time64_t interval_ms) {
    if (!ticker || !ticker->cb) { return -1; }
    return timer__ops_start((node_t*)ticker, NODE_TYPE_TICKER, interval_ms, ticker->cb, ticker->arg);
}

int ct_ticker_stop(ct_ticker_t* ticker) {
    if (!ticker) { return -1; }
    return timer__ops_stop((node_t*)ticker);
}

void ct_ticker_init(ct_ticker_t* ticker) {
    if (!ticker) { return; }
    ticker->is_active = ticker->is_queued = 0;
    ticker->cb                            = NULL;
}

void ct_timer_mgr_process_once(void) {
    ct_mutex_lock(&g_mgr.lock);
    if (g_mgr.stop_requested) {
        g_mgr.stop_requested = false;
        timer__clear();
        ct_mutex_unlock(&g_mgr.lock);
        return;
    }

    for (;;) {
        node_t* node = (node_t*)ct_heap_top(&g_mgr.nodes);
        if (!node) {
            g_mgr.recent = 0;
            break;
        }

        ct_time64_t now = ct_getuptime_ms();
        if (node->next_time > now) { break; }

        ct_timer_callback_t cb  = node->cb;
        void*               arg = node->arg;

        node->is_queued = 0;
        ct_heap_pop(&g_mgr.nodes);
        timer__reuse(node);

        ct_mutex_unlock(&g_mgr.lock);
        if (cb) { cb(arg); }
        ct_mutex_lock(&g_mgr.lock);
    }

    ct_mutex_unlock(&g_mgr.lock);
}

void ct_timer_mgr_run(void) {
    ct_mutex_lock(&g_mgr.lock);
    if (g_mgr.stop_requested) { goto exit; }
    ct_atomic_bool_store(&g_mgr.is_running, true);

    while (ct_atomic_bool_load(&g_mgr.is_running)) {
        node_t* node = (node_t*)ct_heap_top(&g_mgr.nodes);
        if (!node) {
            g_mgr.recent = 0;
            ct_cond_wait(&g_mgr.cv, &g_mgr.lock);
            continue;
        }

        ct_time64_t now = ct_getuptime_ms();
        if (node->next_time <= now) {
            ct_timer_callback_t cb  = node->cb;
            void*               arg = node->arg;

            node->is_queued = 0;
            ct_heap_pop(&g_mgr.nodes);
            timer__reuse(node);

            ct_mutex_unlock(&g_mgr.lock);
            if (cb) { cb(arg); }
            ct_mutex_lock(&g_mgr.lock);
        } else {
            ct_time64_t wait = node->next_time - now;
            if (wait <= 0) { continue; }
            if (wait > 1000) { wait = 1000; }
            g_mgr.recent = node->next_time;
            ct_cond_wait_for(&g_mgr.cv, &g_mgr.lock, wait);
        }
    }

exit:
    g_mgr.stop_requested = false;
    timer__clear();
    ct_mutex_unlock(&g_mgr.lock);
}

void ct_timer_mgr_shutdown(void) {
    ct_mutex_lock(&g_mgr.lock);
    g_mgr.stop_requested = true;
    ct_atomic_bool_store(&g_mgr.is_running, false);
    ct_cond_broadcast(&g_mgr.cv);
    ct_mutex_unlock(&g_mgr.lock);
}

static int timer__compare(const ct_heap_node_t* a, const ct_heap_node_t* b) {
    const node_t* l = (const node_t*)a;
    const node_t* r = (const node_t*)b;
    if (l->next_time < r->next_time) { return -1; }
    if (l->next_time > r->next_time) { return 1; }
    return 0;
}

static int timer__ops_start(node_t* node, enum node_type type, ct_time64_t interval_ms, ct_timer_callback_t cb,
                            void* arg) {
    ct_mutex_lock(&g_mgr.lock);
    const ct_time64_t now = ct_getuptime_ms();
    node->is_active       = 0;
    if (node->is_queued) {
        node->is_queued = 0;
        ct_heap_remove(&g_mgr.nodes, &node->node);
    }

    switch (type) {
        case NODE_TYPE_TIMER:
        case NODE_TYPE_TIMEOUT: break;
        case NODE_TYPE_TICKER: ((ct_ticker_t*)node)->interval = interval_ms; break;
        default: ct_mutex_unlock(&g_mgr.lock); return -1;
    }

    node->type      = type;
    node->next_time = now + interval_ms;
    node->cb        = cb;
    node->arg       = arg;
    node->is_active = 1;

    ct_heap_insert(&g_mgr.nodes, &node->node);
    node->is_queued = 1;
    if (g_mgr.recent > node->next_time || g_mgr.recent == 0) { ct_cond_signal(&g_mgr.cv); }
    ct_mutex_unlock(&g_mgr.lock);
    return 0;
}

static int timer__ops_stop(node_t* node) {
    ct_mutex_lock(&g_mgr.lock);
    if (!node->is_active) {
        ct_mutex_unlock(&g_mgr.lock);
        return -1;
    }
    node->is_active = 0;
    if (node->is_queued) {
        node->is_queued = 0;
        ct_heap_remove(&g_mgr.nodes, &node->node);
    }
    ct_mutex_unlock(&g_mgr.lock);
    return 0;
}

static void timer__reuse(node_t* node) {
    switch (node->type) {
        case NODE_TYPE_TICKER: {
            node->next_time += ((ct_ticker_t*)node)->interval;
            ct_heap_insert(&g_mgr.nodes, &node->node);
            node->is_queued = 1;
        } break;
        case NODE_TYPE_TIMEOUT: {
            node->is_active = 0;
            free(node);
        } break;
        case NODE_TYPE_TIMER:
        default: {
            node->is_active = 0;
        } break;
    }
}

static void timer__clear(void) {
    for (;;) {
        node_t* node = (node_t*)ct_heap_pop(&g_mgr.nodes);
        if (!node) { break; }
        node->is_active = 0;
        node->is_queued = 0;
        if (node->type == NODE_TYPE_TIMEOUT) { free(node); }
    }
}
