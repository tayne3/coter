/**
 * @file ct_mempool.h
 * @brief 内存池实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#ifndef _CT_MEMPOOL_H
#define _CT_MEMPOOL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

/**
 * @brief 内存池结构体
 */
typedef struct ct_mempool {
	char unused;
} ct_mempool_t, ct_mempool_buf_t[1];

/**
 * @brief
 * @param mempool 内存池
 * @param size 申请的内存大小
 * @return 申请到的内存块地址
 */
void *ct_mempool_malloc(ct_mempool_buf_t mempool, size_t size) __ct_func_throw __ct_func_malloc__ __ct_func_wur__;

/**
 * @brief
 * @param mempool 内存池
 * @param nmemb 申请的区块数量
 * @param size 申请的单个区块的内存大小
 * @return 申请到的内存块地址
 */
void *ct_mempool_calloc(ct_mempool_buf_t mempool, size_t nmemb,
						size_t size) __ct_func_throw __ct_func_malloc__ __ct_func_wur__;

/**
 * @brief
 * @param mempool 内存池
 * @param ptr 释放的内存块地址
 */
void ct_mempool_free(ct_mempool_buf_t mempool, void *ptr) __ct_func_throw;

#ifdef __cplusplus
}
#endif
#endif  // _CT_MEMPOOL_H
