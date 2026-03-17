/**
 * @file waitgroup.c
 * @brief 等待组实现
 */
#include "coter/sync/waitgroup.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

int ct_waitgroup_init(ct_waitgroup_t* wg) {
	if (!wg) {
		return -1;
	}
	if (ct_mutex_init(&wg->mutex) != 0) {
		return -1;
	}
	if (ct_cond_init(&wg->cond) != 0) {
		ct_mutex_destroy(&wg->mutex);
		return -1;
	}
	wg->counter = 0;
	return 0;
}

void ct_waitgroup_destroy(ct_waitgroup_t* wg) {
	if (!wg) {
		return;
	}
	ct_mutex_destroy(&wg->mutex);
	ct_cond_destroy(&wg->cond);
}

void ct_waitgroup_add(ct_waitgroup_t* wg, int delta) {
	if (!wg || delta < 0) {
		return;
	}
	ct_mutex_lock(&wg->mutex);
	wg->counter += delta;
	ct_mutex_unlock(&wg->mutex);
}

void ct_waitgroup_done(ct_waitgroup_t* wg) {
	if (!wg) {
		return;
	}
	ct_mutex_lock(&wg->mutex);
	wg->counter--;
	if (wg->counter == 0) {
		ct_cond_broadcast(&wg->cond);
	}
	ct_mutex_unlock(&wg->mutex);
}

void ct_waitgroup_wait(ct_waitgroup_t* wg) {
	if (!wg) {
		return;
	}
	ct_mutex_lock(&wg->mutex);
	while (wg->counter > 0) {
		ct_cond_wait(&wg->cond, &wg->mutex);
	}
	ct_mutex_unlock(&wg->mutex);
}

// -------------------------[STATIC DEFINITION]-------------------------
