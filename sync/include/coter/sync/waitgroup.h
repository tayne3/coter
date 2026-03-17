/**
 * @file waitgroup.h
 * @brief 等待组实现
 */
#ifndef COTER_SYNC_WAITGROUP_H
#define COTER_SYNC_WAITGROUP_H

#include "coter/core/platform.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 等待组
 * @param counter 计数器
 * @param mutex 互斥锁
 * @param cond 条件变量
 */
typedef struct ct_waitgroup {
	int             counter;  // 计数器
	ct_mutex_t      mutex;    // 互斥锁
	ct_cond_t       cond;     // 条件变量
} ct_waitgroup_t;

#define CT_WAITGROUP_INITIALIZER {0, CT_MUTEX_INITIALIZER, CT_COND_INITIALIZER}

/**
 * @brief 初始化等待组
 * @param wg 等待组指针
 * @return int 成功返回0，失败返回非0值
 */
COTER_API int ct_waitgroup_init(ct_waitgroup_t* wg);

/**
 * @brief 销毁等待组
 * @param wg 等待组指针
 */
COTER_API void ct_waitgroup_destroy(ct_waitgroup_t* wg);

/**
 * @brief 增加等待计数
 * @param wg 等待组指针
 * @param delta 增加的计数值
 */
COTER_API void ct_waitgroup_add(ct_waitgroup_t* wg, int delta);

/**
 * @brief 完成一个任务（计数减1）
 * @param wg 等待组指针
 */
COTER_API void ct_waitgroup_done(ct_waitgroup_t* wg);

/**
 * @brief 等待所有任务完成
 * @param wg 等待组指针
 */
COTER_API void ct_waitgroup_wait(ct_waitgroup_t* wg);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_WAITGROUP_H
