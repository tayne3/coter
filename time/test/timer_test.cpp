#include "coter/time/timer.h"

#include <atomic>
#include <thread>

#include "coter/core/time.h"
#include "coter/sync/event.h"
#include "coter/testing/doctest.h"
#include "coter/testing/fff.h"
#include "internal.h"
#include "time_mock.h"

namespace {

struct callback_ctx {
    ct_event_t       event;
    std::atomic<int> count{0};

    callback_ctx() {
        ct_event_init(&event);
        count.store(0);
    }

    ~callback_ctx() { ct_event_destroy(&event); }

    void wait() { REQUIRE(ct_event_timedwait(&event, 2000) == 0); }

    bool wait_timeout(uint32_t ms) { return ct_event_timedwait(&event, ms) == 0; }

    int get_count() const { return count.load(); }
};

void event_count_cb(void* arg) {
    callback_ctx* ctx = (callback_ctx*)arg;
    ctx->count.fetch_add(1);
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

struct TimerFixture {
    ct_timer_t wakeup;
    ct_event_t wakeup_event;

    callback_ctx ctx;
    callback_ctx ctx1;
    callback_ctx ctx2;
    ct_timer_t   timer;
    ct_timer_t   timer1;
    ct_timer_t   timer2;
    ct_ticker_t  ticker;
    arg_holder   holder;

    TimerFixture() {
        RESET_FAKE(mock_ct_gettimeofday_ms);
        RESET_FAKE(mock_ct_getuptime_ms);

        mock_ct_gettimeofday_ms_fake.custom_fake = mock_realtime_ms;
        mock_ct_getuptime_ms_fake.custom_fake    = mock_monotonic_ms;
        mock_time_reset();
    }

    ~TimerFixture() {
        ct_timer_mgr_shutdown();
        ct_timer_mgr_process_once();
    }

    void advance_ms(ct_time64_t ms) {
        mock_time_advance(ms);
        ct_timer_mgr_process_once();
    }
};

}  // namespace

TEST_SUITE_BEGIN("timer");

TEST_CASE("timer API error handling (uninitialized manager)") {
    SUBCASE("ct_set_timeout returns error for null callback") {
        REQUIRE(ct_set_timeout(100, nullptr, nullptr) == -1);
    }
    SUBCASE("stopping an unstarted timer returns error") {
        ct_timer_t t1 = CT_TIMER_INITIALIZER;
        REQUIRE(ct_timer_stop(&t1) == -1);

        ct_timer_t t2;
        ct_timer_init(&t2);
        REQUIRE(ct_timer_stop(&t2) == -1);
    }

    SUBCASE("timer functions reject null pointer arguments") {
        ct_timer_init(nullptr);
        REQUIRE(ct_timer_start(nullptr, 100, event_count_cb, nullptr) == -1);
        REQUIRE(ct_timer_reset(nullptr, 100) == -1);
        REQUIRE(ct_timer_stop(nullptr) == -1);
    }

    SUBCASE("starting a timer with null callback returns error") {
        ct_timer_t timer;
        ct_timer_init(&timer);
        REQUIRE(ct_timer_start(&timer, 100, nullptr, nullptr) == -1);
    }

    SUBCASE("resetting a timer that was never started returns error") {
        ct_timer_t timer;
        ct_timer_init(&timer);
        REQUIRE(ct_timer_reset(&timer, 100) == -1);
    }
}

TEST_CASE_FIXTURE(TimerFixture, "timer core behavior") {
    ct_timer_init(&timer);

    SUBCASE("one-shot timer fires exactly once") {
        REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

        advance_ms(60);
        REQUIRE(ctx.get_count() == 0);

        advance_ms(60);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        advance_ms(300);
        REQUIRE(ctx.get_count() == 1);
    }

    SUBCASE("resetting a timer restarts its countdown") {
        REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

        advance_ms(60);
        REQUIRE(ctx.get_count() == 0);

        REQUIRE(ct_timer_reset(&timer, 100) == 0);

        advance_ms(60);
        REQUIRE(ctx.get_count() == 0);

        advance_ms(60);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);
    }

    SUBCASE("starting a timer twice resets its deadline") {
        REQUIRE(ct_timer_start(&timer, 200, event_count_cb, &ctx) == 0);

        advance_ms(100);
        REQUIRE(ctx.get_count() == 0);

        REQUIRE(ct_timer_start(&timer, 200, event_count_cb, &ctx) == 0);

        advance_ms(150);
        REQUIRE(ctx.get_count() == 0);

        advance_ms(100);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);
    }

    SUBCASE("zero timeout fires immediately on next tick") {
        REQUIRE(ct_timer_start(&timer, 0, event_count_cb, &ctx) == 0);
        advance_ms(0);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);
    }

    SUBCASE("ct_set_timeout fires once after the specified delay") {
        REQUIRE(ct_set_timeout(100, event_count_cb, &ctx) == 0);

        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        advance_ms(110);
        REQUIRE(ctx.get_count() == 1);
    }

    SUBCASE("stopping a timer before expiry prevents its callback") {
        REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

        advance_ms(50);
        REQUIRE(ctx.get_count() == 0);

        REQUIRE(ct_timer_stop(&timer) == 0);

        advance_ms(100);
        REQUIRE(ctx.get_count() == 0);
    }

    SUBCASE("stopping a timer that already fired returns error") {
        REQUIRE(ct_timer_start(&timer, 100, event_count_cb, &ctx) == 0);

        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        REQUIRE(ct_timer_stop(&timer) == -1);
    }

    SUBCASE("timer callback receives the correct user argument") {
        ct_event_init(&holder.event);
        holder.captured = nullptr;

        REQUIRE(ct_timer_start(&timer, 100, verify_arg_cb, &holder) == 0);

        advance_ms(110);
        REQUIRE(ct_event_timedwait(&holder.event, 2000) == 0);
        REQUIRE(holder.captured == &holder);

        ct_event_destroy(&holder.event);
    }
}

TEST_CASE_FIXTURE(TimerFixture, "multiple timers") {
    ct_timer_init(&timer1);
    ct_timer_init(&timer2);

    SUBCASE("fire independently at different deadlines") {
        REQUIRE(ct_timer_start(&timer1, 100, event_count_cb, &ctx1) == 0);
        REQUIRE(ct_timer_start(&timer2, 200, event_count_cb, &ctx2) == 0);

        advance_ms(150);
        ctx1.wait();
        REQUIRE(ctx1.get_count() == 1);
        REQUIRE(ctx2.get_count() == 0);

        advance_ms(110);
        ctx2.wait();
        REQUIRE(ctx1.get_count() == 1);
        REQUIRE(ctx2.get_count() == 1);
    }

    SUBCASE("manager cleans up pending timers on close") {
        REQUIRE(ct_timer_start(&timer1, 100, event_count_cb, &ctx1) == 0);
        REQUIRE(ct_timer_start(&timer2, 10000, event_count_cb, &ctx1) == 0);

        advance_ms(110);
        ctx1.wait();
        REQUIRE(ctx1.get_count() == 1);
    }
}

TEST_SUITE_END();

// =========================================================
// Ticker Suite
// =========================================================
TEST_SUITE_BEGIN("ticker");

TEST_CASE("ticker API error handling") {
    SUBCASE("ticker functions reject null pointer arguments") {
        ct_ticker_init(nullptr);
        REQUIRE(ct_ticker_start(nullptr, 100, event_count_cb, nullptr) == -1);
        REQUIRE(ct_ticker_reset(nullptr, 100) == -1);
        REQUIRE(ct_ticker_stop(nullptr) == -1);
    }

    SUBCASE("starting a ticker with null callback returns error") {
        ct_ticker_t ticker;
        ct_ticker_init(&ticker);
        REQUIRE(ct_ticker_start(&ticker, 100, nullptr, nullptr) == -1);
    }

    SUBCASE("resetting a ticker that was never started returns error") {
        ct_ticker_t ticker;
        ct_ticker_init(&ticker);
        REQUIRE(ct_ticker_reset(&ticker, 100) == -1);
    }
}

TEST_CASE_FIXTURE(TimerFixture, "ticker core behavior") {
    ct_ticker_init(&ticker);

    SUBCASE("periodic ticker fires repeatedly at fixed intervals") {
        REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 2);

        advance_ms(110);
        ctx.wait();
        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 4);

        REQUIRE(ct_ticker_stop(&ticker) == 0);
    }

    SUBCASE("stopping a ticker prevents future callbacks") {
        REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        REQUIRE(ct_ticker_stop(&ticker) == 0);

        advance_ms(300);
        REQUIRE(ctx.get_count() == 1);
    }

    SUBCASE("resetting a ticker changes its callback interval") {
        REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        REQUIRE(ct_ticker_reset(&ticker, 200) == 0);

        advance_ms(150);
        REQUIRE(ctx.get_count() == 1);

        advance_ms(100);
        ctx.wait();
        REQUIRE(ctx.get_count() == 2);

        REQUIRE(ct_ticker_stop(&ticker) == 0);
    }

    SUBCASE("stopped ticker can be restarted with a new interval") {
        REQUIRE(ct_ticker_stop(&ticker) == -1);  // Unstarted ticker stop

        REQUIRE(ct_ticker_start(&ticker, 100, event_count_cb, &ctx) == 0);

        advance_ms(110);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        REQUIRE(ct_ticker_stop(&ticker) == 0);

        REQUIRE(ct_ticker_start(&ticker, 150, event_count_cb, &ctx) == 0);

        advance_ms(160);
        ctx.wait();
        REQUIRE(ctx.get_count() == 2);

        REQUIRE(ct_ticker_stop(&ticker) == 0);
    }

    SUBCASE("resetting a ticker to a shorter interval reschedules correctly") {
        REQUIRE(ct_ticker_start(&ticker, 200, event_count_cb, &ctx) == 0);

        advance_ms(110);
        REQUIRE(ctx.get_count() == 0);

        REQUIRE(ct_ticker_reset(&ticker, 50) == 0);

        advance_ms(60);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        advance_ms(60);
        ctx.wait();
        REQUIRE(ctx.get_count() == 2);

        REQUIRE(ct_ticker_stop(&ticker) == 0);
    }

    SUBCASE("starting a ticker twice restarts its interval") {
        REQUIRE(ct_ticker_start(&ticker, 200, event_count_cb, &ctx) == 0);

        advance_ms(100);
        REQUIRE(ctx.get_count() == 0);

        REQUIRE(ct_ticker_start(&ticker, 200, event_count_cb, &ctx) == 0);

        advance_ms(150);
        REQUIRE(ctx.get_count() == 0);

        advance_ms(100);
        ctx.wait();
        REQUIRE(ctx.get_count() == 1);

        REQUIRE(ct_ticker_stop(&ticker) == 0);
    }

    SUBCASE("ticker remains stable over many consecutive ticks") {
        REQUIRE(ct_ticker_start(&ticker, 50, event_count_cb, &ctx) == 0);

        for (int i = 0; i < 10; ++i) {
            advance_ms(55);
            ctx.wait();
        }
        REQUIRE(ctx.get_count() >= 10);

        REQUIRE(ct_ticker_stop(&ticker) == 0);
    }
}

TEST_SUITE_END();
