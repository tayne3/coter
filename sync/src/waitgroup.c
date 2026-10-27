/**
 * @file waitgroup.c
 * @brief Waitgroup implementation
 */
#include "coter/sync/waitgroup.h"

#include <limits.h>

#include "coter/core/abort.h"

int ct_waitgroup_init(ct_waitgroup_t* wg) {
    if (!wg) { return -1; }
    if (ct_mutex_init(&wg->mu_) != 0) { return -1; }
    if (ct_cond_init(&wg->cond_) != 0) {
        ct_mutex_destroy(&wg->mu_);
        return -1;
    }
    wg->counter_ = 0;
    return 0;
}

void ct_waitgroup_destroy(ct_waitgroup_t* wg) {
    if (!wg) { return; }
    ct_mutex_destroy(&wg->mu_);
    ct_cond_destroy(&wg->cond_);
}

void ct_waitgroup_add(ct_waitgroup_t* wg, int delta) {
    if (!wg) { return; }

    ct_mutex_lock(&wg->mu_);
    const int64_t next = (int64_t)wg->counter_ + (int64_t)delta;
    if (next < 0 || next > UINT32_MAX) { CT_ABORT(); }

    wg->counter_ = (uint32_t)next;
    if (wg->counter_ == 0) { ct_cond_broadcast(&wg->cond_); }
    ct_mutex_unlock(&wg->mu_);
}

void ct_waitgroup_done(ct_waitgroup_t* wg) {
    ct_waitgroup_add(wg, -1);
}

void ct_waitgroup_wait(ct_waitgroup_t* wg) {
    if (!wg) { return; }
    ct_mutex_lock(&wg->mu_);
    while (wg->counter_ > 0) { ct_cond_wait(&wg->cond_, &wg->mu_); }
    ct_mutex_unlock(&wg->mu_);
}

bool ct_waitgroup_wait_for(ct_waitgroup_t* wg, ct_time64_t timeout_ms) {
    if (!wg) { return false; }
    if (timeout_ms < 0) {
        ct_waitgroup_wait(wg);
        return true;
    }

    const ct_time64_t deadline = ct_gettick_ms() + timeout_ms;
    ct_mutex_lock(&wg->mu_);
    while (wg->counter_ > 0) {
        const ct_time64_t now = ct_gettick_ms();
        if (now >= deadline) {
            ct_mutex_unlock(&wg->mu_);
            return false;
        }
        (void)ct_cond_wait_for(&wg->cond_, &wg->mu_, deadline - now);
    }
    ct_mutex_unlock(&wg->mu_);
    return true;
}
