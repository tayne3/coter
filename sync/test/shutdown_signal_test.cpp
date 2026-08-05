/**
 * @file shutdown_signal_test.cpp
 * @brief Tests for C API of shutdown_signal.
 */
#include "coter/sync/shutdown_signal.h"

#include <chrono>
#include <thread>
#include <vector>

#include "coter/testing/doctest.h"

namespace {
using ms = std::chrono::milliseconds;

static int64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

TEST_SUITE_BEGIN("shutdown_signal");

TEST_CASE("timeout_signal: default (no-expiry) is not done") {
    ct_timeout_signal_t sig;
    ct_timeout_signal_init(&sig, -1);
    REQUIRE_FALSE(ct_timeout_signal_is_done(&sig));
    REQUIRE(ct_timeout_signal_remaining(&sig) == -1);
}

TEST_CASE("timeout_signal: zero-ms timeout is immediately done") {
    ct_timeout_signal_t sig;
    ct_timeout_signal_init(&sig, 0);
    std::this_thread::sleep_for(ms(1));
    REQUIRE(ct_timeout_signal_is_done(&sig));
    REQUIRE(ct_timeout_signal_remaining(&sig) == 0);
}

TEST_CASE("timeout_signal: positive timeout not yet done") {
    ct_timeout_signal_t sig;
    ct_timeout_signal_init(&sig, 5000);
    REQUIRE_FALSE(ct_timeout_signal_is_done(&sig));
    ct_time64_t rem = ct_timeout_signal_remaining(&sig);
    REQUIRE(rem > 0);
    REQUIRE(rem <= 5000);
}

TEST_CASE("timeout_signal: positive timeout expires") {
    ct_timeout_signal_t sig;
    ct_timeout_signal_init(&sig, 50);
    REQUIRE_FALSE(ct_timeout_signal_is_done(&sig));
    std::this_thread::sleep_for(ms(120));
    REQUIRE(ct_timeout_signal_is_done(&sig));
    REQUIRE(ct_timeout_signal_remaining(&sig) == 0);
}

TEST_CASE("timeout_signal: cancel before deadline") {
    ct_timeout_signal_t sig;
    ct_timeout_signal_init(&sig, 5000);
    REQUIRE_FALSE(ct_timeout_signal_is_done(&sig));

    bool first = ct_timeout_signal_cancel(&sig);
    REQUIRE(first);
    REQUIRE(ct_timeout_signal_is_done(&sig));
    REQUIRE(ct_timeout_signal_remaining(&sig) == 0);

    // Subsequent cancels return false
    bool second = ct_timeout_signal_cancel(&sig);
    REQUIRE_FALSE(second);
}

TEST_CASE("timeout_signal: null pointers") {
    ct_timeout_signal_init(nullptr, -1);  // Should not crash
    REQUIRE_FALSE(ct_timeout_signal_cancel(nullptr));
    REQUIRE_FALSE(ct_timeout_signal_is_done(nullptr));
    REQUIRE(ct_timeout_signal_remaining(nullptr) == -1);
}

TEST_CASE("shutdown_token: basic lifecycle") {
    ct_shutdown_token_t ctx;
    REQUIRE(ct_shutdown_token_init(&ctx, -1) == 0);

    REQUIRE_FALSE(ct_shutdown_token_is_done(&ctx));
    REQUIRE(ct_shutdown_token_remaining(&ctx) == -1);
    REQUIRE(ct_shutdown_token_token(&ctx) == &ctx.token);

    bool first = ct_shutdown_token_cancel(&ctx);
    REQUIRE(first);
    REQUIRE(ct_shutdown_token_is_done(&ctx));

    bool second = ct_shutdown_token_cancel(&ctx);
    REQUIRE_FALSE(second);

    ct_shutdown_token_destroy(&ctx);
}

TEST_CASE("shutdown_token: null pointers") {
    REQUIRE(ct_shutdown_token_init(nullptr, -1) == -1);
    ct_shutdown_token_destroy(nullptr);  // Should not crash
    REQUIRE_FALSE(ct_shutdown_token_cancel(nullptr));
    REQUIRE_FALSE(ct_shutdown_token_is_done(nullptr));
    REQUIRE(ct_shutdown_token_remaining(nullptr) == -1);
    REQUIRE_FALSE(ct_shutdown_token_wait(nullptr, -1));
    REQUIRE(ct_shutdown_token_token(nullptr) == nullptr);
}

TEST_CASE("shutdown_token: wait() returns false on caller timeout") {
    ct_shutdown_token_t ctx;
    REQUIRE(ct_shutdown_token_init(&ctx, -1) == 0);

    int64_t t0   = now_ms();
    bool    done = ct_shutdown_token_wait(&ctx, 50);  // wait at most 50 ms
    int64_t dt   = now_ms() - t0;

    REQUIRE_FALSE(done);
    REQUIRE(dt >= 30);
    REQUIRE(dt < 2000);

    ct_shutdown_token_destroy(&ctx);
}

TEST_CASE("shutdown_token: wait() returns true when token is cancelled externally") {
    ct_shutdown_token_t ctx;
    REQUIRE(ct_shutdown_token_init(&ctx, -1) == 0);

    int64_t t0 = now_ms();

    std::thread closer([&ctx] {
        std::this_thread::sleep_for(ms(50));
        ct_shutdown_token_cancel(&ctx);
    });

    bool    done = ct_shutdown_token_wait(&ctx, -1);
    int64_t dt   = now_ms() - t0;

    closer.join();

    REQUIRE(done);
    REQUIRE(dt >= 30);
    REQUIRE(dt < 2000);

    ct_shutdown_token_destroy(&ctx);
}

TEST_CASE("shutdown_token: multiple concurrent waiters all wake on cancel") {
    ct_shutdown_token_t ctx;
    REQUIRE(ct_shutdown_token_init(&ctx, -1) == 0);

    constexpr int N          = 4;
    bool          results[N] = {};

    std::thread waiters[N];
    for (int i = 0; i < N; ++i) {
        waiters[i] = std::thread([&ctx, &results, i] { results[i] = ct_shutdown_token_wait(&ctx, -1); });
    }

    std::this_thread::sleep_for(ms(20));
    ct_shutdown_token_cancel(&ctx);

    for (auto& t : waiters) { t.join(); }

    for (int i = 0; i < N; ++i) { REQUIRE(results[i]); }

    ct_shutdown_token_destroy(&ctx);
}

TEST_CASE("shutdown_token: wait() returns true when internal deadline expires") {
    ct_shutdown_token_t ctx;
    REQUIRE(ct_shutdown_token_init(&ctx, 80) == 0);  // 80 ms internal deadline

    int64_t t0   = now_ms();
    bool    done = ct_shutdown_token_wait(&ctx, -1);
    int64_t dt   = now_ms() - t0;

    REQUIRE(done);
    REQUIRE(dt >= 50);
    REQUIRE(dt < 2000);

    ct_shutdown_token_destroy(&ctx);
}

TEST_CASE("shutdown_token: multiple concurrent cancel calls") {
    ct_shutdown_token_t ctx;
    REQUIRE(ct_shutdown_token_init(&ctx, -1) == 0);

    constexpr int N          = 8;
    bool          results[N] = {};

    std::thread closers[N];
    for (int i = 0; i < N; ++i) {
        closers[i] = std::thread([&ctx, &results, i]() mutable {
            std::this_thread::sleep_for(ms(10));
            results[i] = ct_shutdown_token_cancel(&ctx);
        });
    }

    for (auto& t : closers) { t.join(); }

    int true_count = 0;
    for (int i = 0; i < N; ++i) {
        if (results[i]) true_count++;
    }
    REQUIRE(true_count == 1);
    REQUIRE(ct_shutdown_token_is_done(&ctx));

    ct_shutdown_token_destroy(&ctx);
}

TEST_CASE("shutdown_token: multi-thread – cancel wakes waiter within 500ms") {
    ct_shutdown_token_t ctx;
    REQUIRE(ct_shutdown_token_init(&ctx, -1) == 0);

    int64_t t_cancel = -1;
    int64_t t_woken  = -1;

    std::thread waiter([&ctx, &t_woken]() {
        ct_shutdown_token_wait(&ctx, -1);
        t_woken = now_ms();
    });

    std::this_thread::sleep_for(ms(40));
    t_cancel = now_ms();
    ct_shutdown_token_cancel(&ctx);

    waiter.join();

    REQUIRE(ct_shutdown_token_is_done(&ctx));
    REQUIRE(t_woken >= t_cancel);
    int64_t latency = t_woken - t_cancel;
    REQUIRE(latency < 500);

    ct_shutdown_token_destroy(&ctx);
}

TEST_SUITE_END();
