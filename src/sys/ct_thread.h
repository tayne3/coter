/**
 * @file ct_thread.h
 * @brief 线程创建、撤销、同步等功能
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_THREAD_H
#define _CT_THREAD_H
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "base/ct_types.h"

// 线程对象
typedef struct ct_thread {
	pthread_t _t;
} ct_thread_t, ct_thread_buf_t[1];

// 线程入口函数
typedef void* (*ct_thread_routine_t)(void*);

/**
 * @brief 创建线程
 * @param self 线程对象
 * @param routine 线程入口函数
 * @param arg 线程入口函数参数
 * @return 成功返回true; 失败返回false
 */
bool ct_thread_create(ct_thread_buf_t self, ct_thread_routine_t routine, void* arg) __ct_func_throw;

/**
 * @brief 等待线程结束
 * @param self 线程对象
 * @param ret 执行结果
 * @return 成功返回true; 失败返回false
 */
bool ct_thread_join(ct_thread_buf_t self, void** ret);

/**
 * @brief 线程退出
 * @param ret 执行结果
 * @return 成功返回true; 失败返回false
 */
void ct_thread_exit(void* ret);

/**
 * @brief 分离线程
 * @param self 线程对象
 * @return 成功返回true; 失败返回false
 */
bool ct_thread_detach(ct_thread_buf_t self);

/**
 * @brief 取消线程
 * @param self 线程对象
 * @return 成功返回true; 失败返回false
 */
bool ct_thread_cancel(ct_thread_buf_t self);

/**
 * @brief 线程睡眠 (微秒)
 * @param us 睡眠时间(微秒)
 * @return 成功返回true; 失败返回false
 */
bool ct_thread_usleep(uint_t us);

/**
 * @brief 线程睡眠 (毫秒)
 * @param ms 睡眠时间(毫秒)
 * @return 成功返回true; 失败返回false
 */
bool ct_thread_msleep(uint_t ms);

/**
 * @brief 线程睡眠 (秒)
 * @param sec 睡眠时间(秒)
 * @return 成功返回true; 失败返回false
 */
bool ct_thread_sleep(uint_t sec);

#ifdef __cplusplus
}
#endif
#endif  // _CT_THREAD_H
