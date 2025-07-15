/**
 * @file ct_stack.c
 * @brief 栈实现
 * @note
 *  定义了栈的数据结构和相关操作函数，包含栈的初始化、入栈、出栈、查询等操作。
 *  栈是一种后进先出（LIFO）的数据结构，可以用于存储和访问一系列元素。
 *  本文件定义了栈的结构体和基本操作函数。
 */
#include "coter/container/stack.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_stack_init(ct_stack_buf_t self, ct_any_t* buffer, size_t max) {
	assert(self);
	self->_all = buffer;
	self->_max = max;
	ct_stack_clear(self);
}

bool ct_stack_push(ct_stack_buf_t self, ct_any_t item) {
	assert(self);
	if (ct_stack_isfull(self)) {
		return false;
	}
	self->_all[self->_size] = item;
	self->_size++;
	return true;
}

bool ct_stack_pop(ct_stack_buf_t self, ct_any_t* item) {
	assert(self);
	if (ct_stack_isempty(self)) {
		return false;
	}
	self->_size--;
	if (item) {
		*item = self->_all[self->_size];
	}
	return true;
}

bool ct_stack_top(ct_stack_buf_t self, ct_any_t* item) {
	assert(self);
	if (ct_stack_isempty(self)) {
		return false;
	}
	if (item) {
		*item = self->_all[self->_size - 1];
	}
	return true;
}

// -------------------------[STATIC DEFINITION]-------------------------
