/**
 * @file ct_rwlock.h
 * @brief 读写锁
 * @author tayne3@dingtalk.com
 * @date 2024.1.23
 */
#ifndef _CT_RWLOCK_H
#define _CT_RWLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "base/ct_types.h"

typedef struct ct_rwlock {
#ifdef PTHREAD_RWLOCK_INITIALIZER
	pthread_rwlock_t d[1];
#else
	uint32_t d;
#endif
} ct_rwlock_t, ct_rwlock_buf_t[1];

#ifdef PTHREAD_RWLOCK_INITIALIZER
typedef ct_rwlock_t* ct_rwlock_ptr_t;
#else
typedef volatile ct_rwlock_t* ct_rwlock_ptr_t;
#endif

// clang-format off
#ifdef PTHREAD_RWLOCK_INITIALIZER
#define CT_RWLOCK_INITIALIZATION {{PTHREAD_RWLOCK_INITIALIZER}}
#else
#define CT_RWLOCK_INITIALIZATION {0}
#endif
// clang-format on

/**
 * @brief 初始化读写锁
 * @param self 读写锁对象
 * @return true=初始化成功; false=未知错误
 */
bool ct_rwlock_init(ct_rwlock_ptr_t self) __ct_func_throw;

/**
 * @brief 销毁读写锁
 * @param self 读写锁对象
 * @return true=销毁成功; false=未知错误
 */
bool ct_rwlock_destroy(ct_rwlock_ptr_t self);

/**
 * @brief 获取读锁
 * @param self 读写锁对象
 * @return true=获取成功; false=未知错误
 */
bool ct_rwlock_rlock(ct_rwlock_ptr_t self);

/**
 * @brief 获取写锁
 * @param self 读写锁对象
 * @return true=加锁成功; false=未知错误
 */
bool ct_rwlock_wlock(ct_rwlock_ptr_t self);

/**
 * @brief 尝试获取读锁
 * @param self 读写锁对象
 * @return true=获取成功; false=加锁失败
 */
bool ct_rwlock_try_rlock(ct_rwlock_ptr_t self);

/**
 * @brief 尝试获取写锁
 * @param self 读写锁对象
 * @return true=加锁成功; false=加锁失败
 */
bool ct_rwlock_try_wlock(ct_rwlock_ptr_t self);

/**
 * @brief 释放读写锁
 * @param self 读写锁对象
 * @return true=释放成功; false=未知错误
 */
bool ct_rwlock_unlock(ct_rwlock_ptr_t self);

#ifdef __cplusplus
}
#endif
#endif  // _CT_RWLOCK_H
