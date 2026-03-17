#ifndef COTER_SYNC_EVENT_H
#define COTER_SYNC_EVENT_H

#include "coter/core/platform.h"

#ifndef CT_OS_WIN
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef HANDLE ct_event_t;
#else
typedef struct ct_event {
	ct_cond_t  cond;
	ct_mutex_t mutex;
	int        count;
} ct_event_t;
#endif

/**
 * @brief 初始化事件对象
 * @return 成功返回0
 */
COTER_API int ct_event_init(ct_event_t* event);

/**
 * @brief 销毁事件对象
 * @return 成功返回0
 * @note 如果事件对象在销毁时仍被并发等待或使用, 则行为未定义。
 */
COTER_API int ct_event_destroy(ct_event_t* event);

/**
 * @brief 等待事件被触发
 * @return 成功返回0
 */
COTER_API int ct_event_wait(ct_event_t* event);

/**
 * @brief 超时等待事件被触发
 * @return 成功返回0
 */
COTER_API int ct_event_timedwait(ct_event_t* event, uint32_t timeout_ms);

/**
 * @brief 触发事件
 * @return 成功返回0
 * @note 重复调用不会累加次数
 */
COTER_API int ct_event_signal(ct_event_t* event);

/**
 * @brief 重置事件计数
 * @return 成功返回0
 */
COTER_API int ct_event_reset(ct_event_t* event);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_EVENT_H
