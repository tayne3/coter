// clang-format off
/** 
 * @file ct_spinlock.h
 * @brief 自旋锁
 * @author tayne3@dingtalk.com
 * @date 2023.12.28
 * @note 
 * 
 * 自旋锁（Spinlock）是一种同步机制，用于在多线程环境下保护共享资源。
 * 其工作原理是“忙等待”，即当关键代码段被其他线程占用时，当前线程将持续进行忙等待，直到其他线程释放资源。
 *
 * 自旋锁的优势：
 *   - 响应迅速：由于自旋锁的加锁和解锁操作都在当前线程中执行，避免了线程的上下文切换和调度开销，因此其响应速度比互斥锁更快。
 *   - 线程始终处于运行状态：自旋锁采用忙等待的方式，线程始终处于运行状态，不会被挂起，从而避免了线程切换带来的性能损耗。
 *
 * 自旋锁的劣势：
 *   - 资源浪费：如果持有锁的线程长时间不释放，其他线程会持续忙等待，这将导致资源的浪费。
 *   - 不支持优先级反转处理：如果一个高优先级的线程试图获取一个被低优先级线程持有的自旋锁，可能会导致低优先级线程无法释放锁，从而引发死锁。
 *
 * 自旋锁的适用场景：
 *   - 锁的占用时间预计很短：自旋锁不会引起线程切换和上下文切换的开销，因此适用于临界区较小且占用时间短的情况。
 *   - 临界区较小：线程不会长时间等待锁的释放，从而避免线程进入阻塞状态，提高系统的响应速度。
 *   - 并发线程数较少：避免过多的线程竞争。
 *
 * 自旋锁与互斥锁的区别：
 *   - 自旋锁适用于临界区较小、占用时间短、并发线程数较少的情况，在多核环境下更有优势。
 *   - 互斥锁适用于临界区较大、占用时间长、并发线程数较多的情况，在多核环境下可以避免线程的阻塞和上下文切换开销。
 *   - 在单核环境下，由于自旋锁会导致线程持续忙等待，从而浪费CPU资源，因此互斥锁更适合使用。
 */
// clang-format on
#ifndef _CT_SPINLOCK_H
#define _CT_SPINLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

typedef uint32_t                ct_spinlock_t, ct_spinlock_buf_t[1];
typedef volatile ct_spinlock_t *ct_spinlock_ptr_t;

#define CT_SPINLOCK_INITIALIZATION 0

/**
 * @brief 初始化自旋锁
 *
 * @param self 自旋锁对象
 * @return true=初始化成功; false=未知错误
 */
bool ct_spinlock_init(ct_spinlock_ptr_t self) __ct_func_throw;

/**
 * @brief 销毁自旋锁
 *
 * @param self 自旋锁对象
 * @return true=销毁成功; false=未知错误
 */
bool ct_spinlock_destroy(ct_spinlock_ptr_t self);

/**
 * @brief 获取自旋锁
 *
 * @param self 自旋锁对象
 * @return true=获取成功; false=未知错误
 */
bool ct_spinlock_lock(ct_spinlock_ptr_t self);

/**
 * @brief 尝试加锁
 *
 * @param self 自旋锁对象
 * @return true=加锁成功; false=加锁失败
 */
bool ct_spinlock_try_lock(ct_spinlock_ptr_t self);

/**
 * @brief 释放自旋锁
 *
 * @param self 自旋锁对象
 * @return true=释放成功; false=未知错误
 */
bool ct_spinlock_unlock(ct_spinlock_ptr_t self);

#ifdef __cplusplus
}
#endif
#endif  // _CT_SPINLOCK_H
