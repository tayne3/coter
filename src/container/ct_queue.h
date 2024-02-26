/**
 * @file ct_queue.h
 * @brief 队列实现
 * @note
 * 	定义了队列的数据结构和相关操作函数,包含队列的初始化、插入、删除、查询等操作。
 * 	队列是一种先进先出（FIFO）的数据结构，可以用于存储和访问一系列元素。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_QUEUE_H
#define _CT_QUEUE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

/**
 * @brief 队列结构体
 */
typedef struct ct_queue {
	char*  _all;   // 缓存区
	size_t _byte;  // 元素字节大小
	size_t _max;   // 缓存区长度
	size_t _head;  // 头元素索引
	size_t _tail;  // 尾元素索引
	size_t _size;  // 元素数量
} ct_queue_t, ct_queue_buf_t[1];

// 初始化
#define CT_QUEUE_INIT(__buffer, __byte, __max)                                                       \
	{                                                                                                \
		._all = (void*)__buffer, ._byte = __byte, ._max = __max, ._head = 0, ._tail = 0, ._size = 0, \
	}

#define ct_queue_max(self)     ((self)->_max)                                       // 获取 队列 最大容量
#define ct_queue_size(self)    ((self)->_size)                                      // 获取 队列 大小
#define ct_queue_isempty(self) ((self)->_size == 0)                                 // 判断 队列 是否为空
#define ct_queue_isfull(self)  ((self)->_size == (self)->_max)                      // 判断 队列 是否已满
#define ct_queue_clear(self)   ((self)->_size = (self)->_head = (self)->_tail = 0)  // 清空 队列 所有元素

/**
 * @brief 初始化队列
 * @param self 队列-变量指针
 * @param buffer 缓冲区地址
 * @param byte 单个元素字节大小
 * @param max 缓冲区大小
 */
void ct_queue_init(ct_queue_buf_t self, void* buffer, size_t byte, size_t max);

/**
 * @brief 将元素添加到队列的末尾。
 * @param self 队列-变量指针
 * @param item 要添加的元素
 * @return 队列状态
 */
bool ct_queue_enqueue(ct_queue_buf_t self, const void* item);

/**
 * @brief 移除并返回队列的第一个元素
 * @param self 队列-变量指针
 * @param item 用于存储元素的指针
 * @return 成功返回 true，失败返回 false
 */
bool ct_queue_dequeue(ct_queue_buf_t self, void* item);

/**
 * @brief 返回队列的首元素,但不会移除它
 * @param self 队列-变量指针
 * @param item 用于存储元素的指针
 * @return 成功返回 true，失败返回 false
 */
bool ct_queue_head(ct_queue_buf_t self, void* item);

#ifdef __cplusplus
}
#endif
#endif  // _CT_QUEUE_H
