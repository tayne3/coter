/**
 * @file ct_evmsg.h
 * @brief 事件消息中枢
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#ifndef _CT_EVMSG_H
#define _CT_EVMSG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

/**
 * @brief 事件消息
 */
typedef struct {
	uint8_t type;  // 事件类型
	uint8_t id;    // 事件ID
	void   *data;  // 事件数据
	size_t  size;  // 事件数据大小
} ct_evmsg_t, ct_evmsg_buf_t[1];

#define CT_EVMSG_MSG_INIT(_type, _id, _data, _size) {.type = _type, .id = _id, .size = _size, .data = _data}

/**
 * @brief 消息处理函数类型
 * @param msg 事件消息缓冲区
 * @param userdata 用户数据
 * @return true=事件处理完毕; false=事件继续往下处理
 */
typedef bool (*ct_evmsg_handler_t)(ct_evmsg_buf_t msg, void *userdata);

/**
 * @brief 初始化事件消息中枢
 */
CT_API void ct_evmsg_mgr_init(void) __ct_throw;

/**
 * @brief 销毁事件消息中枢
 */
CT_API void ct_evmsg_mgr_destroy(void);

/**
 * @brief 初始化事件消息
 */
CT_API void ct_evmsg_init(ct_evmsg_buf_t msg, uint8_t type, uint8_t id, void *data, size_t size);

/**
 * @brief 事件消息调度
 */
CT_API void ct_evmsg_schedule(void);

/**
 * @brief 订阅特定类型的事件
 * @param type 事件类型
 * @param handler 事件处理函数
 * @param userdata 用户数据
 */
CT_API void ct_evmsg_subscribe(uint8_t type, ct_evmsg_handler_t handler, void *userdata);

/**
 * @brief 发布事件
 * @param msg 事件消息缓冲区
 */
CT_API void ct_evmsg_publish(ct_evmsg_buf_t msg);

#ifdef __cplusplus
}
#endif
#endif  // _CT_EVMSG_H
