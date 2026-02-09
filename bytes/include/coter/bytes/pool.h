/**
 * @file bytepool.h
 * @brief 字节池
 */
#ifndef COTER_BYTES_POOL_H
#define COTER_BYTES_POOL_H

#include "coter/bytes/bytes.h"

#ifdef __cplusplus
extern "C" {
#endif

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
COTER_API ct_bytepool_t* ct_bytepool_create(size_t max_size, size_t bytes_capacity);

/**
 * @brief 销毁字节池
 * @param self 字节池指针
 */
COTER_API void ct_bytepool_destroy(ct_bytepool_t* self);

/**
 * @brief 从字节池获取一个字节数组
 * @param self 字节池指针
 * @return 返回获取的字节数组指针
 */
COTER_API ct_bytes_t* ct_bytepool_get(ct_bytepool_t* self);

/**
 * @brief 将字节数组放回字节池
 * @param self 字节池指针
 * @param bytes 要放回的字节数组指针
 */
COTER_API void ct_bytepool_put(ct_bytepool_t* self, ct_bytes_t* bytes);

#ifdef __cplusplus
}
#endif
#endif  // COTER_BYTES_POOL_H
