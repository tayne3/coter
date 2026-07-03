/**
 * @file cron.c
 * @brief Cron scheduler implementation
 */
#include "coter/time/cron.h"

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "internal.h"

// -------------------------[STATIC DECLARATION]-------------------------

// Compare function for the min-heap to order jobs by next_time
static int cron__compare(const ct_heap_node_t* a, const ct_heap_node_t* b);

static struct {
    ct_heap_t  nodes;  // Min-heap of scheduled cron jobs
    ct_mutex_t lock;   // Protects global manager state
    ct_cond_t  cv;     // Wakes up manager thread on changes

    ct_atomic_bool_t is_running;      // Manager thread running state
    bool             stop_requested;  // Shutdown signal

    ct_time64_t last_realtime;   // Last wall-clock time (ms)
    ct_time64_t last_monotonic;  // Last steady clock time (ms)
    ct_time_t   now;             // Current time (seconds)
} g_mgr = {
    .nodes = CT_HEAP_INIT(cron__compare),
    .lock  = CT_MUTEX_INITIALIZER,
    .cv    = CT_COND_INITIALIZER,

    .is_running     = CT_ATOMIC_VAR_INIT(false),
    .stop_requested = false,

    .last_realtime  = 0,
    .last_monotonic = 0,
    .now            = 0,
};

// Internally starts or restarts a cron job
static int cron__start(ct_cron_t* self, ct_time_t now, int minute, int hour, int day, int week, int month,
                       ct_cron_callback_t cb, void* arg);
// Internally stops a cron job and removes it from the queue
static int cron__stop(ct_cron_t* self);
// Calculates the next execution time and queues the job
static int cron__resched(ct_cron_t* self, ct_time_t now);
// Detects system clock jumps and reschedules all active jobs if needed.
// Returns true if a time jump was handled.
static bool cron__handle_time_jump(void);
// Clears all pending jobs from the given heap
static void cron__clear(ct_heap_t* node);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_cron_mgr_process_once(void) {
    ct_mutex_lock(&g_mgr.lock);
    if (g_mgr.stop_requested) {
        g_mgr.stop_requested = false;
        g_mgr.last_realtime  = 0;
        g_mgr.last_monotonic = 0;
        g_mgr.now            = 0;
        cron__clear(&g_mgr.nodes);
        ct_mutex_unlock(&g_mgr.lock);
        return;
    }

    for (;;) {
        cron__handle_time_jump();

        ct_cron_t* top = (ct_cron_t*)ct_heap_top(&g_mgr.nodes);
        if (!top || top->next_time > g_mgr.now) { break; }

        ct_cron_callback_t cb  = top->cb;
        void*              arg = top->arg;

        top->is_queued = 0;
        ct_heap_pop(&g_mgr.nodes);
        cron__resched(top, g_mgr.now);

        ct_mutex_unlock(&g_mgr.lock);
        if (cb) { cb(arg); }
        ct_mutex_lock(&g_mgr.lock);
    }
    ct_mutex_unlock(&g_mgr.lock);
}

void ct_cron_mgr_run(void) {
    ct_mutex_lock(&g_mgr.lock);

    if (g_mgr.stop_requested) {
        g_mgr.stop_requested = false;
        g_mgr.last_realtime  = 0;
        g_mgr.last_monotonic = 0;
        g_mgr.now            = 0;
        cron__clear(&g_mgr.nodes);
        ct_mutex_unlock(&g_mgr.lock);
        return;
    }

    ct_atomic_bool_store(&g_mgr.is_running, true);

    while (ct_atomic_bool_load(&g_mgr.is_running)) {
        cron__handle_time_jump(); /* Refresh g_mgr.now, regardless of whether a job is executed */

        ct_cron_t* top = (ct_cron_t*)ct_heap_top(&g_mgr.nodes);
        if (!top) {
            ct_cond_wait(&g_mgr.cv, &g_mgr.lock);
            continue;
        }
        if (top->next_time <= g_mgr.now) {
            ct_cron_callback_t cb  = top->cb;
            void*              arg = top->arg;
            top->is_queued         = 0;
            ct_heap_pop(&g_mgr.nodes);
            cron__resched(top, g_mgr.now);

            ct_mutex_unlock(&g_mgr.lock);
            if (cb) { cb(arg); }
            ct_mutex_lock(&g_mgr.lock);
        } else {
            ct_time64_t wait = (ct_time64_t)top->next_time * 1000 - g_mgr.last_realtime;
            if (wait <= 0) { continue; }
            if (wait > 1000) { wait = 1000; }
            ct_cond_wait_for(&g_mgr.cv, &g_mgr.lock, wait);
        }
    }

    g_mgr.stop_requested = false;
    g_mgr.last_realtime  = 0;
    g_mgr.last_monotonic = 0;
    g_mgr.now            = 0;
    cron__clear(&g_mgr.nodes);
    ct_mutex_unlock(&g_mgr.lock);
}

void ct_cron_mgr_shutdown(void) {
    ct_mutex_lock(&g_mgr.lock);
    g_mgr.stop_requested = true;
    ct_atomic_bool_store(&g_mgr.is_running, false);
    ct_cond_broadcast(&g_mgr.cv);
    ct_mutex_unlock(&g_mgr.lock);
}

void ct_cron_init(ct_cron_t* self) {
    if (!self) { return; }
    self->is_active = self->is_queued = 0;
    self->cb                          = NULL;
}

int ct_cron_start(ct_cron_t* self, int minute, int hour, int day, int week, int month, ct_cron_callback_t callback,
                  void* arg) {
    if (!self || !callback) { return -1; }

    int             rc;
    const ct_time_t now = (ct_time_t)(ct_gettimeofday_ms() / 1000);
    ct_mutex_lock(&g_mgr.lock);
    rc = cron__start(self, now, minute, hour, day, week, month, callback, arg);
    if (rc == 0) { ct_cond_signal(&g_mgr.cv); }
    ct_mutex_unlock(&g_mgr.lock);
    return rc;
}

int ct_cron_reset(ct_cron_t* self, int minute, int hour, int day, int week, int month) {
    if (!self || !self->cb) { return -1; }

    int             rc;
    const ct_time_t now = (ct_time_t)(ct_gettimeofday_ms() / 1000);
    ct_mutex_lock(&g_mgr.lock);
    rc = cron__start(self, now, minute, hour, day, week, month, self->cb, self->arg);
    if (rc == 0) { ct_cond_signal(&g_mgr.cv); }
    ct_mutex_unlock(&g_mgr.lock);
    return rc;
}

int ct_cron_stop(ct_cron_t* self) {
    if (!self) { return -1; }

    int rc;
    ct_mutex_lock(&g_mgr.lock);
    rc = cron__stop(self);
    ct_mutex_unlock(&g_mgr.lock);
    return rc;
}

// -------------------------[STATIC DEFINITION]-------------------------

static int cron__compare(const ct_heap_node_t* a, const ct_heap_node_t* b) {
    const ct_cron_t* l = CT_CONTAINER_OF(a, ct_cron_t, node);
    const ct_cron_t* r = CT_CONTAINER_OF(b, ct_cron_t, node);
    if (l->next_time < r->next_time) { return -1; }
    if (l->next_time > r->next_time) { return 1; }
    return 0;
}

static int cron__start(ct_cron_t* self, ct_time_t now, int minute, int hour, int day, int week, int month,
                       ct_cron_callback_t cb, void* arg) {
    cron__stop(self);

    self->minute = minute;
    self->hour   = hour;
    self->day    = day;
    self->week   = week;
    self->month  = month;
    self->cb     = cb;
    self->arg    = arg;

    return cron__resched(self, now);
}

static int cron__stop(ct_cron_t* self) {
    if (!self->is_active) { return -1; }
    self->is_active = 0;
    if (self->is_queued) {
        self->is_queued = 0;
        ct_heap_remove(&g_mgr.nodes, &self->node);
    }
    return 0;
}

static int cron__resched(ct_cron_t* self, ct_time_t now) {
    self->next_time = ct_cron_next_timeout(now, self->minute, self->hour, self->day, self->week, self->month);
    self->is_active = self->next_time >= 0;

    if (self->is_active) {
        ct_heap_insert(&g_mgr.nodes, &self->node);
        self->is_queued = 1;
        return 0;
    }
    return -1;
}

static bool cron__handle_time_jump(void) {
    ct_time64_t now_rt, now_mono;
    int         retries = 0;

    do {
        now_mono = ct_getuptime_ms();
        now_rt   = ct_gettimeofday_ms();
    } while (now_mono + 5 <= ct_getuptime_ms() && ++retries < 5);

    bool is_first_call = (g_mgr.last_realtime == 0);

    ct_time64_t skew = (now_rt - g_mgr.last_realtime) - (now_mono - g_mgr.last_monotonic);

    g_mgr.last_realtime  = now_rt;
    g_mgr.last_monotonic = now_mono;
    g_mgr.now            = (ct_time_t)(now_rt / 1000);

    if (is_first_call) { return false; }
    if (skew < 0) { skew = -skew; }
    if (skew < 1000) { return false; }

    ct_heap_t nodes;
    ct_heap_init(&nodes, cron__compare);
    ct_heap_move(&nodes, &g_mgr.nodes);

    for (;;) {
        ct_cron_t* self = (ct_cron_t*)ct_heap_pop(&nodes);
        if (!self) { break; }
        if (self->is_active) { cron__resched(self, g_mgr.now); }
    }
    return true;
}

static void cron__clear(ct_heap_t* node) {
    for (;;) {
        ct_cron_t* self = (ct_cron_t*)ct_heap_pop(node);
        if (!self) { break; }
        self->is_active = 0;
        self->is_queued = 0;
    }
}
