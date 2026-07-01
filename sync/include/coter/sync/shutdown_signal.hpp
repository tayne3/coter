/**
 * @file shutdown_signal.hpp
 * @brief C++ wrappers over the C shutdown-signal primitives.
 *
 * coter::timeout_signal   — RAII owner of ct_timeout_signal_t; not copyable, movable.
 * coter::shutdown_signal  — Producer; triggers shutdown.  Copyable (shared state).
 * coter::shutdown_token   — Consumer; observes shutdown.  Freely copyable/movable.
 *
 * Shared state is heap-allocated via std::shared_ptr.
 * All synchronisation is delegated to the C layer; no std::mutex/condvar used here.
 */
#ifndef COTER_SYNC_SHUTDOWN_SIGNAL_HPP
#define COTER_SYNC_SHUTDOWN_SIGNAL_HPP

#ifdef __cplusplus

#include <cassert>
#include <memory>
#include <stdexcept>

#include "coter/sync/shutdown_signal.h"

namespace coter {

// ---------------------------------------------------------------------------
// timeout_signal
//
// RAII wrapper for ct_timeout_signal_t.  Lock-free; no blocking wait.
// Not copyable.  After a move the source becomes a cancel-only, never-expires signal.
// ---------------------------------------------------------------------------
class timeout_signal final {
public:
    /**
     * @param timeout_ms  <0: no deadline.  0: expired on construction.  >0: ms timeout.
     */
    explicit timeout_signal(ct_time64_t timeout_ms = -1) noexcept { ct_timeout_signal_init(&d, timeout_ms); }

    timeout_signal(const timeout_signal&)            = delete;
    timeout_signal& operator=(const timeout_signal&) = delete;

    timeout_signal(timeout_signal&& other) noexcept : d(other.d) { ct_timeout_signal_init(&other.d, -1); }
    timeout_signal& operator=(timeout_signal&& other) noexcept {
        if (this != &other) {
            d = other.d;
            ct_timeout_signal_init(&other.d, -1);
        }
        return *this;
    }

    /** Atomically cancel.  Returns true only on the first call. */
    bool cancel() noexcept { return ct_timeout_signal_cancel(&d); }

    /** True if cancelled or deadline has passed.  Lock-free. */
    bool is_done() const noexcept { return ct_timeout_signal_is_done(&d); }

    /** Ms until deadline: 0 if done, -1 if no deadline. */
    ct_time64_t remaining_ms() const noexcept { return ct_timeout_signal_remaining(&d); }

    /** Raw pointer for C API interop. */
    ct_timeout_signal_t*       c_token() noexcept { return &d; }
    const ct_timeout_signal_t* c_token() const noexcept { return &d; }

private:
    ct_timeout_signal_t d;
};

class shutdown_signal;

// ---------------------------------------------------------------------------
// shutdown_token  (Consumer)
//
// Freely copyable: all copies observe the same underlying state.
// Default-constructed tokens are empty: done() == false, wait() == false.
// ---------------------------------------------------------------------------
class shutdown_token {
private:
    /* Heap-allocated state shared by all token and signal copies. */
    struct state final {
        ct_shutdown_token_t d;

        explicit state(ct_time64_t timeout_ms) {
            if (ct_shutdown_token_init(&d, timeout_ms) != 0) {
                throw std::runtime_error("ct_shutdown_token_init failed: out of OS resources");
            }
        }

        ~state() noexcept { ct_shutdown_token_destroy(&d); }

        state(const state&)            = delete;
        state& operator=(const state&) = delete;
        state(state&&)                 = delete;
        state& operator=(state&&)      = delete;
    };
    std::shared_ptr<state> state_;

    friend class shutdown_signal;
    explicit shutdown_token(std::shared_ptr<state> s) noexcept : state_(std::move(s)) {}

public:
    /** Default-construct an empty token: done() == false, wait() == false. */
    shutdown_token() noexcept = default;

    /** True if associated with a shutdown_signal (non-empty). */
    bool     valid() const noexcept { return state_ != nullptr; }
    bool     operator!() const noexcept { return !valid(); }
    explicit operator bool() const noexcept { return valid(); }

    /** Non-blocking check.  True if shutdown has been triggered. */
    bool done() const noexcept {
        if (!state_) { return false; }
        return ct_shutdown_token_is_done(&state_->d);
    }

    /**
     * Block until done or timeout_ms elapses.
     * @param timeout_ms  <0: wait forever; >=0: max wait in ms.
     * @return true if done on return; false if timed out or token is empty.
     */
    bool wait(ct_time64_t timeout_ms = -1) noexcept {
        if (!state_) { return false; }
        return ct_shutdown_token_wait(&state_->d, timeout_ms);
    }

    /** Raw ct_timeout_signal_t* for C APIs; nullptr for empty tokens. */
    ct_timeout_signal_t* c_signal() noexcept {
        if (!state_) { return nullptr; }
        return ct_shutdown_token_token(&state_->d);
    }
    const ct_timeout_signal_t* c_signal() const noexcept {
        if (!state_) { return nullptr; }
        return ct_shutdown_token_token(&state_->d);
    }
};

// ---------------------------------------------------------------------------
// shutdown_signal  (Producer)
//
// Copyable: all copies share the same shutdown domain and any one can call close().
// Not assignable, not movable: the domain is fixed at construction.
// ---------------------------------------------------------------------------
class shutdown_signal {
private:
    std::shared_ptr<shutdown_token::state> state_;

public:
    /** @param timeout_ms  <0: cancel-only (no auto-expiry); >=0: auto-deadline in ms. */
    explicit shutdown_signal(ct_time64_t timeout_ms = -1)
        : state_(std::make_shared<shutdown_token::state>(timeout_ms)) {}

    shutdown_signal(const shutdown_signal&)            = default;
    shutdown_signal& operator=(const shutdown_signal&) = delete;
    shutdown_signal(shutdown_signal&&)                 = delete;
    shutdown_signal& operator=(shutdown_signal&&)      = delete;

    /** Obtain a consumer token sharing this signal's state. */
    shutdown_token token() const noexcept { return shutdown_token(state_); }

    /**
     * Trigger shutdown and wake all waiters.  Thread-safe.
     * @return true only on the first call; false if already closed.
     */
    bool close() noexcept {
        if (!state_) { return false; }
        return ct_shutdown_token_cancel(&state_->d);
    }

    /** True if shutdown has been triggered by any means. */
    bool closed() const noexcept {
        if (!state_) { return false; }
        return ct_shutdown_token_is_done(&state_->d);
    }

    /** Raw ct_shutdown_token_t* for C APIs.  Valid for the signal's lifetime. */
    ct_shutdown_token_t*       c_token() noexcept { return state_ ? &state_->d : nullptr; }
    const ct_shutdown_token_t* c_token() const noexcept { return state_ ? &state_->d : nullptr; }
};

}  // namespace coter

#endif  // __cplusplus
#endif  // COTER_SYNC_SHUTDOWN_SIGNAL_HPP
