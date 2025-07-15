/**
 * @file ct_bytepool.h
 * @brief 字节池
 * @author tayne3@dingtalk.com
 */
#ifndef _CT_BYTEPOOL_H
#define _CT_BYTEPOOL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "coter/container/bytes.h"

/**
 * @brief 字节池指针
 */
typedef struct ct_bytepool ct_bytepool_t;

/**
 * @brief 创建字节池
 * @param max_size 字节池最大容量
 * @param bytes_capacity 单个字节数组容量
 * @return 返回创建的字节池指针
 */
ct_bytepool_t* ct_bytepool_create(size_t max_size, size_t bytes_capacity);

/**
 * @brief 销毁字节池
 * @param self 字节池指针
 */
void ct_bytepool_destroy(ct_bytepool_t* self) __ct_nonnull(1);

/**
 * @brief 从字节池获取一个字节数组
 * @param self 字节池指针
 * @return 返回获取的字节数组指针
 */
ct_bytes_t* ct_bytepool_get(ct_bytepool_t* self) __ct_nonnull(1);

/**
 * @brief 将字节数组放回字节池
 * @param self 字节池指针
 * @param bytes 要放回的字节数组指针
 */
void ct_bytepool_put(ct_bytepool_t* self, ct_bytes_t* bytes) __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_BYTEPOOL_H
