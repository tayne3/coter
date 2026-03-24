/**
 * @file list.c
 * @brief 链表实现
 * @note
 *  定义了链表的数据结构和相关操作函数，包含链表的初始化、插入、删除、查询等操作。
 *  链表是一种常见的数据结构，用于存储和访问一系列元素。
 *  本文件定义了链表的节点结构体和链表的基本操作函数。
 */
#include "coter/container/list.h"

void ct_list_init(ct_list_t* self) {
    if (!self) { return; }
    self->prev = self->next = self;
}

size_t ct_list_size(const ct_list_t* self) {
    if (!self) { return 0; }
    size_t size = 0;
    ct_list_foreach(node, self) {
        size++;
    }
    return size;
}

void ct_list_append(ct_list_t* self, ct_list_t* node) {
    if (!self || !node) { return; }
    node->prev       = self->prev;
    node->next       = self;
    self->prev->next = node;
    self->prev       = node;
}

void ct_list_prepend(ct_list_t* self, ct_list_t* node) {
    if (!self || !node) { return; }
    node->prev       = self;
    node->next       = self->next;
    self->next->prev = node;
    self->next       = node;
}

void ct_list_before(ct_list_t* target, ct_list_t* node) {
    if (!target || !node) { return; }
    node->prev         = target->prev;
    node->next         = target;
    target->prev->next = node;
    target->prev       = node;
}

void ct_list_after(ct_list_t* target, ct_list_t* node) {
    if (!target || !node) { return; }
    node->prev         = target;
    node->next         = target->next;
    target->next->prev = node;
    target->next       = node;
}

void ct_list_remove(ct_list_t* node) {
    if (!node) { return; }
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void ct_list_splice_prev(ct_list_t* self, ct_list_t* list) {
    if (!self || !list) { return; }
    if (ct_list_isempty(list)) { return; }
    ct_list_last(list)->next  = ct_list_first(self);
    ct_list_first(self)->prev = ct_list_last(list);
    ct_list_first(list)->prev = self;
    ct_list_first(self)       = ct_list_first(list);
    ct_list_init(list);
}

void ct_list_splice_next(ct_list_t* self, ct_list_t* list) {
    if (!self || !list) { return; }
    if (ct_list_isempty(list)) { return; }
    ct_list_first(list)->prev = ct_list_last(self);
    ct_list_last(self)->next  = ct_list_first(list);
    ct_list_last(list)->next  = self;
    ct_list_last(self)        = ct_list_last(list);
    ct_list_init(list);
}
