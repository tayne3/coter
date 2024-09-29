/**
 * @file ct_queue.c
 * @brief 队列实现
 * @note
 * 	定义了队列的数据结构和相关操作函数,包含队列的初始化、插入、删除、查询等操作。
 * 	队列是一种先进先出（FIFO）的数据结构，可以用于存储和访问一系列元素。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_queue.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define CT_QUEUE_INDEX_INC(self, x) ((x) + 1 >= (self)->_max ? 0 : (x) + 1)  // 队列-索引 递增
#define CT_QUEUE_INDEX_DEC(self, x) ((x) == 0 ? (self)->_max - 1 : (x) - 1)  // 队列-索引 递减
#define CT_QUEUE_ITEM(self, idx)    (&(self)->_all[(idx) * (self)->_byte])   // 队列-数据

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_queue_init(ct_queue_buf_t self, void* buffer, size_t byte, size_t max) {
	assert(self);
	assert(buffer);
	assert(byte);

	self->_all  = buffer;
	self->_byte = byte;
	self->_max  = max;

	ct_queue_clear(self);
}

bool ct_queue_enqueue(ct_queue_buf_t self, const void* item) {
	assert(self);
	assert(self->_byte);
	assert(item);

	if (ct_queue_isfull(self)) {
		return false;
	}
	memcpy(CT_QUEUE_ITEM(self, self->_tail), item, self->_byte);
	self->_tail = CT_QUEUE_INDEX_INC(self, self->_tail);
	self->_size++;
	return true;
}

bool ct_queue_dequeue(ct_queue_buf_t self, void* item) {
	assert(self);
	assert(self->_byte);
	assert(item);

	if (ct_queue_isempty(self)) {
		return false;
	}
	memcpy(item, CT_QUEUE_ITEM(self, self->_head), self->_byte);
	self->_head = CT_QUEUE_INDEX_INC(self, self->_head);
	self->_size--;
	return true;
}

bool ct_queue_head(ct_queue_buf_t self, void* item) {
	assert(self);
	assert(self->_byte);
	assert(item);

	if (ct_queue_isempty(self)) {
		return false;
	}
	memcpy(item, CT_QUEUE_ITEM(self, self->_head), self->_byte);
	return true;
}

int ct_queue_traverse(ct_queue_buf_t self, int (*callback)(void* item, void* arg), void* item, void* arg) {
	assert(self);
	assert(self->_byte);
	assert(callback);

	size_t idx = self->_head;
	for (size_t i = 0; i < self->_size; i++) {
		memcpy(item, CT_QUEUE_ITEM(self, idx), self->_byte);
		const int ret = callback(item, arg);
		if (ret) {
			return ret;
		}
		idx = CT_QUEUE_INDEX_INC(self, idx);
	}
	return 0;
}

// -------------------------[STATIC DEFINITION]-------------------------
