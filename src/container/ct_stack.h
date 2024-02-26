/**
 * @file ct_stack.h
 * @brief 栈实现
 * @note
 *  定义了栈的数据结构和相关操作函数，包含栈的初始化、入栈、出栈、查询等操作。
 *  栈是一种后进先出（LIFO）的数据结构，可以用于存储和访问一系列元素。
 *  本文件定义了栈的结构体和基本操作函数。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_STACK_H
#define _CT_STACK_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"
#include "common/ct_any.h"

/**
 * @brief 栈结构体
 */
typedef struct ct_stack {
	ct_any_t* _all;   // 缓存区
	size_t    _max;   // 缓存区长度
	size_t    _size;  // 元素数量
} ct_stack_t, ct_stack_buf_t[1];

// 初始化
#define CT_STACK_INIT(__buffer, __max)                      \
	{                                                       \
		._all = (void*)__buffer, ._max = __max, ._size = 0, \
	}

#define ct_stack_max(self)     ((self)->_max)                   // 获取 栈 最大容量
#define ct_stack_size(self)    ((self)->_size)                  // 获取 栈 大小
#define ct_stack_isempty(self) ((self)->_size == 0)             // 判断 栈 是否为空
#define ct_stack_isfull(self)  ((self)->_size == (self)->_max)  // 判断 栈 是否已满
#define ct_stack_clear(self)   ((self)->_size = 0)              // 清空 栈 所有元素

/**
 * @brief 初始化栈
 * @param self 栈-变量指针
 * @param buffer 缓冲区地址
 * @param max 缓冲区大小
 */
void ct_stack_init(ct_stack_buf_t self, ct_any_t* buffer, size_t max);

/**
 * @brief 将元素value推入栈顶
 * @param self 栈-变量指针
 * @param item 要添加的元素
 * @return 栈状态
 */
bool ct_stack_push(ct_stack_buf_t self, ct_any_t item);

/**
 * @brief 弹出栈顶的元素
 * @param self 栈-变量指针
 * @param item 用于存储元素的指针
 * @return 成功返回 true，失败返回 false
 */
bool ct_stack_pop(ct_stack_buf_t self, ct_any_buf_t item);

/**
 * @brief 返回栈顶元素, 但不将其从栈中移除
 * @param self 栈-变量指针
 * @param item 用于存储元素的指针
 * @return 成功返回 true，失败返回 false
 */
bool ct_stack_top(ct_stack_buf_t self, ct_any_buf_t item);

#ifdef __cplusplus
}
#endif
#endif  // _CT_STACK_H
