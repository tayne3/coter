#include "coter/time/timer.h"

#include <catch.hpp>

#include "coter/core/platform.h"
#include "coter/sync/atomic.h"
#include "coter/sync/event.h"
#include "coter/thread/thread.h"
#include "coter/time/ticker.h"

namespace {

struct test_env {
    ct_atomic_long_t monotonic       = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t  manager_stopped = CT_ATOMIC_VAR_INIT(0);
    ct_thread_t      manager_thread;
    ct_timer_t       wakeup;
    ct_event_t       wakeup_event;
    bool             started = false;

    test_env() {}

    ~test_env() { stop(); }

    void stop() {
        if (!started) { return; }
        ct_timer_mgr_close();
        (void)ct_thread_join(manager_thread, nullptr);
        ct_event_destroy(&wakeup_event);
        started = false;
    }
};

static test_env g_env;

ct_time64_t mock_gettime_ms() {
    return (ct_time64_t)ct_atomic_long_load(&g_env.monotonic);
}

int timer_thread_run(void* arg) {
    test_env* env = (test_env*)arg;
    ct_timer_mgr_run();
    ct_atomic_int_store(&env->manager_stopped, 1);
    return 0;
}

void wakeup_cb(void* arg) {
    ct_event_t* ev = (ct_event_t*)arg;
    ct_event_signal(ev);
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

void start() {
    ct_atomic_long_store(&g_env.monotonic, 0);
    ct_atomic_int_store(&g_env.manager_stopped, 0);
    g_env.started = false;
    ct_event_init(&g_env.wakeup_event);
    ct_timer_mgr_init(mock_gettime_ms);
    REQUIRE(ct_thread_create(&g_env.manager_thread, nullptr, timer_thread_run, &g_env) == 0);
    g_env.started = true;

    ct_timer_init(&g_env.wakeup);
    ct_timer_start(&g_env.wakeup, 1000000, wakeup_cb, &g_env.wakeup_event);
}

void stop() {
    g_env.stop();
    REQUIRE(ct_atomic_int_load(&g_env.manager_stopped) == 1);
}

void advance_ms(ct_time64_t ms) {
    ct_atomic_long_add(&g_env.monotonic, (long)ms);
    ct_event_reset(&g_env.wakeup_event);
    ct_timer_reset(&g_env.wakeup, 0);
    REQUIRE(ct_event_wait(&g_env.wakeup_event) == 0);
}

}  // namespace

TEST_CASE("one-shot timer fires exactly once", "[timer]") {
    start();

    callback_ctx ctx;
    ct_timer_t   timer;
    ct_timer_init(&timer);

    REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

    advance_ms(60);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);
    REQUIRE_FALSE(ctx.wait_timeout(10));

    advance_ms(60);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    advance_ms(300);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    stop();
}

TEST_CASE("resetting a timer restarts its countdown", "[timer]") {
    start();

    callback_ctx ctx;
    ct_timer_t   timer;
    ct_timer_init(&timer);

    REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

    advance_ms(60);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);

    REQUIRE(ct_timer_reset(&timer, 100) == 0);

    advance_ms(60);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);
    REQUIRE_FALSE(ctx.wait_timeout(10));

    advance_ms(60);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    stop();
}

TEST_CASE("periodic ticker fires repeatedly at fixed intervals", "[ticker]") {
    start();

    callback_ctx ctx;
    ct_ticker_t  ticker;
    ct_ticker_init(&ticker);

    REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 2);

    advance_ms(110);
    ctx.wait();
    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 4);

    REQUIRE(ct_ticker_stop(&ticker) == 0);
    stop();
}

TEST_CASE("ct_set_timeout fires once after the specified delay", "[timer]") {
    start();

    callback_ctx ctx;
    REQUIRE(ct_set_timeout(100, event_count_cb, &ctx) == 0);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    advance_ms(110);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    stop();
}

TEST_CASE("multiple timers fire independently at different deadlines", "[timer]") {
    start();

    callback_ctx ctx1;
    callback_ctx ctx2;
    ct_timer_t   timer1;
    ct_timer_t   timer2;
    ct_timer_init(&timer1);
    ct_timer_init(&timer2);

    REQUIRE(ct_timer_start(&timer1, 100, event_count_cb, &ctx1) == 0);
    REQUIRE(ct_timer_start(&timer2, 200, event_count_cb, &ctx2) == 0);

    advance_ms(150);
    ctx1.wait();
    REQUIRE(ct_atomic_int_load(&ctx1.count) == 1);
    REQUIRE(ct_atomic_int_load(&ctx2.count) == 0);

    advance_ms(110);
    ctx2.wait();
    REQUIRE(ct_atomic_int_load(&ctx1.count) == 1);
    REQUIRE(ct_atomic_int_load(&ctx2.count) == 1);

    stop();
}

TEST_CASE("stopping a timer before expiry prevents its callback", "[timer]") {
    start();

    callback_ctx ctx;
    ct_timer_t   timer;
    ct_timer_init(&timer);

    REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

    advance_ms(50);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);
    REQUIRE_FALSE(ctx.wait_timeout(10));

    REQUIRE(ct_timer_stop(&timer) == 0);

    advance_ms(100);
    REQUIRE_FALSE(ctx.wait_timeout(50));
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);

    stop();
}

TEST_CASE("stopping a timer that already fired returns error", "[timer]") {
    start();

    callback_ctx ctx;
    ct_timer_t   timer;
    ct_timer_init(&timer);

    REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    REQUIRE(ct_timer_stop(&timer) == -1);

    stop();
}

TEST_CASE("stopping an unstarted timer returns error", "[timer]") {
    ct_timer_t timer;
    ct_timer_init(&timer);

    REQUIRE(ct_timer_stop(&timer) == -1);
}

TEST_CASE("stopping a ticker prevents future callbacks", "[timer]") {
    start();

    callback_ctx ctx;
    ct_ticker_t  ticker;
    ct_ticker_init(&ticker);

    REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    REQUIRE(ct_ticker_stop(&ticker) == 0);

    advance_ms(300);
    REQUIRE_FALSE(ctx.wait_timeout(50));
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    stop();
}

TEST_CASE("resetting a ticker changes its callback interval", "[timer]") {
    start();

    callback_ctx ctx;
    ct_ticker_t  ticker;
    ct_ticker_init(&ticker);

    REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    REQUIRE(ct_ticker_reset(&ticker, 200) == 0);

    advance_ms(150);
    REQUIRE_FALSE(ctx.wait_timeout(50));
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    advance_ms(100);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 2);

    REQUIRE(ct_ticker_stop(&ticker) == 0);
    stop();
}

TEST_CASE("stopped ticker can be restarted with a new interval", "[timer]") {
    start();

    callback_ctx ctx;
    ct_ticker_t  ticker;
    ct_ticker_init(&ticker);

    REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    REQUIRE(ct_ticker_stop(&ticker) == 0);

    REQUIRE(ct_ticker_start(&ticker, 150, event_count_cb, &ctx) == 0);

    advance_ms(160);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 2);

    REQUIRE(ct_ticker_stop(&ticker) == 0);
    stop();
}

TEST_CASE("timer functions reject null pointer arguments", "[timer]") {
    ct_timer_init(nullptr);
    REQUIRE(ct_timer_start(nullptr, 100, event_count_cb, nullptr) == -1);
    REQUIRE(ct_timer_reset(nullptr, 100) == -1);
    REQUIRE(ct_timer_stop(nullptr) == -1);
}

TEST_CASE("starting a timer with null callback returns error", "[timer]") {
    ct_timer_t timer;
    ct_timer_init(&timer);
    REQUIRE(ct_timer_start(&timer, 100, nullptr, nullptr) == -1);
}

TEST_CASE("resetting a timer that was never started returns error", "[timer]") {
    ct_timer_t timer;
    ct_timer_init(&timer);
    REQUIRE(ct_timer_reset(&timer, 100) == -1);
}

TEST_CASE("ticker functions reject null pointer arguments", "[timer]") {
    ct_ticker_init(nullptr);
    REQUIRE(ct_ticker_start(nullptr, 100, event_count_cb, nullptr) == -1);
    REQUIRE(ct_ticker_reset(nullptr, 100) == -1);
    REQUIRE(ct_ticker_stop(nullptr) == -1);
}

TEST_CASE("starting a ticker with null callback returns error", "[timer]") {
    ct_ticker_t ticker;
    ct_ticker_init(&ticker);
    REQUIRE(ct_ticker_start(&ticker, 100, nullptr, nullptr) == -1);
}

TEST_CASE("resetting a ticker that was never started returns error", "[timer]") {
    ct_ticker_t ticker;
    ct_ticker_init(&ticker);
    REQUIRE(ct_ticker_reset(&ticker, 100) == -1);
}

TEST_CASE("resetting a ticker to a shorter interval reschedules correctly", "[timer]") {
    start();

    callback_ctx ctx;
    ct_ticker_t  ticker;
    ct_ticker_init(&ticker);

    REQUIRE(ct_ticker_start(&ticker, 200, event_count_cb, &ctx) == 0);

    advance_ms(110);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);

    REQUIRE(ct_ticker_reset(&ticker, 50) == 0);

    advance_ms(60);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    advance_ms(60);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 2);

    REQUIRE(ct_ticker_stop(&ticker) == 0);
    stop();
}

TEST_CASE("starting a timer twice resets its deadline", "[timer]") {
    start();

    callback_ctx ctx;
    ct_timer_t   timer;
    ct_timer_init(&timer);

    REQUIRE(ct_timer_start(&timer, 200, event_count_cb, &ctx) == 0);

    advance_ms(100);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);

    REQUIRE(ct_timer_start(&timer, 200, event_count_cb, &ctx) == 0);

    advance_ms(150);
    REQUIRE_FALSE(ctx.wait_timeout(50));
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);

    advance_ms(100);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    stop();
}

TEST_CASE("zero timeout fires immediately on next tick", "[timer]") {
    start();

    callback_ctx ctx;
    ct_timer_t   timer;
    ct_timer_init(&timer);

    REQUIRE(ct_timer_start(&timer, 0, event_count_cb, &ctx) == 0);

    advance_ms(0);
    REQUIRE(ctx.wait_timeout(50));
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    stop();
}

TEST_CASE("starting a ticker twice restarts its interval", "[timer]") {
    start();

    callback_ctx ctx;
    ct_ticker_t  ticker;
    ct_ticker_init(&ticker);

    REQUIRE(ct_ticker_start(&ticker, 200, event_count_cb, &ctx) == 0);

    advance_ms(100);
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);

    REQUIRE(ct_ticker_start(&ticker, 200, event_count_cb, &ctx) == 0);

    advance_ms(150);
    REQUIRE_FALSE(ctx.wait_timeout(50));
    REQUIRE(ct_atomic_int_load(&ctx.count) == 0);

    advance_ms(100);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    REQUIRE(ct_ticker_stop(&ticker) == 0);
    stop();
}

TEST_CASE("ct_set_timeout returns error when manager is not initialized", "[timer]") {
    callback_ctx ctx;
    REQUIRE(ct_set_timeout(100, event_count_cb, &ctx) == -1);
}

TEST_CASE("timer callback receives the correct user argument", "[timer]") {
    start();

    arg_holder holder;
    ct_event_init(&holder.event);
    holder.captured = nullptr;

    ct_timer_t timer;
    ct_timer_init(&timer);
    REQUIRE(ct_timer_start(&timer, 100, verify_arg_cb, &holder) == 0);

    advance_ms(110);
    REQUIRE(ct_event_wait(&holder.event) == 0);
    REQUIRE(holder.captured == &holder);

    ct_event_destroy(&holder.event);
    stop();
}

TEST_CASE("manager cleans up pending timers on close", "[timer]") {
    start();

    callback_ctx ctx;
    ct_timer_t   timer1;
    ct_timer_t   timer2;
    ct_timer_init(&timer1);
    ct_timer_init(&timer2);

    REQUIRE(ct_timer_start(&timer1, 100, event_count_cb, &ctx) == 0);
    REQUIRE(ct_timer_start(&timer2, 10000, event_count_cb, &ctx) == 0);

    advance_ms(110);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) == 1);

    stop();
}

TEST_CASE("ticker remains stable over many consecutive ticks", "[timer]") {
    start();

    callback_ctx ctx;
    ct_ticker_t  ticker;
    ct_ticker_init(&ticker);

    REQUIRE(ct_ticker_start(&ticker, 50, event_count_cb, &ctx) == 0);

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();

    advance_ms(55);
    ctx.wait();
    REQUIRE(ct_atomic_int_load(&ctx.count) >= 10);

    REQUIRE(ct_ticker_stop(&ticker) == 0);
    stop();
}
