/**
 * @file ct_log_control.h
 * @brief 日志控制器
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_LOG_CONTROL_H
#define _CT_LOG_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ct_log_storage.h"

/**
 * @brief 日志回调函数类型
 * @param message 日志消息
 * @param size 消息长度
 */
typedef void (*ct_log_callback_t)(char *message, size_t size);

// 日志控制项
typedef struct {
	bool              is_print;  // 是否打印输出
	int               id;        // 控制项ID
	ct_log_storage_t *storage;   // 存储配置 (为空则不存储到文件)
	ct_log_callback_t callback;  // 回调函数
} ct_log_control_t;

/**
 * @brief 获取指定类型的控制对象指针
 * @param type 控制类型
 * @return 返回指定类型的控制对象指针，如果未找到则返回空指针
 */
ct_log_control_t *ct_log_control_get(int type);

/**
 * @brief 向控制对象发送请求并获取响应
 * @param type 控制类型
 * @return 返回指定类型的控制对象指针，如果未找到则返回空指针
 */
ct_log_control_t *ct_log_control_ask(int type);

/**
 * @brief 关闭指定类型的控制对象
 * @param type 控制类型
 */
void ct_log_control_close(int type);

/**
 * @brief 修改日志配置
 * @param type 日志类型
 * @param is_print 是否打印输出
 * @param callback 日志回调函数
 * @param storage 日志存储结构体指针
 */
void ct_log_config_set(int type, bool is_print, ct_log_callback_t callback, ct_log_storage_t *storage);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_CONTROL_H
