/**
 * @file ct_heap.h
 * @brief 堆实现
 * @note
 * 	定义了堆的数据结构和相关操作函数，包含堆的初始化、插入、删除、查询等操作。
 * 	堆是一种特殊的完全二叉树结构，可以用于高效地维护一组元素，并支持快速的插入和删除操作。
 * 	本文件定义了堆的结构体和基本操作函数。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_HEAP_H
#define _CT_HEAP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"
#include "common/ct_any.h"

/// 排序比较函数类型 (返回值: true=触发交换, false=无操作)
typedef bool (*ct_heap_sort_t)(const ct_any_buf_t, const ct_any_buf_t);

/**
 * @brief 堆结构体
 * @note
 * cheap是一种特殊的完全二叉树结构: 最小堆/最大堆 (取决于给定的排序比较函数);
 * 最小堆:
 * - 其中每个结点的值都是一个小于其子节点的值;
 * - 顶部的结点是最小堆的根结点,根结点的值是最小值;
 * 最大堆:
 * - 其中每个结点的值都是一个大于其子节点的值;
 * - 顶部的结点是最大堆的根结点,根结点的值是最大值;
 */
typedef struct ct_heap {
	ct_any_t*      _all;   // 所有元素
	size_t         _max;   // 最大容量
	size_t         _size;  // 堆的大小
	ct_heap_sort_t _sort;  // 排序比较函数
} ct_heap_t, ct_heap_buf_t[1];

#define ct_heap_size(self)    ((self)->_size)                                          // 获取 堆 大小
#define ct_heap_max(self)     ((self)->_max)                                           // 获取 堆 最大容量
#define ct_heap_isempty(self) ((self)->_size == 0)                                     // 判断 堆 是否为空
#define ct_heap_isfull(self)  ((self)->_size >= (self)->_max)                          // 判断 堆 是否已满
#define ct_heap_data(self, i) ((self)->_all[i])                                        // 获取 堆 指定元素
#define ct_heap_first(self)   (ct_heap_isempty(self) ? ct_any_null : (self)->_all[0])  // 获取 堆 堆顶元素
#define ct_heap_clear(self)   ((self)->_size = 0)                                      // 清空 堆 所有元素

/**
 * @brief 最小堆/最大堆-遍历
 * @param it 元素指针
 * @param i 元素索引
 * @param type 元素类型
 * @param self 最小堆/最大堆-变量指针
 */
#define ct_heap_foreach(it, i, type, self)                                         \
	for (i = 0, it = (type)(uint64_t) & (self)->_all[i].d; i < ct_heap_size(self); \
		 i++, it   = (type)(uint64_t) & (self)->_all[i].d)

/**
 * @brief 最小堆/最大堆-遍历
 * @param it 元素指针
 * @param i 元素索引
 * @param type 元素类型
 * @param self 最小堆/最大堆-变量指针
 */
#define ct_heap_foreach_p(it, i, type, self) \
	for (i = 0, it = (type*)(self)->_all[i].d.ptr; i < ct_heap_size(self); i++, it = (type*)(self)->_all[i].d.ptr)

/// 初始化堆
void ct_heap_init(ct_heap_buf_t self, ct_any_t* all, size_t max, ct_heap_sort_t sort);
/// 向上调整堆
void ct_heap_up(ct_heap_buf_t self, size_t i);
/// 向下调整堆
void ct_heap_down(ct_heap_buf_t self, size_t i);
/// 重新排序
void ct_heap_reorder(ct_heap_t* heap);
/// 插入元素
void ct_heap_insert(ct_heap_buf_t self, ct_any_t data);
/// 移除堆顶元素
void ct_heap_remove(ct_heap_buf_t self);
/// 获取并移除堆顶元素
ct_any_t ct_heap_take(ct_heap_buf_t self);
/// 拷贝所有元素到目标堆
bool ct_heap_copy(ct_heap_buf_t self, ct_heap_t* target);

#ifdef __cplusplus
}
#endif
#endif  // _CT_HEAP_H
