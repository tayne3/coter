/**
 * @file utils.h
 * @brief 工具函数
 */
#ifndef COTER_MEM_ALLOC_H
#define COTER_MEM_ALLOC_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 反向拷贝指定字节数的数据
 *
 * @param dest 目标缓冲区的指针
 * @param src 源缓冲区的指针
 * @param n 要复制的字节数
 * @return 返回目标缓冲区的指针
 */
void *ct_reverse_memcpy(void *dest, const void *src, size_t n);

void *ct_reverse_memmove(void *dest, const void *src, size_t n);

#ifdef __cplusplus
}
#endif
#endif  // COTER_MEM_ALLOC_H
