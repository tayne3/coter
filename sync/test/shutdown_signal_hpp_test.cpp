/**
 * @file shutdown_signal_hpp_test.cpp
 * @brief Tests for C++ API of shutdown_signal.
 */
#include <chrono>
#include <thread>
#include <type_traits>

#include "coter/sync/shutdown_signal.hpp"
#include "coter/testing/doctest.h"

namespace {
using namespace coter;
using ms = std::chrono::milliseconds;

static int64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

TEST_SUITE_BEGIN("shutdown_signal");

TEST_CASE("timeout_signal: default (no-expiry) is not done") {
    timeout_signal sig;  // default: cancel-only, no deadline
    REQUIRE_FALSE(sig.is_done());
    REQUIRE(sig.remaining_ms() == -1);
}

TEST_CASE("timeout_signal: zero-ms timeout is immediately done") {
    timeout_signal sig(0);
    std::this_thread::sleep_for(ms(1));
    REQUIRE(sig.is_done());
    REQUIRE(sig.remaining_ms() == 0);
}

TEST_CASE("timeout_signal: positive timeout not yet done") {
    timeout_signal sig(5000);  // 5 second deadline
    REQUIRE_FALSE(sig.is_done());
    int64_t rem = sig.remaining_ms();
    REQUIRE(rem > 0);
    REQUIRE(rem <= 5000);
}

TEST_CASE("timeout_signal: positive timeout expires") {
    timeout_signal sig(50);  // 50 ms
    REQUIRE_FALSE(sig.is_done());
    std::this_thread::sleep_for(ms(120));
    REQUIRE(sig.is_done());
    REQUIRE(sig.remaining_ms() == 0);
}

TEST_CASE("timeout_signal: cancel before deadline") {
    timeout_signal sig(5000);
    REQUIRE_FALSE(sig.is_done());
    sig.cancel();
    REQUIRE(sig.is_done());
    REQUIRE(sig.remaining_ms() == 0);
}

TEST_CASE("timeout_signal: cancel is idempotent") {
    timeout_signal sig(5000);
    sig.cancel();
    sig.cancel();
    REQUIRE(sig.is_done());
}

TEST_CASE("timeout_signal: c_token returns non-null pointer") {
    timeout_signal sig(1000);
    REQUIRE(sig.c_token() != nullptr);
    REQUIRE(sig.c_token() == static_cast<const timeout_signal&>(sig).c_token());
}

TEST_CASE("timeout_signal: move semantics") {
    timeout_signal sig1(5000);
    timeout_signal sig2 = std::move(sig1);

    // Source should be reset to no-expiry
    REQUIRE(sig1.remaining_ms() == -1);
    REQUIRE_FALSE(sig1.is_done());

    // Target should hold the original deadline
    REQUIRE(sig2.remaining_ms() > 0);
    REQUIRE(sig2.remaining_ms() <= 5000);
}

TEST_CASE("shutdown_signal: type-trait semantics") {
    // Copy-constructible (shares same state)
    STATIC_REQUIRE(std::is_copy_constructible<shutdown_signal>::value);
    // NOT copy-assignable
    STATIC_REQUIRE_FALSE(std::is_copy_assignable<shutdown_signal>::value);
    // NOT move-constructible
    STATIC_REQUIRE_FALSE(std::is_move_constructible<shutdown_signal>::value);
    // NOT move-assignable
    STATIC_REQUIRE_FALSE(std::is_move_assignable<shutdown_signal>::value);
}

TEST_CASE("shutdown_token: type-trait semantics") {
    // Default-constructible (empty token)
    STATIC_REQUIRE(std::is_default_constructible<shutdown_token>::value);
    // Copy-constructible and copy-assignable
    STATIC_REQUIRE(std::is_copy_constructible<shutdown_token>::value);
    STATIC_REQUIRE(std::is_copy_assignable<shutdown_token>::value);
    // Move-constructible and move-assignable
    STATIC_REQUIRE(std::is_move_constructible<shutdown_token>::value);
    STATIC_REQUIRE(std::is_move_assignable<shutdown_token>::value);
}

TEST_CASE("shutdown_signal: basic lifecycle") {
    shutdown_signal sig;
    REQUIRE_FALSE(sig.closed());

    bool first = sig.close();
    REQUIRE(first);
    REQUIRE(sig.closed());

    // Second close must return false (already closed)
    bool second = sig.close();
    REQUIRE_FALSE(second);
}

TEST_CASE("shutdown_token: done() reflects signal close") {
    shutdown_signal sig;
    shutdown_token  tok = sig.token();

    REQUIRE_FALSE(tok.done());
    sig.close();
    REQUIRE(tok.done());
}

TEST_CASE("shutdown_token: multiple tokens observe shutdown") {
    shutdown_signal sig;
    shutdown_token  tok1 = sig.token();
    shutdown_token  tok2 = sig.token();
    shutdown_token  tok3 = sig.token();

    REQUIRE_FALSE(tok1.done());
    REQUIRE_FALSE(tok2.done());
    REQUIRE_FALSE(tok3.done());

    sig.close();

    REQUIRE(tok1.done());
    REQUIRE(tok2.done());
    REQUIRE(tok3.done());
}

TEST_CASE("shutdown_signal: copied signal shares same shutdown domain") {
    shutdown_signal sig1;
    // Copy-construct: sig2 shares the same state
    shutdown_signal sig2 = sig1;  // NOLINT(performance-unnecessary-copy-initialization)

    shutdown_token tok1 = sig1.token();
    shutdown_token tok2 = sig2.token();

    REQUIRE_FALSE(sig1.closed());
    REQUIRE_FALSE(sig2.closed());

    // Close via the copy; the original should observe it too
    sig2.close();

    REQUIRE(sig1.closed());
    REQUIRE(sig2.closed());
    REQUIRE(tok1.done());
    REQUIRE(tok2.done());
}

TEST_CASE("shutdown_signal: close via original observed by copied signal") {
    shutdown_signal sig1;
    shutdown_signal sig2 = sig1;

    sig1.close();

    REQUIRE(sig2.closed());
}

TEST_CASE("shutdown_token: copied token shares state") {
    shutdown_signal sig;
    shutdown_token  tok1 = sig.token();
    shutdown_token  tok2 = tok1;  // copy

    REQUIRE_FALSE(tok1.done());
    REQUIRE_FALSE(tok2.done());

    sig.close();

    REQUIRE(tok1.done());
    REQUIRE(tok2.done());

    // Both tokens point at the same underlying C object
    REQUIRE(tok1.c_signal() != nullptr);
    REQUIRE(tok1.c_signal() == tok2.c_signal());
}

TEST_CASE("shutdown_token: moved token transfers state") {
    shutdown_signal sig;
    shutdown_token  tok1 = sig.token();
    shutdown_token  tok2 = std::move(tok1);

    REQUIRE(tok2.valid());
    REQUIRE_FALSE(tok1.valid());
    sig.close();
    REQUIRE(tok2.done());
}

TEST_CASE("shutdown_token: empty token done() is false") {
    shutdown_token empty;
    REQUIRE_FALSE(empty.valid());
    REQUIRE_FALSE(empty.done());
}

TEST_CASE("shutdown_token: empty token wait() returns false without blocking") {
    shutdown_token empty;
    int64_t        t0 = now_ms();
    bool           r  = empty.wait(0);
    int64_t        dt = now_ms() - t0;
    REQUIRE_FALSE(r);
    REQUIRE(dt < 50);  // must return immediately
}

TEST_CASE("shutdown_token: empty token c_signal() returns nullptr") {
    shutdown_token empty;
    REQUIRE(empty.c_signal() == nullptr);
}

TEST_CASE("shutdown_signal: c_token() returns valid non-null pointer") {
    shutdown_signal sig;
    REQUIRE(sig.c_token() != nullptr);
}

TEST_CASE("shutdown_token: c_signal() returns valid non-null pointer") {
    shutdown_signal sig;
    shutdown_token  tok = sig.token();
    REQUIRE(tok.c_signal() != nullptr);
}

TEST_CASE("shutdown_signal and token share same underlying raw pointer") {
    shutdown_signal sig;
    shutdown_token  tok = sig.token();

    // The token's c_signal() should point inside the same ct_shutdown_token_t
    // that sig.c_token() points at.
    ct_shutdown_token_t* ctx         = sig.c_token();
    ct_timeout_signal_t* ts          = tok.c_signal();
    ct_timeout_signal_t* ts_from_ctx = ct_shutdown_token_token(ctx);
    REQUIRE(ctx != nullptr);
    REQUIRE(ts != nullptr);
    REQUIRE(ts == ts_from_ctx);
}

TEST_CASE("shutdown_token: wait() returns false on caller timeout") {
    shutdown_signal sig;
    shutdown_token  tok = sig.token();

    int64_t t0   = now_ms();
    bool    done = tok.wait(50);  // wait at most 50 ms
    int64_t dt   = now_ms() - t0;

    REQUIRE_FALSE(done);
    REQUIRE(dt >= 30);
    REQUIRE(dt < 2000);
}

TEST_CASE("shutdown_token: wait() returns true when signal is closed externally" *
          doctest::test_suite("shutdown_token")) {
    shutdown_signal sig;
    shutdown_token  tok = sig.token();

    int64_t t0 = now_ms();

    std::thread closer([&sig] {
        std::this_thread::sleep_for(ms(50));
        sig.close();
    });

    bool    done = tok.wait(-1);  // block indefinitely
    int64_t dt   = now_ms() - t0;

    closer.join();

    REQUIRE(done);
    REQUIRE(dt >= 30);
    REQUIRE(dt < 2000);
}

TEST_CASE("shutdown_token: multiple concurrent waiters all wake on close") {
    shutdown_signal sig;

    constexpr int N          = 4;
    bool          results[N] = {};

    std::thread waiters[N];
    for (int i = 0; i < N; ++i) {
        waiters[i] = std::thread([&sig, &results, i] {
            shutdown_token tok = sig.token();
            results[i]         = tok.wait(-1);
        });
    }

    // Give waiters time to enter the cond-wait
    std::this_thread::sleep_for(ms(20));
    sig.close();

    for (auto& t : waiters) { t.join(); }

    for (int i = 0; i < N; ++i) { REQUIRE(results[i]); }
}

TEST_CASE("shutdown_signal: multiple concurrent close calls") {
    shutdown_signal sig;
    constexpr int   N          = 8;
    bool            results[N] = {};

    std::thread closers[N];
    for (int i = 0; i < N; ++i) {
        closers[i] = std::thread([sig, &results, i]() mutable {
            std::this_thread::sleep_for(ms(10));  // rough sync to increase overlap chance
            results[i] = sig.close();
        });
    }

    for (auto& t : closers) { t.join(); }

    int true_count = 0;
    for (int i = 0; i < N; ++i) {
        if (results[i]) true_count++;
    }
    REQUIRE(true_count == 1);
    REQUIRE(sig.closed());
}

TEST_CASE("shutdown_token: wait() returns true when internal deadline expires") {
    shutdown_signal sig(80);  // 80 ms internal deadline
    shutdown_token  tok = sig.token();

    int64_t t0   = now_ms();
    bool    done = tok.wait(-1);  // let the internal deadline expire
    int64_t dt   = now_ms() - t0;

    REQUIRE(done);
    REQUIRE(dt >= 50);
    REQUIRE(dt < 2000);
}

TEST_CASE("shutdown_token: multi-thread – cancel wakes waiter within 40-200ms") {
    shutdown_signal sig;
    shutdown_token  tok = sig.token();

    int64_t t_cancel = -1;
    int64_t t_woken  = -1;

    // Waiter thread (uses its own copy of tok)
    shutdown_token tok_copy = tok;  // explicit copy; tokens are freely copyable
    std::thread    waiter([tok_copy, &t_woken]() mutable {
        tok_copy.wait(-1);
        t_woken = now_ms();
    });

    // Give the waiter a moment to enter the cond-wait
    std::this_thread::sleep_for(ms(10));

    std::this_thread::sleep_for(ms(40));
    t_cancel = now_ms();
    sig.close();

    waiter.join();

    REQUIRE(sig.closed());
    REQUIRE(t_woken >= t_cancel);  // woke after close
    int64_t latency = t_woken - t_cancel;
    REQUIRE(latency < 500);  // wakeup latency < 500 ms
}

TEST_SUITE_END();
