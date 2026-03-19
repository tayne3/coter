/**
 * @file timer.c
 * @brief 定时器机制
 */
#include "coter/time/timer.h"

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/time/ticker.h"

typedef enum node_type {
    NODE_TYPE_TIMER = 0,
    NODE_TYPE_TIMEOUT,
    NODE_TYPE_TICKER,
} node_type_t;

typedef struct timer_node {
    CT_TIMER_BASE
} node_t;

static struct timer_manager {
    ct_heap_t  nodes;
    ct_mutex_t lock;
    ct_cond_t  cv;

    ct_time64_t (*gettime_cb)(void);

    ct_atomic_bool_t is_shutdown;

    ct_time64_t recent;
} g_mgr = {
    .lock = CT_MUTEX_INITIALIZER,
    .cv   = CT_COND_INITIALIZER,

    .gettime_cb = ct_getuptime_ms,

    .is_shutdown = CT_ATOMIC_VAR_INIT(1),

    .recent = 0,
};

static int  node__compare(const ct_heap_node_t* a, const ct_heap_node_t* b);
static int  node__ops_start(node_t* node, enum node_type type, ct_time64_t interval_ms, ct_timer_callback_t cb,
                            void* arg);
static int  node__ops_stop(node_t* node);
static void node__reuse(node_t* node);
static void node__clear(void);

void ct_timer_init(ct_timer_t* timer) {
    if (!timer) { return; }
    timer->is_active = timer->is_queued = 0;
    timer->cb                           = NULL;
}

int ct_timer_start(ct_timer_t* timer, ct_time64_t timeout_ms, void (*cb)(void*), void* arg) {
    if (!timer || !cb) { return -1; }
    return node__ops_start((node_t*)timer, NODE_TYPE_TIMER, timeout_ms, cb, arg);
}

int ct_timer_reset(ct_timer_t* timer, ct_time64_t timeout_ms) {
    if (!timer || !timer->cb) { return -1; }
    return node__ops_start((node_t*)timer, NODE_TYPE_TIMER, timeout_ms, timer->cb, timer->arg);
}

int ct_timer_stop(ct_timer_t* timer) {
    if (!timer) { return -1; }
    return node__ops_stop((node_t*)timer);
}

int ct_set_timeout(ct_time64_t timeout_ms, void (*cb)(void*), void* arg) {
    if (!cb) { return -1; }

    ct_timer_t* timer = (ct_timer_t*)calloc(1, sizeof(ct_timer_t));
    if (!timer) { return -1; }

    int ret = node__ops_start((node_t*)timer, NODE_TYPE_TIMEOUT, timeout_ms, cb, arg);
    if (ret != 0) { free(timer); }
    return ret;
}

int ct_ticker_start(ct_ticker_t* ticker, ct_time64_t interval_ms, ct_ticker_callback_t cb, void* arg) {
    if (!ticker || !cb) { return -1; }
    return node__ops_start((node_t*)ticker, NODE_TYPE_TICKER, interval_ms, cb, arg);
}

int ct_ticker_reset(ct_ticker_t* ticker, ct_time64_t interval_ms) {
    if (!ticker || !ticker->cb) { return -1; }
    return node__ops_start((node_t*)ticker, NODE_TYPE_TICKER, interval_ms, ticker->cb, ticker->arg);
}

int ct_ticker_stop(ct_ticker_t* ticker) {
    if (!ticker) { return -1; }
    return node__ops_stop((node_t*)ticker);
}

void ct_ticker_init(ct_ticker_t* ticker) {
    if (!ticker) { return; }
    ticker->is_active = ticker->is_queued = 0;
    ticker->cb                            = NULL;
}

void ct_timer_mgr_init(ct_time64_t (*gettime_cb)(void)) {
    g_mgr.gettime_cb = gettime_cb ? gettime_cb : ct_getuptime_ms;
    g_mgr.recent     = 0;

    ct_heap_init(&g_mgr.nodes, node__compare);
    ct_atomic_bool_store(&g_mgr.is_shutdown, false);
}

void ct_timer_mgr_run(void) {
    ct_mutex_lock(&g_mgr.lock);
    while (!ct_atomic_bool_load(&g_mgr.is_shutdown)) {
        node_t* node = (node_t*)ct_heap_top(&g_mgr.nodes);
        if (!node) {
            g_mgr.recent = 0;
            ct_cond_wait(&g_mgr.cv, &g_mgr.lock);
            continue;
        }
        ct_time64_t now = g_mgr.gettime_cb();
        if (node->next_time > now) {
            ct_time64_t wait = node->next_time - now;
            if (wait > 1000) { wait = 1000; }
            if (wait < 0) { wait = 0; }
            g_mgr.recent = node->next_time;
            // printf("[%s:%d] ---- run, now: %lld, next_time: %lld, wait: %lld\n", __ct_file__, __ct_line__, now,
            // node->next_time, wait);
            ct_cond_timedwait(&g_mgr.cv, &g_mgr.lock, (uint32_t)wait);
            continue;
        }

        ct_timer_callback_t cb  = node->cb;
        void*               arg = node->arg;

        node->is_queued = 0;
        ct_heap_pop(&g_mgr.nodes);
        node__reuse(node);

        ct_mutex_unlock(&g_mgr.lock);
        if (cb) { cb(arg); }
        ct_mutex_lock(&g_mgr.lock);
    }
    node__clear();
    ct_mutex_unlock(&g_mgr.lock);
}

void ct_timer_mgr_close(void) {
    if (ct_atomic_bool_load(&g_mgr.is_shutdown)) { return; }

    ct_mutex_lock(&g_mgr.lock);
    ct_atomic_bool_store(&g_mgr.is_shutdown, true);
    ct_cond_broadcast(&g_mgr.cv);
    ct_mutex_unlock(&g_mgr.lock);
}

static int node__compare(const ct_heap_node_t* a, const ct_heap_node_t* b) {
    const node_t* l = (const node_t*)a;
    const node_t* r = (const node_t*)b;
    if (l->next_time < r->next_time) { return -1; }
    if (l->next_time > r->next_time) { return 1; }
    return 0;
}

static int node__ops_start(node_t* node, enum node_type type, ct_time64_t interval_ms, ct_timer_callback_t cb,
                           void* arg) {
    if (ct_atomic_bool_load(&g_mgr.is_shutdown)) { return -1; }

    const ct_time64_t now = g_mgr.gettime_cb();
    ct_mutex_lock(&g_mgr.lock);
    node->is_active = 0;
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

    // printf("[%s:%d] ---- node__ops_start, now: %lld, next_time: %lld, interval: %lld\n", __ct_file__, __ct_line__,
    // now, node->next_time, interval_ms);

    ct_heap_insert(&g_mgr.nodes, &node->node);
    node->is_queued = 1;
    if (g_mgr.recent > node->next_time || g_mgr.recent == 0) { ct_cond_signal(&g_mgr.cv); }
    ct_mutex_unlock(&g_mgr.lock);
    return 0;
}

static int node__ops_stop(node_t* node) {
    if (ct_atomic_bool_load(&g_mgr.is_shutdown)) { return -1; }

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

static void node__reuse(node_t* node) {
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

static void node__clear(void) {
    while (1) {
        node_t* node = (node_t*)ct_heap_pop(&g_mgr.nodes);
        if (!node) { break; }
        node->is_active = 0;
        node->is_queued = 0;
        if (node->type == NODE_TYPE_TIMEOUT) { free(node); }
    }
}
