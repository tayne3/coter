/**
 * @file shutdown_signal.c
 * @brief Cancellation and shutdown primitives.
 */
#include "coter/sync/shutdown_signal.h"

void ct_timeout_signal_init(ct_timeout_signal_t* self, ct_time64_t timeout_ms) {
    if (!self) { return; }
    ct_atomic_int_store(&self->cancelled_, 0);
    if (timeout_ms < 0) {
        ct_atomic_llong_store(&self->deadline_ms_, -1);
    } else {
        const ct_time64_t now      = ct_gettick_ms();
        const ct_time64_t deadline = timeout_ms > INT64_MAX - now ? INT64_MAX : now + timeout_ms;
        ct_atomic_llong_store(&self->deadline_ms_, (long long)deadline);
    }
}

bool ct_timeout_signal_cancel(ct_timeout_signal_t* self) {
    if (!self) { return false; }
    return ct_atomic_int_exchange(&self->cancelled_, 1) == 0;
}

bool ct_timeout_signal_is_done(const ct_timeout_signal_t* self) {
    if (!self) { return false; }
    if (ct_atomic_int_load(&self->cancelled_)) { return true; }
    ct_time64_t dl = (ct_time64_t)ct_atomic_llong_load(&self->deadline_ms_);
    if (dl >= 0) { return (ct_time64_t)(ct_gettick_ms() - dl) >= 0; }
    return false;
}

ct_time64_t ct_timeout_signal_remaining(const ct_timeout_signal_t* self) {
    if (!self) { return -1; }
    if (ct_atomic_int_load(&self->cancelled_)) { return 0; }
    ct_time64_t dl = (ct_time64_t)ct_atomic_llong_load(&self->deadline_ms_);
    if (dl < 0) { return -1; }

    ct_time64_t diff = (ct_time64_t)(dl - ct_gettick_ms());
    if (diff <= 0) { return 0; }
    return diff;
}

int ct_shutdown_token_init(ct_shutdown_token_t* self, ct_time64_t timeout_ms, ct_shutdown_token_callback_t on_cancel,
                           void* arg) {
    if (!self) { return -1; }
    if (ct_mutex_init(&self->mutex_) != 0) { return -1; }
    if (ct_cond_init(&self->cond_) != 0) {
        ct_mutex_destroy(&self->mutex_);
        return -1;
    }
    self->cb_  = on_cancel;
    self->arg_ = arg;
    ct_timeout_signal_init(&self->signal_, timeout_ms);
    return 0;
}

void ct_shutdown_token_destroy(ct_shutdown_token_t* self) {
    if (!self) { return; }
    ct_mutex_destroy(&self->mutex_);
    ct_cond_destroy(&self->cond_);
}

bool ct_shutdown_token_cancel(ct_shutdown_token_t* self) {
    if (!self) { return false; }

    const ct_shutdown_token_callback_t on_cancel = self->cb_;
    void* const                        arg       = self->arg_;

    bool first = ct_timeout_signal_cancel(&self->signal_);
    if (first) {
        ct_mutex_lock(&self->mutex_);
        ct_cond_broadcast(&self->cond_);
        ct_mutex_unlock(&self->mutex_);

        if (on_cancel) { on_cancel(arg); }
    }
    return first;
}

bool ct_shutdown_token_wait(ct_shutdown_token_t* self, ct_time64_t timeout_ms) {
    if (!self) { return false; }

    ct_time64_t user_deadline = 0;
    if (timeout_ms >= 0) {
        const ct_time64_t now = ct_gettick_ms();
        user_deadline         = timeout_ms > INT64_MAX - now ? INT64_MAX : now + timeout_ms;
    }

    ct_mutex_lock(&self->mutex_);

    while (!ct_shutdown_token_is_done(self)) {
        ct_time64_t remain = ct_shutdown_token_remaining(self);
        if (remain == 0) { break; }

        ct_time64_t user_remain = -1;
        if (timeout_ms >= 0) {
            user_remain = (ct_time64_t)(user_deadline - ct_gettick_ms());
            if (user_remain <= 0) { break; }
        }

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

        (void)ct_cond_wait_for(&self->cond_, &self->mutex_, to_wait);
    }

    const bool done = ct_shutdown_token_is_done(self);
    ct_mutex_unlock(&self->mutex_);

    return done;
}
