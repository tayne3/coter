#include "coter/time/cron.h"

#include <catch.hpp>

#include "coter/core/platform.h"
#include "coter/sync/atomic.h"
#include "coter/sync/event.h"
#include "coter/thread/thread.h"

namespace {

struct test_env {
    ct_atomic_long_t realtime        = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_long_t monotonic       = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t  manager_stopped = CT_ATOMIC_VAR_INIT(0);
    ct_thread_t      manager_thread;
    ct_cron_t        wakeup;
    bool             started = false;

    test_env() {}

    ~test_env() {
        if (!started) { return; }
        ct_cron_mgr_close();
        (void)ct_thread_join(manager_thread, nullptr);
    }
};

static test_env g_env;

void do_nothing_cb(void*) {
}

struct callback_ctx {
    ct_event_t      event;
    ct_atomic_int_t count;

    callback_ctx() {
        ct_event_init(&event);
        ct_atomic_int_store(&count, 0);
    }
    ~callback_ctx() { ct_event_destroy(&event); }
    void wait() { REQUIRE(ct_event_wait(&event) == 0); }
    bool wait_timeout(uint32_t ms) { return ct_event_timedwait(&event, ms) == 0; }
};

void event_count_cb(void* arg) {
    callback_ctx* ctx = (callback_ctx*)arg;
    ct_atomic_int_add(&ctx->count, 1);
    ct_event_signal(&ctx->event);
}

struct arg_holder {
    void*      captured;
    ct_event_t event;
};

void verify_arg_cb(void* arg) {
    arg_holder* h = (arg_holder*)arg;
    h->captured   = arg;
    ct_event_signal(&h->event);
}

int cron_thread_run(void* arg) {
    test_env* env = (test_env*)arg;
    ct_cron_mgr_run();
    ct_atomic_int_store(&env->manager_stopped, 1);
    return 0;
}

ct_time64_t mock_realtime_ms() {
    return static_cast<ct_time64_t>(ct_atomic_long_load(&g_env.realtime)) * 1000;
}

ct_time64_t mock_monotonic_ms() {
    return static_cast<ct_time64_t>(ct_atomic_long_load(&g_env.monotonic)) * 1000;
}

ct_time_t make_test_time(int year, int month, int day, int hour, int min, int sec) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year  = year - 1900;
    tm.tm_mon   = month - 1;
    tm.tm_mday  = day;
    tm.tm_hour  = hour;
    tm.tm_min   = min;
    tm.tm_sec   = sec;
    tm.tm_isdst = -1;
    return mktime(&tm);
}

void start() {
    const ct_time_t now = make_test_time(2020, 1, 1, 0, 0, 0);
    ct_atomic_long_store(&g_env.realtime, (long)now);
    ct_atomic_long_store(&g_env.monotonic, 0);
    ct_atomic_int_store(&g_env.manager_stopped, 0);
    g_env.started = false;

    ct_cron_mgr_init(mock_realtime_ms, mock_monotonic_ms);

    REQUIRE(ct_thread_create(&g_env.manager_thread, nullptr, cron_thread_run, &g_env) == 0);

    ct_cron_init(&g_env.wakeup);
    REQUIRE(ct_cron_start(&g_env.wakeup, -1, -1, -1, -1, -1, do_nothing_cb, nullptr) == 0);
    g_env.started = true;
}

void stop() {
    if (!g_env.started) { return; }
    ct_cron_mgr_close();
    REQUIRE(ct_thread_join(g_env.manager_thread, nullptr) == 0);
    REQUIRE(ct_atomic_int_load(&g_env.manager_stopped) == 1);
    g_env.started = false;
}

void advance_seconds(ct_time_t seconds) {
    ct_atomic_long_add(&g_env.realtime, seconds);
    ct_atomic_long_add(&g_env.monotonic, seconds);
    REQUIRE(ct_cron_reset(&g_env.wakeup, -1, -1, -1, -1, -1) == 0);
}

void advance_seconds_skew(ct_time_t r, ct_time_t m) {
    ct_atomic_long_add(&g_env.realtime, r);
    ct_atomic_long_add(&g_env.monotonic, m);
    REQUIRE(ct_cron_reset(&g_env.wakeup, -1, -1, -1, -1, -1) == 0);
}

}  // namespace

TEST_CASE("minutely cron fires at correct interval with mock time", "[cron]") {
    start();

    callback_ctx ctx;
    ct_cron_t    cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

    advance_seconds(59);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);
    REQUIRE_FALSE(ctx.wait_timeout(50));

    advance_seconds(1);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    advance_seconds(60);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 2);

    REQUIRE(ct_cron_stop(&cron) == 0);
    stop();
}

TEST_CASE("stopping cron prevents future executions", "[cron]") {
    start();

    callback_ctx ctx;
    ct_cron_t    cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

    advance_seconds(60);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) >= 1);

    REQUIRE(ct_cron_stop(&cron) == 0);

    advance_seconds(120);
    REQUIRE_FALSE(ctx.wait_timeout(100));

    stop();
}

TEST_CASE("time jump reschedules without catchup burst", "[cron]") {
    start();

    callback_ctx ctx;
    ct_cron_t    cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_start(&cron, 0, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

    advance_seconds(1800);
    REQUIRE_FALSE(ctx.wait_timeout(100));

    advance_seconds_skew(7200, 0);
    REQUIRE_FALSE(ctx.wait_timeout(100));

    advance_seconds(1800);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    REQUIRE(ct_cron_stop(&cron) == 0);
    stop();
}

TEST_CASE("multiple tasks due at same time can fire together", "[cron]") {
    start();

    callback_ctx minute_ctx;
    callback_ctx hour_ctx;
    ct_cron_t    minute_cron;
    ct_cron_t    hour_cron;
    ct_cron_init(&minute_cron);
    ct_cron_init(&hour_cron);

    REQUIRE(ct_cron_start(&minute_cron, -1, -1, -1, -1, -1, event_count_cb, &minute_ctx) == 0);
    REQUIRE(ct_cron_start(&hour_cron, 0, -1, -1, -1, -1, event_count_cb, &hour_ctx) == 0);

    advance_seconds(60);
    minute_ctx.wait();
    REQUIRE(ct_atomic_int_load(&minute_ctx.count) >= 1);
    REQUIRE(ct_atomic_int_load(&hour_ctx.count) == 0);

    advance_seconds(3540);
    minute_ctx.wait();
    hour_ctx.wait();
    REQUIRE(ct_atomic_int_load(&minute_ctx.count) >= 2);
    REQUIRE(ct_atomic_int_load(&hour_ctx.count) >= 1);

    REQUIRE(ct_cron_stop(&minute_cron) == 0);
    REQUIRE(ct_cron_stop(&hour_cron) == 0);
    stop();
}

TEST_CASE("cron functions reject null pointer arguments", "[cron]") {
    ct_cron_init(nullptr);
    REQUIRE(ct_cron_start(nullptr, -1, -1, -1, -1, -1, do_nothing_cb, nullptr) == -1);
    REQUIRE(ct_cron_reset(nullptr, -1, -1, -1, -1, -1) == -1);
    REQUIRE(ct_cron_stop(nullptr) == -1);
}

TEST_CASE("starting a cron with null callback returns error", "[cron]") {
    ct_cron_t cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, nullptr, nullptr) == -1);
}

TEST_CASE("stopping an unstarted cron returns error", "[cron]") {
    ct_cron_t cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_stop(&cron) == -1);
}

TEST_CASE("resetting a cron that was never started returns error", "[cron]") {
    ct_cron_t cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_reset(&cron, -1, -1, -1, -1, -1) == -1);
}

TEST_CASE("ct_cron_stop returns error when manager is not running", "[cron]") {
    ct_cron_t cron;
    ct_cron_init(&cron);
    start();
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, do_nothing_cb, nullptr) == 0);
    stop();
    REQUIRE(ct_cron_stop(&cron) == -1);
}

TEST_CASE("starting a cron twice replaces its schedule", "[cron]") {
    start();

    callback_ctx ctx;
    ct_cron_t    cron;
    ct_cron_init(&cron);

    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

    advance_seconds(30);
    REQUIRE_FALSE(ctx.wait_timeout(50));

    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

    advance_seconds(30);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    REQUIRE(ct_cron_stop(&cron) == 0);
    stop();
}

TEST_CASE("cron callback receives the correct user argument", "[cron]") {
    start();

    arg_holder holder;
    ct_event_init(&holder.event);
    holder.captured = nullptr;

    ct_cron_t cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, verify_arg_cb, &holder) == 0);

    advance_seconds(60);
    REQUIRE(ct_event_wait(&holder.event) == 0);
    REQUIRE(holder.captured == &holder);

    ct_cron_stop(&cron);
    ct_event_destroy(&holder.event);
    stop();
}

TEST_CASE("minutely cron remains stable over many consecutive ticks", "[cron]") {
    start();

    callback_ctx ctx;
    ct_cron_t    cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

    for (int i = 0; i < 5; ++i) {
        advance_seconds(60);
        ctx.wait();
    }
    REQUIRE(ct_atomic_int_load(&ctx.count) >= 5);

    REQUIRE(ct_cron_stop(&cron) == 0);
    stop();
}
