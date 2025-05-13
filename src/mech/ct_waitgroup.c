/**
 * @file ct_waitgroup.c
 * @brief 等待组实现
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "mech/ct_waitgroup.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

int ct_waitgroup_init(ct_waitgroup_t* wg) {
	assert(wg != NULL);
	if (pthread_mutex_init(&wg->mutex, NULL) != 0) {
		return -1;
	}
	if (pthread_cond_init(&wg->cond, NULL) != 0) {
		pthread_mutex_destroy(&wg->mutex);
		return -1;
	}
	wg->counter = 0;
	return 0;
}

void ct_waitgroup_destroy(ct_waitgroup_t* wg) {
	assert(wg != NULL);
	pthread_mutex_destroy(&wg->mutex);
	pthread_cond_destroy(&wg->cond);
}

void ct_waitgroup_add(ct_waitgroup_t* wg, int delta) {
	assert(wg != NULL);
	assert(delta >= 0);  // delta必须为非负数
	pthread_mutex_lock(&wg->mutex);
	wg->counter += delta;
	pthread_mutex_unlock(&wg->mutex);
}

void ct_waitgroup_done(ct_waitgroup_t* wg) {
	assert(wg != NULL);
	pthread_mutex_lock(&wg->mutex);
	wg->counter--;
	if (wg->counter == 0) {
		pthread_cond_broadcast(&wg->cond);
	}
	pthread_mutex_unlock(&wg->mutex);
}

void ct_waitgroup_wait(ct_waitgroup_t* wg) {
	assert(wg != NULL);
	pthread_mutex_lock(&wg->mutex);
	while (wg->counter > 0) {
		pthread_cond_wait(&wg->cond, &wg->mutex);
	}
	pthread_mutex_unlock(&wg->mutex);
}

// -------------------------[STATIC DEFINITION]-------------------------
