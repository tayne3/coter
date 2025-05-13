/**
 * @file ct_utils.h
 * @brief 工具函数
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_UTILS_H
#define _CT_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

#include "prefix/ct_macro.h"

/**
 * @brief 反向拷贝指定字节数的数据
 *
 * @param dest 目标缓冲区的指针
 * @param src 源缓冲区的指针
 * @param n 要复制的字节数
 * @return 返回目标缓冲区的指针
 */
void *ct_reverse_memcpy(void *dest, const void *src, size_t n) __ct_nonnull(1, 2);
// #define ct_copy_backward ct_reverse_memcpy

void *ct_reverse_memmove(void *dest, const void *src, size_t n) __ct_nonnull(1, 2);

#ifdef __cplusplus
}
#endif
#endif  // _CT_UTILS_H
