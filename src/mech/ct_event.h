/**
 * @file ct_event.h
 * @brief 事件机制
 * @author tayne3@dingtalk.com
 * @date 2023.11.29
 */
#ifndef _CT_EVENT_H
#define _CT_EVENT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"
#include "common/ct_any.h"
#include "sys/ct_cond.h"
#include "sys/ct_mutex.h"

// 单事件id
typedef uint8_t ct_event_id_t;
// 多事件id
typedef uint64_t ct_event_ids_t;

#define CT_EVENT_ID_INVALID      0U                                                   // 单事件id空值
#define CT_EVENT_ID_MIN          (CT_EVENT_ID_INVALID + 1)                            // 单事件id最小值
#define CT_EVENT_ID_MAX          65U                                                  // 单事件id最大值
#define CT_EVENT_ID_ISVALID(id)  ((id) >= CT_EVENT_ID_MIN && (id) < CT_EVENT_ID_MAX)  // 单事件id是否有效
#define CT_EVENT_IDS_INITIALIZER 0ULL                                                 // 多事件id初值

// 事件控制结构体 (一个事件类型 对应 一个事件控制变量)
typedef struct ct_event {
	ct_event_ids_t ids;                        // 次id
	ct_mutex_buf_t mutex;                      // 互斥锁
	ct_cond_buf_t  cond;                       // 条件变量
	ct_any_t       args[CT_EVENT_ID_MAX - 1];  // 参数
} ct_event_t, ct_event_buf_t[1];

// 事件-初始化
#define CT_EVENT_INITIALIZER                                                                                   \
	{                                                                                                          \
		.ids = CT_EVENT_IDS_INITIALIZER, .mutex = {CT_MUTEX_INITIALIZATION}, .cond = {CT_COND_INITIALIZATION}, \
		.args = {CT_ANY_INIT_INVALID},                                                                         \
	}
// 事件-空
#define CT_EVENT_NULL (ct_event_t) CT_EVENT_INITIALIZER

/**
 * @brief 发送事件 (指定事件类型 和 单个事件id)
 * @param self 事件控制变量
 * @param id 事件id
 * @param arg 事件参数
 */
void ct_event_send(ct_event_buf_t self, ct_event_id_t id, ct_any_t arg);

/**
 * @brief 清空所有事件
 * @param self 事件控制变量
 */
void ct_event_clear(ct_event_buf_t self);

/**
 * @brief 获取事件参数
 * @param self 事件控制变量
 * @param id 事件id
 * @return ct_any_t 返回事件参数
 */
ct_any_t ct_event_arg_get(ct_event_buf_t self, ct_event_id_t id);

/**
 * @brief 获取并删除事件参数
 * @param self 事件控制变量
 * @param id 事件id
 * @return ct_any_t 返回事件参数
 */
ct_any_t ct_event_arg_take(ct_event_buf_t self, ct_event_id_t id);

/**
 * @brief 等待接收单个事件 (指定事件类型)
 * @param self 事件控制变量
 * @return ct_event_id_t 返回接收到的事件id
 */
ct_event_id_t ct_event_receive(ct_event_buf_t self);

/**
 * @brief 等待接收单个事件 (指定事件类型 和 单事件id)
 * @param self 事件控制变量
 * @param id 单个事件id
 * @return ct_event_id_t 返回接收到的事件id
 */
ct_event_id_t ct_event_receive_single(ct_event_buf_t self, ct_event_id_t id);

/**
 * @brief 等待接收单个事件 (指定事件类型 和 多事件id)
 * @param self 事件控制变量
 * @param ids 多个事件id
 * @return ct_event_id_t 返回接收到的事件id
 */
ct_event_id_t ct_event_receive_multiple(ct_event_buf_t self, ct_event_ids_t ids);

/**
 * @brief 尝试接收单个事件 (指定事件类型)
 * @param self 事件控制变量
 * @return ct_event_id_t 返回收到的事件id，未收到时返回无效值
 */
ct_event_id_t ct_event_try_receive(ct_event_buf_t self);

/**
 * @brief 尝试接收单个事件 (指定事件类型 和 单事件id)
 * @param self 事件控制变量
 * @param id 单个事件id
 * @return ct_event_id_t 返回收到的事件id，未收到时返回无效值
 */
ct_event_id_t ct_event_try_receive_single(ct_event_buf_t self, ct_event_id_t id);

/**
 * @brief 尝试接收单个事件 (指定事件类型 和 多事件id)
 * @param self 事件控制变量
 * @param ids 多个事件id
 * @return ct_event_id_t 返回收到的事件id，未收到时返回无效值
 */
ct_event_id_t ct_event_try_receive_multiple(ct_event_buf_t self, ct_event_ids_t ids);

/**
 * @brief 检查单事件id是否有效
 * @param id 单事件id
 * @return bool 返回单事件id是否有效
 */
bool ct_event_id_isvalid(ct_event_id_t id);

/**
 * @brief 检查多事件id是否有效
 * @param ids 多事件id
 * @return bool 返回多事件id是否有效
 */
bool ct_event_ids_isvalid(ct_event_ids_t ids);

/**
 * @brief 将多个 ct_event_id_t 合并为单个 ct_event_ids_t
 * @param count ct_event_id_t的数量
 * @param start ct_event_id_t的起始值
 * @param ... 其他的ct_event_id_t
 * @return ct_event_ids_t 返回合并后的单个ct_event_ids_t
 */
ct_event_ids_t ct_event_ids_from(size_t count, size_t start, ...);

/**
 * @brief 将一个 ct_event_id_t 添加到 ct_event_ids_t
 * @param ids 多事件id
 * @param value 要添加的单事件id
 */
void ct_event_ids_add(ct_event_ids_t *ids, ct_event_id_t value);

/**
 * @brief 从 ct_event_ids_t 中删除一个 ct_event_id_t
 * @param ids 多事件id
 * @param value 要删除的单事件id
 */
void ct_event_ids_remove(ct_event_ids_t *ids, ct_event_id_t value);

#ifdef __cplusplus
}
#endif
#endif
