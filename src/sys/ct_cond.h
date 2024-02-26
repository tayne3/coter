/**
 * @file ct_cond.h
 * @brief 条件变量
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_COND_H
#define _CT_COND_H
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "ct_mutex.h"

// 条件变量
typedef struct ct_cond {
	pthread_cond_t _d[1];
} ct_cond_t, ct_cond_buf_t[1];

// clang-format off
#define CT_COND_INITIALIZATION {{PTHREAD_COND_INITIALIZER}}
// clang-format on

/**
 * @brief 初始化条件变量
 * @param self 条件变量对象
 * @return true=初始化成功; false=初始化失败
 */
bool ct_cond_init(ct_cond_buf_t self) __ct_func_throw;

/**
 * @brief 销毁条件变量
 * @param self 条件变量对象
 * @return true=销毁成功; false=销毁失败
 */
bool ct_cond_destroy(ct_cond_buf_t self) __ct_func_throw;

/**
 * @brief 唤醒一个等待条件变量的线程
 * @param self 条件变量对象
 * @return true=唤醒成功; false=唤醒失败
 */
bool ct_cond_notify_one(ct_cond_buf_t self) __ct_func_thrownl;

/**
 * @brief 唤醒所有等待条件变量的线程
 * @param self 条件变量对象
 * @return true=唤醒成功; false=唤醒失败
 */
bool ct_cond_notify_all(ct_cond_buf_t self) __ct_func_thrownl;

/**
 * @brief 等待条件变量
 * @param self 条件变量对象
 * @param mutex 互斥锁对象
 * @return true=等待成功; false=等待失败
 */
bool ct_cond_wait(ct_cond_buf_t self, ct_mutex_buf_t mutex);

/**
 * @brief 等待条件变量，直到超时或被唤醒
 * @param self 条件变量对象
 * @param mutex 互斥锁对象
 * @param ms 超时时长 (-1=永不超时; 0=立刻返回; >0=超时时长)
 * @return true=等待成功; false=等待超时或失败
 */
bool ct_cond_timewait(ct_cond_buf_t self, ct_mutex_buf_t mutex, int ms);

#ifdef __cplusplus
}
#endif
#endif  // _CT_COND_H
