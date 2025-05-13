/**
 * @file app.h
 * @brief Application 实例
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#ifndef _APP_H
#define _APP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "log.h"

/**
 * @brief 应用实例
 */
typedef struct gapp gapp_t;

/**
 * @brief 创建应用实例
 * @return gapp_t* 返回应用实例的指针
 */
gapp_t* gapp_create(void);

/**
 * @brief 执行应用逻辑
 * @param self 应用实例的指针
 * @return int 返回异常标志的值
 */
int gapp_exec(gapp_t* self);

/**
 * @brief 应用退出处理
 * @param code 退出状态码
 * @param msg 退出消息
 */
void global_exit(int code, const char* msg);

/**
 * @brief 添加结束回调
 * @param callback 结束回调函数
 * @note 在程序退出时, 按添加顺序的逆序执行
 * @return 成功返回0，失败返回非0值
 */
int global_atexit(void (*callback)(void));

/**
 * @brief 异步执行
 * @param routine 异步执行函数
 * @param arg 异步执行参数
 * @return 成功返回0, 失败返回非0值
 */
int global_async(void (*routine)(void*), void* arg);

#ifdef __cplusplus
}
#endif
#endif  // _APP_H
