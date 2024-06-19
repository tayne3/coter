/**
 * @file ct_platform.h
 * @brief 封装后的的跨平台的标准库函数
 * @author tayne3@dingtalk.com
 * @date 2024.2.20
 */
#ifndef _CT_PLATFORM_H
#define _CT_PLATFORM_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#include "platform/ct_platform_msvc.h"
#else
#include "platform/ct_platform_unix.h"
#endif

#ifdef __cplusplus
}
#endif
#endif  // _CT_PLATFORM_H
