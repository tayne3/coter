/**
 * @file app.h
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#ifndef _APP_H
#define _APP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

/**
 * @brief 应用实例
 */
typedef struct app app_t, *app_ptr_t;

/**
 * @brief 创建应用实例
 * @return app_ptr_t 返回应用实例的指针
 */
app_ptr_t app_create(void);

/**
 * @brief 执行应用逻辑
 * @param self 应用实例的指针
 * @return int 返回异常标志的值
 */
int app_exec(app_ptr_t self);

/**
 * @brief 应用退出处理
 * @param code 退出状态码
 * @param msg 退出消息
 */
void app_exit(int code, const char* msg);

/**
 * @brief 应用崩溃
 * @param code 退出状态码
 * @param msg 退出消息
 */
void app_crash(int code, const char* msg);

#ifdef __cplusplus
}
#endif
#endif  // _APP_H
