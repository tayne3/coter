/**
 * @file evhub.h
 * @brief 基于发布-订阅模式的线程安全同步事件中枢。
 *
 * @details
 * 事件中枢 (Event Hub) 提供了一种中心化的事件分发机制，用于实现模块间的解耦。
 * 发布者无需关心订阅者的细节即可发布事件，订阅者则接收其所关注类型的事件。
 *
 * 核心特性:
 * - **同步执行**: 事件回调函数在 `ct_evhub_publish` 调用期间，于发布者所在的线程中同步执行。
 * - **线程安全**: 所有公开接口均为线程安全。内部使用读写锁，允许多个发布者并发执行，
 *   同时保证订阅管理的原子性。
 * - **性能特点**: 针对“频繁发布、静态订阅”的场景进行了优化。订阅与取消订阅是写操作，
 *   会暂时阻塞所有发布活动，因此不适合订阅关系频繁变化的场景。
 *
 * @note
 * 该模块是进程内事件通信的理想选择，适用于需要简单高效的组件间事件响应机制的场合。
 */
#ifndef COTER_EVENT_HUB_H
#define COTER_EVENT_HUB_H

#include "coter/container/vector.h"
#include "coter/sync/rwlock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 事件中枢结构体
 */
typedef struct ct_evhub {
	ct_rwlock_t rwlock;    // 读写锁
	ct_vector_t sub_list;  // 订阅者列表
} ct_evhub_t;

/**
 * @brief 事件中枢处理函数类型
 */
typedef void (*ct_evhub_callback_t)(uint32_t type, void *data, void *user_data);

/**
 * @brief 初始化事件中枢
 * @param self 事件中枢指针
 */
COTER_API void ct_evhub_init(ct_evhub_t *self);

/**
 * @brief 反初始化事件中枢
 * @param self 事件中枢指针
 */
COTER_API void ct_evhub_deinit(ct_evhub_t *self);

/**
 * @brief 订阅事件
 * @param self 事件中枢指针
 * @param type 事件类型
 * @param cb 事件回调函数
 * @param user_data 用户数据
 * @return 0=成功, 其他=失败
 */
COTER_API int ct_evhub_subscribe(ct_evhub_t *self, uint32_t type, ct_evhub_callback_t cb, void *user_data);

/**
 * @brief 取消订阅事件
 * @param self 事件中枢指针
 * @param type 事件类型
 * @param cb 事件回调函数
 * @return 0=成功, 其他=失败
 */
COTER_API int ct_evhub_unsubscribe(ct_evhub_t *self, uint32_t type, ct_evhub_callback_t cb);

/**
 * @brief 发布事件
 * @param self 事件中枢指针
 * @param type 事件类型
 * @param data 事件数据
 * @return 0=成功, 其他=失败
 */
COTER_API int ct_evhub_publish(ct_evhub_t *self, uint32_t type, void *data);

#ifdef __cplusplus
}
#endif
#endif  // COTER_EVENT_HUB_H
