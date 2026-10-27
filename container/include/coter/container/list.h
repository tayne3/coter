/**
 * @file list.h
 * @brief 链表实现
 * @note
 *  定义了链表的数据结构和相关操作函数，包含链表的初始化、插入、删除、查询等操作。
 *  链表是一种常见的数据结构，用于存储和访问一系列元素。
 *  本文件定义了链表的节点结构体和链表的基本操作函数。
 */
#ifndef COTER_CONTAINER_LIST_H
#define COTER_CONTAINER_LIST_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ct_list;

/**
 * @struct ct_list
 * @brief 链表结构体
 * @note
 * ct_list 是一种双向链表;
 * 头节点是第零个节点，头节点固定存在，也被称为哨兵节点或者头部哨兵节点;
 * 首节点是第一个节点, 头节点的后继节点为首节点, 首节点的前驱节点为头节点;
 * 尾节点是最后一个节点, 头节点的前驱节点为尾节点，尾节点的后继节点为头节点;
 */
typedef struct ct_list {
    struct ct_list* prev_;  // 前驱节点
    struct ct_list* next_;  // 后继节点
} ct_list_t, ct_list_buf_t[1];

// 计算包含链表节点的对象指针
//	- node_ 指向链表节点的指针
// 	- type_ 包含链表节点的条目的类型
// 	- member_ 在结构体@type中表示链表头的成员变量的名称
#define ct_list_entry(node_, type_, member_) CT_CONTAINER_OF(node_, type_, member_)

#define ct_list_isempty(head_)          ((head_)->next_ == (void*)(head_))  // 判断链表是否为空
#define ct_list_first(head_)            ((head_)->next_)                     // 获取首节点
#define ct_list_last(head_)             ((head_)->prev_)                     // 获取尾节点
#define ct_list_is_first(head_, node_) ((node_)->next_ == (void*)(head_))  // 判断节点是否为首节点
#define ct_list_is_last(head_, node_)  ((node_)->prev_ == (void*)(head_))  // 判断节点是否为尾节点
#define ct_list_next(pos_)              ((pos_)->next_)                      // 获取下一个节点
#define ct_list_prev(pos_)              ((pos_)->prev_)                      // 获取上一个节点

#define ct_list_first_entry(head_, type_, member_) ct_list_entry((head_)->next_, type_, member_)
#define ct_list_last_entry(head_, type_, member_)  ct_list_entry((head_)->prev_, type_, member_)

#define ct_list_next_entry(pos_, type_, member_) \
    !(pos_) ? NULL : ct_list_entry(((ct_list_t*)(&(pos_)->member_))->next_, type_, member_)
#define ct_list_prev_entry(pos_, type_, member_) \
    !(pos_) ? NULL : ct_list_entry(((ct_list_t*)(&(pos_)->member_))->prev_, type_, member_)

// 遍历链表节点
// 	- pos_ 用于遍历的节点
// 	- head_ 指向链表头的链表节点指针
// 在遍历链表时，节点和链表头必须保持不变。对链表进行任何修改将导致未定义行为。
#define ct_list_foreach(pos_, head_) \
    for (ct_list_t* pos_ = (head_)->next_; pos_ != (void*)(head_); pos_ = pos_->next_)

// 遍历链表节点
// 	- pos_ 用于遍历的节点
// 	- head_ 指向链表头的指针
// 	- type_ 包含链表节点的条目的类型
// 	- member_ 在结构体@type中表示链表头的成员变量的名称
// 在遍历链表时，节点和链表头必须保持不变。对链表进行任何修改将导致未定义行为。
#define ct_list_foreach_entry(pos_, head_, type_, member_)            \
    for (type_* pos_ = ct_list_first_entry((head_), type_, member_); \
         pos_ && (void*)(&(pos_)->member_) != (void*)(head_); pos_ = ct_list_next_entry(pos_, type_, member_))

// 遍历链表节点 (从指定位置开始遍历)
// 	- pos_ 用于遍历的起始节点
// 	- head_ 指向链表头的指针
// 	- type_ 包含链表节点的条目的类型
// 	- member_ 在结构体@type中表示链表头的成员变量的名称
// 在遍历链表时，节点和链表头必须保持不变。对链表进行任何修改将导致未定义行为。
#define ct_list_foreach_entry_from(pos_, head_, type_, member_) \
    for (; pos_ && (void*)(&(pos_)->member_) != (void*)(head_); \
         (pos_) = ct_list_next_entry((pos_), type_, member_))

// 遍历链表节点
// 	- pos_ 用于遍历的节点
// 	- head_ 指向链表头的指针
// 	- type_ 包含链表节点的条目的类型
// 	- member_ 在结构体@type中表示链表头的成员变量的名称
// 在遍历链表时，节点和链表头必须保持不变。对链表进行任何修改将导致未定义行为。
// 此宏允许在遍历链表时安全地删除当前节点。
#define ct_list_foreach_entry_safe(pos_, head_, type_, member_)     \
    for (type_* pos_ = ct_list_first_entry(head_, type_, member_), \
                 *next_node_ = ct_list_next_entry(pos_, type_, member_);   \
         pos_ && (void*)(&(pos_)->member_) != (void*)(head_);       \
         pos_ = next_node_, next_node_ = ct_list_next_entry(next_node_, type_, member_))

// 初始化/重置链表
CT_API void ct_list_init(ct_list_t* self);
// 获取链表的大小
CT_API size_t ct_list_size(const ct_list_t* self);

// 在链表头部添加新节点
CT_API void ct_list_append(ct_list_t* self, ct_list_t* node);
// 在链表尾部添加新节点
CT_API void ct_list_prepend(ct_list_t* self, ct_list_t* node);

// 在链表指定节点之前添加新节点
CT_API void ct_list_before(ct_list_t* target, ct_list_t* node);
// 在链表指定节点之后添加新节点
CT_API void ct_list_after(ct_list_t* target, ct_list_t* node);

// 删除链表指定节点
CT_API void ct_list_remove(ct_list_t* node);

// 链表拼接, 将另一个链表的前驱节点作为链表的前驱节点
CT_API void ct_list_splice_prev(ct_list_t* self, ct_list_t* list);
// 链表拼接, 将另一个链表的后继节点作为链表的后继节点
CT_API void ct_list_splice_next(ct_list_t* self, ct_list_t* list);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_LIST_H
