/**
 * @file ct_app.h
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#ifndef _CT_APP_H
#define _CT_APP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

/**
 * @brief coter 应用实例
 */
typedef struct ct_app ct_app_t, *ct_app_ptr_t;

/**
 * @brief 创建应用实例
 * @return ct_app_ptr_t 返回应用实例的指针
 */
COTER_API ct_app_ptr_t ct_app_create(void);

/**
 * @brief 执行应用逻辑
 * @param self 应用实例的指针
 * @return int 返回异常标志的值
 */
COTER_API int ct_app_exec(ct_app_ptr_t self);

/**
 * @brief 应用退出处理
 * @param code 退出状态码
 * @param msg 退出消息
 */
COTER_API void ct_app_exit(int code, const char* msg);

#ifdef __cplusplus
}
#endif
#endif  // _CT_APP_H
