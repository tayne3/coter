/**
 * @file ct_mutex.c
 * @brief 互斥锁
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "ct_mutex.h"

#include <assert.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_mutex]"

// -------------------------[GLOBAL DEFINITION]-------------------------

bool ct_mutex_init(ct_mutex_buf_t self)
{
	assert(self);
	return 0 == pthread_mutex_init(self->_d, ct_nullptr);
}

bool ct_mutex_destroy(ct_mutex_buf_t self)
{
	assert(self);
	return 0 == pthread_mutex_destroy(self->_d);
}

bool ct_mutex_lock(ct_mutex_buf_t self)
{
	assert(self);
	return 0 == pthread_mutex_lock(self->_d);
}

bool ct_mutex_try_lock(ct_mutex_buf_t self)
{
	assert(self);
	return 0 == pthread_mutex_trylock(self->_d);
}

bool ct_mutex_unlock(ct_mutex_buf_t self)
{
	assert(self);
	return 0 == pthread_mutex_unlock(self->_d);
}

// -------------------------[STATIC DEFINITION]-------------------------
