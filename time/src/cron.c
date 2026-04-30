/**
 * @file cron.c
 * @brief cron 调度器
 */
#include "coter/time/cron.h"

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

static struct cron_manager {
    ct_heap_t  nodes;
    ct_mutex_t lock;
    ct_cond_t  cv;

    ct_cron_gettime_cb realtime_cb;
    ct_cron_gettime_cb monotonic_cb;

    ct_atomic_bool_t is_shutdown;

    ct_time64_t last_realtime;
    ct_time64_t last_monotonic;
    ct_time_t   now;
} g_mgr = {
    .lock = CT_MUTEX_INITIALIZER,
    .cv   = CT_COND_INITIALIZER,

    .realtime_cb  = ct_gettimeofday_ms,
    .monotonic_cb = ct_getuptime_ms,

    .is_shutdown = CT_ATOMIC_VAR_INIT(1),

    .last_realtime  = 0,
    .last_monotonic = 0,
    .now            = 0,
};

static int  cron__compare(const ct_heap_node_t* a, const ct_heap_node_t* b);
static int  cron__start(ct_cron_t* cron, ct_time_t now, int minute, int hour, int day, int week, int month,
                        ct_cron_callback_t cb, void* arg);
static int  cron__stop(ct_cron_t* cron);
static int  cron__resched(ct_cron_t* cron, ct_time_t now);
static bool cron__handle_time_jump(void);
static void cron__clear(ct_heap_t* node);

void ct_cron_init(ct_cron_t* cron) {
    if (!cron) { return; }
    cron->is_active = cron->is_queued = 0;
    cron->cb                          = NULL;
}

int ct_cron_start(ct_cron_t* cron, int minute, int hour, int day, int week, int month, ct_cron_callback_t callback,
                  void* arg) {
    if (!cron || !callback || ct_atomic_bool_load(&g_mgr.is_shutdown)) { return -1; }

    int             ret;
    const ct_time_t now = (ct_time_t)(g_mgr.realtime_cb() / 1000);
    ct_mutex_lock(&g_mgr.lock);
    ret = cron__start(cron, now, minute, hour, day, week, month, callback, arg);
    if (ret == 0) { ct_cond_signal(&g_mgr.cv); }
    ct_mutex_unlock(&g_mgr.lock);
    return ret;
}

int ct_cron_reset(ct_cron_t* cron, int minute, int hour, int day, int week, int month) {
    if (!cron || !cron->cb || ct_atomic_bool_load(&g_mgr.is_shutdown)) { return -1; }

    int             ret;
    const ct_time_t now = (ct_time_t)(g_mgr.realtime_cb() / 1000);
    ct_mutex_lock(&g_mgr.lock);
    ret = cron__start(cron, now, minute, hour, day, week, month, cron->cb, cron->arg);
    if (ret == 0) { ct_cond_signal(&g_mgr.cv); }
    ct_mutex_unlock(&g_mgr.lock);
    return ret;
}

int ct_cron_stop(ct_cron_t* cron) {
    if (!cron || ct_atomic_bool_load(&g_mgr.is_shutdown)) { return -1; }

    int ret;
    ct_mutex_lock(&g_mgr.lock);
    ret = cron__stop(cron);
    ct_mutex_unlock(&g_mgr.lock);
    return ret;
}

void ct_cron_mgr_init(ct_cron_gettime_cb realtime_cb, ct_cron_gettime_cb monotonic_cb) {
    g_mgr.realtime_cb    = realtime_cb ? realtime_cb : ct_gettimeofday_ms;
    g_mgr.monotonic_cb   = monotonic_cb ? monotonic_cb : ct_getuptime_ms;
    g_mgr.last_realtime  = g_mgr.realtime_cb();
    g_mgr.last_monotonic = g_mgr.monotonic_cb();
    g_mgr.now            = (ct_time_t)(g_mgr.last_realtime / 1000);

    ct_heap_init(&g_mgr.nodes, cron__compare);
    ct_atomic_bool_store(&g_mgr.is_shutdown, false);
}

void ct_cron_mgr_run(void) {
    ct_mutex_lock(&g_mgr.lock);
    while (!ct_atomic_bool_load(&g_mgr.is_shutdown)) {
        ct_cron_t* cron = (ct_cron_t*)ct_heap_top(&g_mgr.nodes);
        if (!cron) {
            ct_cond_wait(&g_mgr.cv, &g_mgr.lock);
            continue;
        }
        if (cron__handle_time_jump()) { continue; }
        if (cron->next_time > g_mgr.now) {
            ct_time64_t wait = (ct_time64_t)cron->next_time * 1000 - g_mgr.last_realtime;
            if (wait > 1000) { wait = 1000; }
            if (wait < 0) { wait = 0; }
            ct_cond_wait_for(&g_mgr.cv, &g_mgr.lock, wait);
            continue;
        }

        ct_cron_callback_t cb  = cron->cb;
        void*              arg = cron->arg;

        cron->is_queued = 0;
        ct_heap_pop(&g_mgr.nodes);
        cron__resched(cron, g_mgr.now);

        ct_mutex_unlock(&g_mgr.lock);
        if (cb) { cb(arg); }
        ct_mutex_lock(&g_mgr.lock);
    }
    cron__clear(&g_mgr.nodes);
    ct_mutex_unlock(&g_mgr.lock);
}

void ct_cron_mgr_close(void) {
    if (ct_atomic_bool_load(&g_mgr.is_shutdown)) { return; }

    ct_mutex_lock(&g_mgr.lock);
    ct_atomic_bool_store(&g_mgr.is_shutdown, true);
    ct_cond_broadcast(&g_mgr.cv);
    ct_mutex_unlock(&g_mgr.lock);
}

static int cron__compare(const ct_heap_node_t* a, const ct_heap_node_t* b) {
    const ct_cron_t* l = CONTAINER_OF(a, ct_cron_t, node);
    const ct_cron_t* r = CONTAINER_OF(b, ct_cron_t, node);
    if (l->next_time < r->next_time) { return -1; }
    if (l->next_time > r->next_time) { return 1; }
    return 0;
}

static int cron__start(ct_cron_t* cron, ct_time_t now, int minute, int hour, int day, int week, int month,
                       ct_cron_callback_t cb, void* arg) {
    cron__stop(cron);

    cron->minute = minute;
    cron->hour   = hour;
    cron->day    = day;
    cron->week   = week;
    cron->month  = month;
    cron->cb     = cb;
    cron->arg    = arg;

    return cron__resched(cron, now);
}

static int cron__stop(ct_cron_t* cron) {
    if (!cron->is_active) { return -1; }
    cron->is_active = 0;
    if (cron->is_queued) {
        cron->is_queued = 0;
        ct_heap_remove(&g_mgr.nodes, &cron->node);
    }
    return 0;
}

static int cron__resched(ct_cron_t* cron, ct_time_t now) {
    cron->next_time = ct_cron_next_timeout(now, cron->minute, cron->hour, cron->day, cron->week, cron->month);
    cron->is_active = cron->next_time >= 0;

    if (cron->is_active) {
        ct_heap_insert(&g_mgr.nodes, &cron->node);
        cron->is_queued = 1;
        return 0;
    }
    return -1;
}

static bool cron__handle_time_jump() {
    const ct_time64_t now_rt   = g_mgr.realtime_cb();
    const ct_time64_t now_mono = g_mgr.monotonic_cb();
    ct_time64_t       skew     = (now_rt - g_mgr.last_realtime) - (now_mono - g_mgr.last_monotonic);

    g_mgr.last_realtime  = now_rt;
    g_mgr.last_monotonic = now_mono;
    g_mgr.now            = (ct_time_t)(now_rt / 1000);

    if (skew < 0) { skew = -skew; }
    if (skew < 1000) { return false; }

    ct_heap_t nodes;
    ct_heap_init(&nodes, cron__compare);
    ct_heap_move(&nodes, &g_mgr.nodes);

    while (1) {
        ct_cron_t* cron = (ct_cron_t*)ct_heap_pop(&nodes);
        if (!cron) { break; }
        if (cron->is_active) { cron__resched(cron, g_mgr.now); }
    }
    return true;
}

static void cron__clear(ct_heap_t* node) {
    while (1) {
        ct_cron_t* cron = (ct_cron_t*)ct_heap_pop(node);
        if (!cron) { break; }
        cron->is_active = 0;
        cron->is_queued = 0;
    }
}
