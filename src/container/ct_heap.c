/**
 * @file ct_heap.c
 * @brief 堆实现
 * @note
 * 	定义了堆的数据结构和相关操作函数，包含堆的初始化、插入、删除、查询等操作。
 * 	堆是一种特殊的完全二叉树结构，可以用于高效地维护一组元素，并支持快速的插入和删除操作。
 * 	本文件定义了堆的结构体和基本操作函数。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_heap.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_heap]"

#define CT_HEAP_INDEX_PARENT(i)     ((i - 1) >> 1)                                     // 获取父节点的索引
#define CT_HEAP_INDEX_LEFT(i)       ((i << 1) + 1)                                     // 获取左子节点的索引
#define CT_HEAP_INDEX_RIGHT(i)      ((i + 1) << 1)                                     // 获取右子节点的索引
#define CT_HEAP_COMPARE(self, l, r) (self)->_sort(&(self)->_all[l], &(self)->_all[r])  // 排序比较函数
#define CT_HEAP_SWAP(self, l, r)    ct_any_swap(&(self)->_all[l], &(self)->_all[r])    // 交换两个元素的位置

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_heap_init(ct_heap_buf_t self, ct_any_t* all, size_t max, ct_heap_sort_t sort) {
	self->_all  = all;
	self->_sort = sort;
	self->_size = 0;
	self->_max  = max;
}

void ct_heap_up(ct_heap_buf_t self, size_t i) {
	size_t parent = CT_HEAP_INDEX_PARENT(i);
	for (; i > 0 && CT_HEAP_COMPARE(self, i, parent);) {
		CT_HEAP_SWAP(self, i, parent);
		i      = parent;
		parent = CT_HEAP_INDEX_PARENT(i);
	}
}

void ct_heap_down(ct_heap_buf_t self, size_t i) {
	size_t index_min   = i;
	size_t index_left  = CT_HEAP_INDEX_LEFT(index_min);
	size_t index_right = CT_HEAP_INDEX_RIGHT(index_min);

	ct_forever {
		if (index_left < self->_size && CT_HEAP_COMPARE(self, index_left, i)) {
			i = index_left;
		}
		if (index_right < self->_size && CT_HEAP_COMPARE(self, index_right, i)) {
			i = index_right;
		}
		if (i == index_min) {
			break;
		}
		// 交换位置
		CT_HEAP_SWAP(self, index_min, i);
		index_min   = i;
		index_left  = CT_HEAP_INDEX_LEFT(index_min);
		index_right = CT_HEAP_INDEX_RIGHT(index_min);
	}
}

void ct_heap_reorder(ct_heap_buf_t self) {
	// 自底向上构建堆
	for (size_t i = self->_size / 2; i > 0; i--) {
		ct_heap_down(self, i);
	}
}

void ct_heap_insert(ct_heap_buf_t self, ct_any_t data) {
	if (self->_size >= self->_max) {
		return;
	}
	self->_all[self->_size] = data;
	ct_heap_up(self, self->_size);
	self->_size++;
}

void ct_heap_remove(ct_heap_buf_t self) {
	// 判断堆是否为空
	if (ct_heap_isempty(self)) {
		return;
	}

	self->_size--;

	if (!ct_heap_isempty(self)) {
		self->_all[0] = self->_all[self->_size];
		ct_heap_down(self, 0);
	}
}

ct_any_t ct_heap_take(ct_heap_buf_t self) {
	if (ct_heap_isempty(self)) {
		return ct_any_null;
	}

	ct_any_t result = self->_all[0];

	self->_size--;

	if (!ct_heap_isempty(self)) {
		self->_all[0] = self->_all[self->_size];
		ct_heap_down(self, 0);
	}
	return result;
}

bool ct_heap_copy(ct_heap_buf_t self, ct_heap_t* target) {
	// 检查容量
	if (target->_max < self->_size) {
		return false;
	}
	// 拷贝所有元素
	for (size_t i = 0; i < self->_size; i++) {
		target->_all[i] = self->_all[i];
	}
	target->_size = self->_size;
	target->_sort = self->_sort;
	return true;
}

// -------------------------[STATIC DEFINITION]-------------------------
