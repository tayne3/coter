/**
 * @file ct_mutex.h
 * @brief 互斥锁
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#ifndef _CT_MUTEX_H
#define _CT_MUTEX_H
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "base/ct_types.h"

// 互斥锁
typedef struct ct_mutex {
	pthread_mutex_t _d[1];
} ct_mutex_t, ct_mutex_buf_t[1];

// clang-format off
// 互斥锁初始化
#define CT_MUTEX_INITIALIZATION {{PTHREAD_MUTEX_INITIALIZER}}
// clang-format on

/**
 * @brief 初始化互斥锁
 * @param self 互斥锁对象
 * @return true=初始化成功; false=初始化失败
 */
bool ct_mutex_init(ct_mutex_buf_t self) __ct_func_throw;

/**
 * @brief 销毁互斥锁
 * @param self 互斥锁对象
 * @return true=销毁成功; false=销毁失败
 */
bool ct_mutex_destroy(ct_mutex_buf_t self) __ct_func_throw;

/**
 * @brief 获取互斥锁
 * @param self 互斥锁对象
 * @return true=获取成功; false=获取失败
 */
bool ct_mutex_lock(ct_mutex_buf_t self) __ct_func_thrownl;

/**
 * @brief 尝试加锁
 * @param self 互斥锁对象
 * @return true=加锁成功; false=加锁失败
 */
bool ct_mutex_try_lock(ct_mutex_buf_t self) __ct_func_thrownl;

/**
 * @brief 释放互斥锁
 * @param self 互斥锁对象
 * @return true=释放成功; false=释放失败
 */
bool ct_mutex_unlock(ct_mutex_buf_t self) __ct_func_thrownl;

#ifdef __cplusplus
}
#endif
#endif
