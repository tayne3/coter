/**
 * @file stack.h
 * @brief LIFO 栈
 */
#ifndef COTER_CONTAINER_STACK_H
#define COTER_CONTAINER_STACK_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_stack {
    char*  _all;   // 元素缓冲区
    size_t _byte;  // 单个元素字节大小
    size_t _max;   // 缓冲区可容纳的元素数量
    size_t _size;  // 当前元素数量
} ct_stack_t, ct_stack_buf_t[1];

#define CT_STACK_INIT(__buffer, __byte, __max) {(char*)(__buffer), (__byte), (__max), 0}

#define ct_stack_max(self)     ((self)->_max)
#define ct_stack_size(self)    ((self)->_size)
#define ct_stack_isempty(self) ((self)->_size == 0)
#define ct_stack_isfull(self)  ((self)->_size == (self)->_max)
#define ct_stack_clear(self)   ((self)->_size = 0)

/**
 * @brief 初始化栈
 * @param buffer 用户提供的缓冲区，需至少能容纳 max 个 byte 字节的元素
 * @param byte   单个元素的字节大小
 * @param max    缓冲区可容纳的元素数量
 * @return 成功返回 0，参数非法返回 -1
 */
CT_API int ct_stack_init(ct_stack_buf_t self, void* buffer, size_t byte, size_t max);

/**
 * @brief 入栈：将 item 指向的数据拷贝至栈顶
 * @param item 指向待入栈数据的指针
 * @return 栈满或 self 为 NULL 时返回 false
 */
CT_API bool ct_stack_push(ct_stack_buf_t self, const void* item);

/**
 * @brief 出栈：移除栈顶元素，若 item 非 NULL 则将数据拷贝至 item
 * @param item 输出缓冲区，可为 NULL（仅移除不取值）
 * @return 栈空或 self 为 NULL 时返回 false
 */
CT_API bool ct_stack_pop(ct_stack_buf_t self, void* item);

/**
 * @brief 查看栈顶元素（不移除），数据拷贝至 item
 * @param item 输出缓冲区，可为 NULL（仅查询不取值）
 * @return 栈空或 self 为 NULL 时返回 false
 */
CT_API bool ct_stack_top(ct_stack_buf_t self, void* item);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_STACK_H
