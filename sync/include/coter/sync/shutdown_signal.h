/**
 * @file shutdown_signal.h
 * @brief Cancellation and shutdown primitives.
 */
#ifndef COTER_SYNC_SHUTDOWN_SIGNAL_H
#define COTER_SYNC_SHUTDOWN_SIGNAL_H

#include "coter/core/macro.h"
#include "coter/core/time.h"
#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lock-free cancel/deadline token for polling.
 */
typedef struct ct_timeout_signal {
    ct_atomic_llong_t deadline_ms_; /**< Absolute uptime deadline (ms); <0 = no deadline. */
    ct_atomic_int_t   cancelled_;   /**< Non-zero once cancelled; never cleared. */
} ct_timeout_signal_t;

/** Static initializer for an unarmed signal (no deadline). */
#define CT_TIMEOUT_SIGNAL_INITIALIZER {CT_ATOMIC_VAR_INIT(-1), CT_ATOMIC_VAR_INIT(0)}

/**
 * @brief Initialise a timeout signal.
 * @param timeout_ms <0: no deadline; 0: already expired; >0: expires after this many ms.
 */
CT_API void ct_timeout_signal_init(ct_timeout_signal_t* self, ct_time64_t timeout_ms);

/**
 * @brief Cancel the signal. Idempotent; one-way.
 * @return true if this call cancelled the signal; false if it was already cancelled.
 */
CT_API bool ct_timeout_signal_cancel(ct_timeout_signal_t* self);

/**
 * @return true if cancelled or expired; false for NULL.
 */
CT_API bool ct_timeout_signal_is_done(const ct_timeout_signal_t* self);

/**
 * @return 0 if done; -1 if no deadline; >0 otherwise.
 */
CT_API ct_time64_t ct_timeout_signal_remaining(const ct_timeout_signal_t* self);

typedef void (*ct_shutdown_token_callback_t)(void* arg);

/**
 * @brief Blocking token that wraps ct_timeout_signal_t.
 */
typedef struct ct_shutdown_token {
    ct_timeout_signal_t          signal_;
    ct_mutex_t                   mutex_;
    ct_cond_t                    cond_;
    ct_shutdown_token_callback_t cb_;
    void*                        arg_;
} ct_shutdown_token_t;

/**
 * @brief Initialise a token.
 * @param timeout_ms  <0: no deadline; 0: already expired; >0: expires after this many ms.
 * @param on_cancel  Optional callback invoked once by the first cancellation.
 * @param arg        Context passed to the callback.
 * @return 0 on success, -1 on failure.
 */
CT_API int ct_shutdown_token_init(ct_shutdown_token_t* self, ct_time64_t timeout_ms,
                                  ct_shutdown_token_callback_t on_cancel, void* arg);

/**
 * @brief Destroy the token.
 * @note No thread may be waiting on the token.
 */
CT_API void ct_shutdown_token_destroy(ct_shutdown_token_t* self);

/**
 * @brief Cancel the token and wake all waiters.
 * @return true if this call cancelled the token; false if it was already cancelled.
 */
CT_API bool ct_shutdown_token_cancel(ct_shutdown_token_t* self);

/**
 * @brief Wait until the token is done or the caller timeout expires.
 * @param timeout_ms  <0: wait indefinitely; >=0: per-call timeout in ms.
 * @return true if done; false on caller timeout or NULL.
 */
CT_API bool ct_shutdown_token_wait(ct_shutdown_token_t* self, ct_time64_t timeout_ms);

/** @return true if cancelled or expired; false for NULL. */
CT_INLINE bool ct_shutdown_token_is_done(const ct_shutdown_token_t* self) {
    if (!self) { return false; }
    return ct_timeout_signal_is_done(&self->signal_);
}

/** @return 0 if done; -1 if no deadline or NULL; >0 otherwise. */
CT_INLINE ct_time64_t ct_shutdown_token_remaining(const ct_shutdown_token_t* self) {
    if (!self) { return -1; }
    return ct_timeout_signal_remaining(&self->signal_);
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_SHUTDOWN_SIGNAL_H
