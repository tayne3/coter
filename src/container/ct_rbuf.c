/**
 * @file ct_rbuf.c
 * @brief 环形缓冲区实现
 * @note
 *  定义了环形缓冲区结构体和相关操作宏，用于实现环形缓冲区的功能。
 *  环形缓冲区是一种循环队列的实现，可以高效地存储和访问数据。
 *  该文件包含了环形缓冲区的全局声明、宏定义和函数声明。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_rbuf.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_rbuf]"

#define CT_RBUF_INDEX_INC(self, x) ((x) + 1 >= (self)->_max ? 0 : (x) + 1)           // 环形缓冲区-索引 递增
#define CT_RBUF_INDEX_DEC(self, x) ((x) == 0 ? (self)->_max - 1 : (x) - 1)           // 环形缓冲区-索引 递减
#define CT_RBUF_INDEX_CAL(self, x) ((x) >= (self)->_max ? (x) % (self)->_max : (x))  // 环形缓冲区-索引 计算实际值
#define CT_RBUF_DATA(self, index)  (&(self)->_all[(index) * (self)->_byte])          // 环形缓冲区-数据

#if 0
/**
 * @brief 环形缓冲区-遍历
 * @param i 元素索引
 * @param it 元素指针
 * @param pos 元素位置
 * @param type 元素类型
 * @param self 环形缓冲区-变量指针
 */
#define ct_rbuf_foreach(i, it, pos, type, self)                                                    \
	for ((i) = 0, (pos) = (self)->head, (it) = (type *)&(self)->all[(pos)]; (pos) != (self)->tail; \
		 ++(i), (pos) = ct_rbuf_index_inc(self, pos), (it) = (type *)&(self)->all[(pos)])
#endif

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_rbuf_init(ct_rbuf_buf_t self, void *buffer, size_t byte, size_t max) {
	assert(self);
	assert(buffer);
	assert(byte);

	self->_all  = (char *)buffer;
	self->_byte = byte;
	self->_max  = max;

	ct_rbuf_clear(self);
}

bool ct_rbuf_put(ct_rbuf_buf_t self, const void *item) {
	assert(self);
	assert(self->_byte);
	assert(item);

	if (ct_rbuf_isfull(self)) {
		return false;
	}
	if (item) {
		memcpy(CT_RBUF_DATA(self, self->_tail), item, self->_byte);
	}

	self->_tail = CT_RBUF_INDEX_INC(self, self->_tail);
	self->_size++;
	return true;
}

bool ct_rbuf_take(ct_rbuf_buf_t self, void *item) {
	assert(self);
	assert(self->_byte);
	assert(item);

	if (ct_rbuf_isempty(self)) {
		return false;
	}
	if (item) {
		memcpy(item, CT_RBUF_DATA(self, self->_head), self->_byte);
	}

	self->_head = CT_RBUF_INDEX_INC(self, self->_head);
	self->_size--;
	return true;
}

size_t ct_rbuf_puts(ct_rbuf_buf_t self, const void *items, size_t size) {
	assert(self);
	assert(self->_byte);
	assert(items);

	{
		const size_t available = self->_max - self->_size;
		size                   = (size < available) ? size : available;
	}

	if (size == 0) {
		return 0;
	}

	if (self->_tail + size <= self->_max) {
		memcpy(CT_RBUF_DATA(self, self->_tail), items, size * self->_byte);
		self->_tail += size;
	} else {
		const size_t first = self->_max - self->_tail;
		memcpy(CT_RBUF_DATA(self, self->_tail), items, first * self->_byte);
		memcpy(self->_all, &((const char *)items)[first * self->_byte], (size - first) * self->_byte);
		self->_tail = (size - first);
	}

	self->_size += size;
	return size;
}

size_t ct_rbuf_takes(ct_rbuf_buf_t self, void *items, size_t size) {
	assert(self);
	assert(self->_byte);
	assert(items);

	{
		const size_t available = self->_size;
		size                   = (size < available) ? size : available;
	}

	if (size == 0) {
		return 0;
	}

	if (self->_head + size <= self->_max) {
		memcpy(items, CT_RBUF_DATA(self, self->_head), size * self->_byte);
		self->_head += size;
	} else {
		const size_t first = self->_max - self->_head;
		memcpy(items, CT_RBUF_DATA(self, self->_head), first * self->_byte);
		memcpy(&((char *)items)[first * self->_byte], self->_all, (size - first) * self->_byte);
		self->_head = (size - first);
	}

	self->_size -= size;
	return size;
}

size_t ct_rbuf_gets(ct_rbuf_buf_t self, void *items, size_t size) {
	assert(self);
	assert(self->_byte);
	assert(items);

	{
		const size_t available = self->_size;
		size                   = (size < available) ? size : available;
	}

	if (size == 0) {
		return 0;
	}

	if (self->_head + size <= self->_max) {
		memcpy(items, CT_RBUF_DATA(self, self->_head), size * self->_byte);
	} else {
		const size_t first = self->_max - self->_head;
		memcpy(items, CT_RBUF_DATA(self, self->_head), first * self->_byte);
		memcpy(&((char *)items)[first * self->_byte], self->_all, (size - first) * self->_byte);
	}

	return size;
}

size_t ct_rbuf_remove(ct_rbuf_buf_t self, size_t size) {
	assert(self);
	assert(self->_byte);

	if (size > self->_size) {
		size = self->_size;
	}
	self->_head = CT_RBUF_INDEX_CAL(self, self->_head + size);
	self->_size -= size;
	return size;
}

void *ct_rbuf_items(const ct_rbuf_buf_t self, size_t offset, size_t size[1]) {
	assert(self);
	assert(self->_byte);

	if (offset >= self->_size) {
		if (size) {
			*size = 0;
		}
		return NULL;
	}

	if (self->_head + offset < self->_max) {
		if (size) {
			*size = self->_max - self->_head - offset;
			if (*size > self->_size) {
				*size = self->_size;
			}
		}
		return CT_RBUF_DATA(self, self->_head + offset);
	} else {
		if (size) {
			*size = self->_size - offset;
		}
		const size_t first = self->_max - self->_head;
		return CT_RBUF_DATA(self, offset - first);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------
