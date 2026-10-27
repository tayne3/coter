/**
 * @file shutdown_signal.c
 * @brief Implementation of ct_timeout_signal_t and ct_shutdown_token_t.
 */
#include "coter/sync/shutdown_signal.h"

void ct_timeout_signal_init(ct_timeout_signal_t* token, ct_time64_t timeout_ms) {
    if (!token) { return; }
    ct_atomic_int_store(&token->cancelled_, 0);
    if (timeout_ms < 0) {
        /* deadline_ms_ < 0 is the "no deadline" sentinel. */
        ct_atomic_llong_store(&token->deadline_ms_, -1);
    } else {
        ct_atomic_llong_store(&token->deadline_ms_, (long long)(ct_gettick_ms() + timeout_ms));
    }
}

bool ct_timeout_signal_cancel(ct_timeout_signal_t* token) {
    if (!token) { return false; }
    /* Swap 0 → 1; returns true only for the thread that did the swap. */
    return ct_atomic_int_exchange(&token->cancelled_, 1) == 0;
}

bool ct_timeout_signal_is_done(const ct_timeout_signal_t* token) {
    if (!token) { return false; }
    if (ct_atomic_int_load(&token->cancelled_)) { return true; }
    ct_time64_t dl = (ct_time64_t)ct_atomic_llong_load(&token->deadline_ms_);
    /* dl < 0: no deadline.  dl >= 0: done when (now - dl) >= 0. */
    if (dl >= 0) { return (ct_time64_t)(ct_gettick_ms() - dl) >= 0; }
    return false;
}

ct_time64_t ct_timeout_signal_remaining(const ct_timeout_signal_t* token) {
    if (!token) { return -1; }
    if (ct_atomic_int_load(&token->cancelled_)) { return 0; }
    ct_time64_t dl = (ct_time64_t)ct_atomic_llong_load(&token->deadline_ms_);
    if (dl < 0) { return -1; } /* No deadline. */

    ct_time64_t diff = (ct_time64_t)(dl - ct_gettick_ms());
    if (diff <= 0) { return 0; }
    return diff;
}

int ct_shutdown_token_init(ct_shutdown_token_t* ctx, ct_time64_t timeout_ms) {
    if (!ctx) { return -1; }
    if (ct_mutex_init(&ctx->mutex_) != 0) { return -1; }
    if (ct_cond_init(&ctx->cond_) != 0) {
        ct_mutex_destroy(&ctx->mutex_);
        return -1;
    }
    ct_timeout_signal_init(&ctx->token_, timeout_ms);
    return 0;
}

void ct_shutdown_token_destroy(ct_shutdown_token_t* ctx) {
    if (!ctx) { return; }
    ct_mutex_destroy(&ctx->mutex_);
    ct_cond_destroy(&ctx->cond_);
}

bool ct_shutdown_token_cancel(ct_shutdown_token_t* ctx) {
    if (!ctx) { return false; }

    bool first = ct_timeout_signal_cancel(&ctx->token_);
    if (first) {
        /* Hold the mutex across broadcast to ensure no waiter misses the wakeup
           between its is_done() check and its cond_wait entry. */
        ct_mutex_lock(&ctx->mutex_);
        ct_cond_broadcast(&ctx->cond_);
        ct_mutex_unlock(&ctx->mutex_);
    }
    return first;
}

bool ct_shutdown_token_wait(ct_shutdown_token_t* ctx, ct_time64_t timeout_ms) {
    if (!ctx) { return false; }

    ct_time64_t user_deadline = 0;
    if (timeout_ms >= 0) { user_deadline = ct_gettick_ms() + timeout_ms; }

    ct_mutex_lock(&ctx->mutex_);

    while (!ct_shutdown_token_is_done(ctx)) {
        ct_time64_t remain = ct_shutdown_token_remaining(ctx);
        /* Guard against the race where is_done() returned false at the loop
           top but the token deadline crossed before remaining() was read. */
        if (remain == 0) { break; }

        ct_time64_t user_remain = -1;
        if (timeout_ms >= 0) {
            user_remain = (ct_time64_t)(user_deadline - ct_gettick_ms());
            if (user_remain <= 0) { break; }
        }

        /* to_wait is the minimum of the two active deadlines.
           If neither deadline is set (remain < 0 && user_remain < 0),
           to_wait is -1 and we wait indefinitely for a broadcast. */
        ct_time64_t to_wait;
        if (remain > 0 && user_remain < 0) {
            to_wait = remain;
        } else if (remain < 0 && user_remain > 0) {
            to_wait = user_remain;
        } else if (remain > 0 && user_remain > 0) {
            to_wait = CT_MIN(remain, user_remain);
        } else {
            to_wait = -1;
        }

        (void)ct_cond_wait_for(&ctx->cond_, &ctx->mutex_, to_wait);
    }

    const bool done = ct_shutdown_token_is_done(ctx);
    ct_mutex_unlock(&ctx->mutex_);

    return done;
}
