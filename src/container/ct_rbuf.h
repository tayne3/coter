/**
 * @file ct_rbuf.h
 * @brief 环形缓冲区实现
 * @note
 *  定义了环形缓冲区结构体和相关操作宏，用于实现环形缓冲区的功能。
 *  环形缓冲区是一种循环队列的实现，可以高效地存储和访问数据。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_RBUF_H
#define _CT_RBUF_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

/**
 * @brief 环形缓冲区结构体
 */
typedef struct ct_rbuf {
	char  *_all;   // 缓存区
	size_t _byte;  // 元素字节大小
	size_t _max;   // 缓存区长度
	size_t _head;  // 头元素索引
	size_t _tail;  // 尾元素索引
	size_t _size;  // 元素数量
} ct_rbuf_t, ct_rbuf_buf_t[1];

// 初始化
#define CT_RBUF_INIT(__buffer, __byte, __max) \
	{                                         \
		._all  = (void *)__buffer,            \
		._byte = __byte,                      \
		._max  = __max,                       \
		._head = 0,                           \
		._tail = 0,                           \
		._size = 0,                           \
	}

#define ct_rbuf_max(self)       ((self)->_max)                                       // 获取 最大容量
#define ct_rbuf_size(self)      ((self)->_size)                                      // 获取 大小
#define ct_rbuf_isempty(self)   ((self)->_size == 0)                                 // 判断 是否为空
#define ct_rbuf_isfull(self)    ((self)->_size == (self)->_max)                      // 判断 是否已满
#define ct_rbuf_available(self) ((self)->_max - (self)->_size)                       // 获取 可用空间
#define ct_rbuf_clear(self)     ((self)->_size = (self)->_head = (self)->_tail = 0)  // 清空 所有元素

/**
 * @brief 初始化环形缓冲区
 * @param self 环形缓冲区-变量指针
 * @param buffer 缓冲区地址
 * @param max 缓冲区大小
 */
CT_API void ct_rbuf_init(ct_rbuf_buf_t self, void *buffer, size_t byte, size_t max) __ct_nonnull(1, 2);

/**
 * @brief 向环形缓冲区添加单个元素
 * @param self 环形缓冲区-变量指针
 * @param item 要添加的元素
 * @return 成功返回 true，失败返回 false
 */
CT_API bool ct_rbuf_put(ct_rbuf_buf_t self, const void *item) __ct_nonnull(1);

/**
 * @brief 从环形缓冲区取出单个元素
 * @param self 环形缓冲区-变量指针
 * @param item 用于存储元素的指针
 * @return 成功返回 true，失败返回 false
 */
CT_API bool ct_rbuf_take(ct_rbuf_buf_t self, void *item) __ct_nonnull(1);

/**
 * @brief 向环形缓冲区添加多个元素
 * @param self 环形缓冲区-变量指针
 * @param items 要添加的元素数组
 * @param size 要添加的元素数量
 * @return 成功添加的元素数量
 * @note
 * 该函数用于从环形缓冲区中取出多个元素，如果要取出的元素数量超过了环形缓冲区的大小，将只取出环形缓冲区中的所有元素。
 * 该函数相当于从环形缓冲区中获取到元素之后将其删除。
 */
CT_API size_t ct_rbuf_puts(ct_rbuf_buf_t self, const void *items, size_t size) __ct_nonnull(1);

/**
 * @brief 从环形缓冲区取出多个元素
 * @param self 环形缓冲区-变量指针
 * @param items 用于存储元素的指针数组
 * @param size 要获取的元素数量
 * @return 成功获取的元素数量
 */
CT_API size_t ct_rbuf_takes(ct_rbuf_buf_t self, void *items, size_t size) __ct_nonnull(1);

/**
 * @brief 从环形缓冲区获取多个元素
 * @param self 环形缓冲区-变量指针
 * @param items 用于存储元素的指针数组
 * @param size 要获取的元素数量
 * @return 成功获取的元素数量
 * @note
 * 该函数用于从环形缓冲区中获取多个元素，如果要获取的元素数量超过了环形缓冲区的大小，将只获取环形缓冲区中的所有元素。
 */
CT_API size_t ct_rbuf_gets(ct_rbuf_buf_t self, void *items, size_t size) __ct_nonnull(1);

/**
 * @brief 从环形缓冲区中移除指定数量的元素
 * @param self 环形缓冲区-变量指针
 * @param size 要移除的元素数量
 * @return 成功移除的元素数量
 * @note
 * 该函数用于从环形缓冲区中移除指定数量的元素，如果要移除的元素数量超过了环形缓冲区的大小，将移除环形缓冲区中的所有元素。
 */
CT_API size_t ct_rbuf_remove(ct_rbuf_buf_t self, size_t size) __ct_nonnull(1);

/**
 * @brief 获取环形缓冲区指定位置的元素区块指针
 * @param self 环形缓冲区-变量指针
 * @param offset 元素偏移量
 * @param size 获取的元素数量
 * @return 成功返回元素起始指针, 若索引越界，则返回空指针
 * @note
 * 该函数不改变环形缓冲区中的数据, 仅用于获取元素区块指针
 * 由于环形缓冲区是循环队列, 所以需要分段获取元素区块, 无法一次性获取全部元素, 一般情况下不需要使用该函数
 */
CT_API void *ct_rbuf_items(const ct_rbuf_buf_t self, size_t offset, size_t size[1]) __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_RBUF_H
