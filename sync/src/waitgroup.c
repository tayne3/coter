/**
 * @file waitgroup.c
 * @brief Waitgroup implementation
 */
#include "coter/sync/waitgroup.h"

#include <limits.h>

#include "coter/core/abort.h"

int ct_waitgroup_init(ct_waitgroup_t* wg) {
    if (!wg) { return -1; }
    if (ct_mutex_init(&wg->_mu) != 0) { return -1; }
    if (ct_cond_init(&wg->_cond) != 0) {
        ct_mutex_destroy(&wg->_mu);
        return -1;
    }
    wg->_counter = 0;
    return 0;
}

void ct_waitgroup_destroy(ct_waitgroup_t* wg) {
    if (!wg) { return; }
    ct_mutex_destroy(&wg->_mu);
    ct_cond_destroy(&wg->_cond);
}

void ct_waitgroup_add(ct_waitgroup_t* wg, int delta) {
    if (!wg) { return; }

    ct_mutex_lock(&wg->_mu);
    const int64_t next = (int64_t)wg->_counter + (int64_t)delta;
    if (next < 0 || next > UINT32_MAX) { CT_ABORT(); }

    wg->_counter = (uint32_t)next;
    if (wg->_counter == 0) { ct_cond_broadcast(&wg->_cond); }
    ct_mutex_unlock(&wg->_mu);
}

void ct_waitgroup_done(ct_waitgroup_t* wg) {
    ct_waitgroup_add(wg, -1);
}

void ct_waitgroup_wait(ct_waitgroup_t* wg) {
    if (!wg) { return; }
    ct_mutex_lock(&wg->_mu);
    while (wg->_counter > 0) { ct_cond_wait(&wg->_cond, &wg->_mu); }
    ct_mutex_unlock(&wg->_mu);
}

bool ct_waitgroup_wait_for(ct_waitgroup_t* wg, ct_time64_t timeout_ms) {
    if (!wg) { return false; }
    if (timeout_ms < 0) {
        ct_waitgroup_wait(wg);
        return true;
    }

    const ct_time64_t deadline = ct_getuptime_ms() + timeout_ms;
    ct_mutex_lock(&wg->_mu);
    while (wg->_counter > 0) {
        const ct_time64_t now = ct_getuptime_ms();
        if (now >= deadline) {
            ct_mutex_unlock(&wg->_mu);
            return false;
        }
        (void)ct_cond_wait_for(&wg->_cond, &wg->_mu, deadline - now);
    }
    ct_mutex_unlock(&wg->_mu);
    return true;
}
