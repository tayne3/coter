/**
 * @file shutdown_signal.h
 * @brief Cancellation and shutdown primitives.
 *
 * ct_timeout_signal_t  — lock-free cancel/deadline token; poll-only, no blocking.
 * ct_shutdown_token_t  — wraps the above with a mutex + condvar for blocking waits.
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

typedef struct ct_timeout_signal {
    ct_atomic_llong_t deadline_ms; /**< Absolute uptime deadline (ms); <0 = no deadline. */
    ct_atomic_int_t   cancelled;   /**< Non-zero once cancelled. One-way; never cleared. */
} ct_timeout_signal_t;

/**
 * @brief Initialise a timeout signal.
 * @param timeout_ms  <0: no deadline (cancel-only).
 *                     0: expired immediately on init.
 *                    >0: expires after this many ms of uptime.
 */
CT_API void ct_timeout_signal_init(ct_timeout_signal_t* token, ct_time64_t timeout_ms);

/**
 * @brief Cancel the signal.  Idempotent; one-way.
 * @return true if this call triggered the cancellation; false if already done.
 */
CT_API bool ct_timeout_signal_cancel(ct_timeout_signal_t* token);

/**
 * @brief Check whether the signal is done (cancelled or deadline passed).
 * @return true if done; always false for NULL.
 */
CT_API bool ct_timeout_signal_is_done(const ct_timeout_signal_t* token);

/**
 * @brief Milliseconds remaining until the deadline.
 * @return  0  already done.
 *         -1  no deadline was set.
 *         >0  remaining ms.
 */
CT_API ct_time64_t ct_timeout_signal_remaining(const ct_timeout_signal_t* token);

/**
 * Extends ct_timeout_signal_t with a mutex and condvar so threads can block
 * in ct_shutdown_token_wait() until cancelled or expired.
 */
typedef struct ct_shutdown_token {
    ct_timeout_signal_t token;
    ct_mutex_t          mutex;
    ct_cond_t           cond;
} ct_shutdown_token_t;

/**
 * @brief Init the shutdown token.
 * @return 0 on success, -1 if mutex or condvar init fails (partial rollback is handled).
 */
CT_API int ct_shutdown_token_init(ct_shutdown_token_t* ctx, ct_time64_t timeout_ms);

/**
 * @brief Destroy mutex and condvar.
 * @note Caller must ensure no threads are blocked in ct_shutdown_token_wait().
 */
CT_API void ct_shutdown_token_destroy(ct_shutdown_token_t* ctx);

/**
 * @brief Cancel and wake all threads blocked in ct_shutdown_token_wait().
 * @return true if this was the first cancellation.
 */
CT_API bool ct_shutdown_token_cancel(ct_shutdown_token_t* ctx);

/** Delegates to ct_timeout_signal_is_done(); always false for NULL. */
CT_INLINE bool ct_shutdown_token_is_done(const ct_shutdown_token_t* ctx) {
    if (!ctx) { return false; }
    return ct_timeout_signal_is_done(&ctx->token);
}

/** Delegates to ct_timeout_signal_remaining(); returns -1 for NULL. */
CT_INLINE ct_time64_t ct_shutdown_token_remaining(const ct_shutdown_token_t* ctx) {
    if (!ctx) { return -1; }
    return ct_timeout_signal_remaining(&ctx->token);
}

/**
 * @brief Block until done or timeout_ms elapses.
 * @param timeout_ms  <0: wait forever; >=0: per-call deadline in ms.
 * @return true  if the token is done on return.
 * @return false if the per-call timeout expired before the token was done,
 *               or ctx is NULL.
 */
CT_API bool ct_shutdown_token_wait(ct_shutdown_token_t* ctx, ct_time64_t timeout_ms);

/** Returns a pointer to the inner ct_timeout_signal_t; NULL for NULL ctx. */
CT_INLINE ct_timeout_signal_t* ct_shutdown_token_token(ct_shutdown_token_t* ctx) {
    if (!ctx) { return NULL; }
    return &ctx->token;
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_SHUTDOWN_SIGNAL_H
