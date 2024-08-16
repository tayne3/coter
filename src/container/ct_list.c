/**
 * @file ct_list.c
 * @brief 链表实现
 * @note
 *  定义了链表的数据结构和相关操作函数，包含链表的初始化、插入、删除、查询等操作。
 *  链表是一种常见的数据结构，用于存储和访问一系列元素。
 *  本文件定义了链表的节点结构体和链表的基本操作函数。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_list.h"

#include <assert.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_list]"

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_list_init(ct_list_buf_t self) {
	assert(self);
	self->prev = self->next = self;
}

bool ct_list_isempty(const ct_list_buf_t self) {
	assert(self);
	return self->next == self;
}

size_t ct_list_size(const ct_list_buf_t self) {
	assert(self);
	size_t size = 0;
	ct_list_foreach (node, self) {
		size++;
	}
	return size;
}

void ct_list_append(ct_list_buf_t self, ct_list_buf_t node) {
	assert(self);
	assert(node);
	node->prev       = self->prev;
	node->next       = self;
	self->prev->next = node;
	self->prev       = node;
}

void ct_list_prepend(ct_list_buf_t self, ct_list_buf_t node) {
	assert(self);
	assert(node);
	node->prev       = self;
	node->next       = self->next;
	self->next->prev = node;
	self->next       = node;
}

void ct_list_before(ct_list_buf_t target, ct_list_buf_t node) {
	assert(target);
	assert(node);
	node->prev         = target->prev;
	node->next         = target;
	target->prev->next = node;
	target->prev       = node;
}

void ct_list_after(ct_list_buf_t target, ct_list_buf_t node) {
	assert(target);
	assert(node);
	node->prev         = target;
	node->next         = target->next;
	target->next->prev = node;
	target->next       = node;
}

void ct_list_remove(ct_list_buf_t node) {
	assert(node);
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

void ct_list_splice_prev(ct_list_buf_t self, ct_list_buf_t list) {
	assert(self);
	assert(list);
	if (ct_list_isempty(list)) {
		return;
	}
	ct_list_last(list)->next  = ct_list_first(self);
	ct_list_first(self)->prev = ct_list_last(list);
	ct_list_first(list)->prev = self;
	ct_list_first(self)       = ct_list_first(list);
	ct_list_init(list);
}

void ct_list_splice_next(ct_list_buf_t self, ct_list_buf_t list) {
	assert(self);
	assert(list);
	if (ct_list_isempty(list)) {
		return;
	}
	ct_list_first(list)->prev = ct_list_last(self);
	ct_list_last(self)->next  = ct_list_first(list);
	ct_list_last(list)->prev  = self;
	ct_list_last(self)        = ct_list_last(list);
	ct_list_init(list);
}

bool ct_list_isfirst(const ct_list_buf_t self, const ct_list_buf_t node) {
	assert(self);
	assert(node);
	return ct_list_first(self) == node;
}

bool ct_list_islast(const ct_list_buf_t self, const ct_list_buf_t node) {
	assert(self);
	assert(node);
	return ct_list_last(self) == node;
}

// -------------------------[STATIC DEFINITION]-------------------------
