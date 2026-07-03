#include "coter/time/cron.h"

#include <atomic>
#include <thread>

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
};

void event_count_cb(void* arg) {
    callback_ctx* ctx = (callback_ctx*)arg;
    ctx->count.fetch_add(1);
    ct_event_signal(&ctx->event);
}

struct CronFixture {
    ct_time64_t current_offs_real{0};

    CronFixture() {
        RESET_FAKE(mock_ct_gettimeofday_ms);
        RESET_FAKE(mock_ct_getuptime_ms);

        mock_ct_gettimeofday_ms_fake.custom_fake = mock_realtime_ms;
        mock_ct_getuptime_ms_fake.custom_fake    = mock_monotonic_ms;

        ct_time64_t fixed_time     = make_time(2023, 1, 1, 12, 0, 0);
        ct_time64_t current_uptime = mock_monotonic_ms();
        current_offs_real          = fixed_time * 1000 - current_uptime;
        mock_time_set_offs_real(current_offs_real);
    }

    ~CronFixture() noexcept {
        ct_cron_mgr_shutdown();
        ct_cron_mgr_process_once();  // Process remaining events
    }

    void advance_seconds(ct_time64_t seconds) {
        for (ct_time64_t i = 0; i < seconds; ++i) {
            mock_time_advance(1000);
            ct_cron_mgr_process_once();
        }
    }

    void advance_seconds_skew(ct_time64_t r, ct_time64_t m) {
        mock_time_advance(m * 1000);
        current_offs_real += (r - m) * 1000;
        mock_time_set_offs_real(current_offs_real);
        ct_cron_mgr_process_once();
    }
};

}  // namespace

TEST_SUITE_BEGIN("cron");

TEST_CASE("cron basic API error handling") {
    ct_cron_t cron;
    ct_cron_init(&cron);

    SUBCASE("functions reject null pointer arguments") {
        ct_cron_init(nullptr);
        REQUIRE(ct_cron_start(nullptr, -1, -1, -1, -1, -1, [](void*) {}, nullptr) == -1);
        REQUIRE(ct_cron_reset(nullptr, -1, -1, -1, -1, -1) == -1);
        REQUIRE(ct_cron_stop(nullptr) == -1);
    }

    SUBCASE("starting a cron with null callback returns error") {
        REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, nullptr, nullptr) == -1);
    }

    SUBCASE("stopping an unstarted cron returns error") {
        REQUIRE(ct_cron_stop(&cron) == -1);
    }

    SUBCASE("resetting a cron that was never started returns error") {
        REQUIRE(ct_cron_reset(&cron, -1, -1, -1, -1, -1) == -1);
    }
}

TEST_CASE("ct_cron_stop returns error after manager cleanup") {
    ct_cron_t cron;
    ct_cron_init(&cron);

    CronFixture env;
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, [](void*) {}, nullptr) == 0);
    ct_cron_mgr_shutdown();
    ct_cron_mgr_process_once();

    REQUIRE(ct_cron_stop(&cron) == -1);
}

TEST_CASE_FIXTURE(CronFixture, "cron execution behaviors") {
    ct_cron_t cron;
    ct_cron_init(&cron);

    callback_ctx ctx;

    SUBCASE("minutely cron fires at correct interval with mock time") {
        REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

        advance_seconds(59);
        REQUIRE(ctx.count.load() == 0);

        advance_seconds(1);
        ctx.wait();
        REQUIRE(ctx.count.load() == 1);

        advance_seconds(60);
        ctx.wait();
        REQUIRE(ctx.count.load() == 2);

        REQUIRE(ct_cron_stop(&cron) == 0);
    }

    SUBCASE("stopping cron prevents future executions") {
        REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

        advance_seconds(60);
        ctx.wait();
        REQUIRE(ctx.count.load() == 1);  // Exact assertion, not >= 1

        REQUIRE(ct_cron_stop(&cron) == 0);

        advance_seconds(120);
        REQUIRE(ctx.count.load() == 1);  // Ensure it didn't fire again
    }

    SUBCASE("time jump reschedules without catchup burst") {
        REQUIRE(ct_cron_start(&cron, 0, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

        advance_seconds(1800);
        REQUIRE(ctx.count.load() == 0);

        advance_seconds_skew(7200, 0);
        REQUIRE(ctx.count.load() == 0);

        advance_seconds(1800);
        ctx.wait();
        REQUIRE(ctx.count.load() == 1);

        REQUIRE(ct_cron_stop(&cron) == 0);
    }

    SUBCASE("starting a cron twice replaces its schedule") {
        REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

        advance_seconds(30);
        REQUIRE(ctx.count.load() == 0);

        REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

        advance_seconds(30);
        ctx.wait();
        REQUIRE(ctx.count.load() == 1);

        REQUIRE(ct_cron_stop(&cron) == 0);
    }

    SUBCASE("minutely cron remains stable over many consecutive ticks") {
        REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

        for (int i = 0; i < 5; ++i) {
            advance_seconds(60);
            ctx.wait();
        }
        REQUIRE(ctx.count.load() == 5);

        REQUIRE(ct_cron_stop(&cron) == 0);
    }

    SUBCASE("cron callback receives the correct user argument") {
        struct arg_holder {
            void*      captured;
            ct_event_t event;
        } holder;
        holder.captured = nullptr;
        ct_event_init(&holder.event);

        REQUIRE(ct_cron_start(
                    &cron, -1, -1, -1, -1, -1,
                    [](void* arg) {
                        arg_holder* h = (arg_holder*)arg;
                        h->captured   = arg;
                        ct_event_signal(&h->event);
                    },
                    &holder) == 0);

        advance_seconds(60);
        REQUIRE(ct_event_timedwait(&holder.event, 2000) == 0);
        REQUIRE(holder.captured == &holder);

        ct_cron_stop(&cron);
        ct_event_destroy(&holder.event);
    }
}

TEST_CASE_FIXTURE(CronFixture, "multiple tasks due at same time can fire together") {
    callback_ctx ctx;

    ct_cron_t cron;
    ct_cron_init(&cron);
    REQUIRE(ct_cron_start(&cron, -1, -1, -1, -1, -1, event_count_cb, &ctx) == 0);

    callback_ctx hour_ctx;

    ct_cron_t hour_cron;
    ct_cron_init(&hour_cron);
    REQUIRE(ct_cron_start(&hour_cron, 0, -1, -1, -1, -1, event_count_cb, &hour_ctx) == 0);

    advance_seconds(60);
    ctx.wait();
    REQUIRE(ctx.count.load() == 1);
    REQUIRE(hour_ctx.count.load() == 0);

    advance_seconds(3540);
    ctx.wait();
    hour_ctx.wait();
    REQUIRE(ctx.count.load() == 60);  // It advanced by 60 minutes overall, minutely fired 60 times total
    REQUIRE(hour_ctx.count.load() == 1);

    REQUIRE(ct_cron_stop(&cron) == 0);
    REQUIRE(ct_cron_stop(&hour_cron) == 0);
}

TEST_SUITE_END();
