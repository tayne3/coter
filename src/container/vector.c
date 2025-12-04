/**
 * @file ct_vector.c
 * @brief 动态数组实现
 */
#include "coter/container/vector.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define CT_VECTOR_ITEM(self, idx) (&(self)->_all[(idx) * (self)->_byte])  // 动态数组-指定元素
#define CT_VECTOR_CAP(self)       ((self)->_cap)                          // 动态数组-最大容量
#define CT_VECTOR_SIZE(self)      ((self)->_size)                         // 动态数组-当前大小
#define CT_VECTOR_MEMORY_MAX      0x80000000                              // 动态数组-最大内存限制

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_vector_init(ct_vector_t* self, size_t byte, size_t capacity) {
	if (!self) {
		return;
	}
	if (!byte) {
		return;
	}
	if (byte * capacity > CT_VECTOR_MEMORY_MAX) {
		return;
	}
	self->_all = calloc(capacity, byte);
	if (!self->_all) {
		return;
	}
	self->_byte = byte;
	self->_cap  = capacity;
	self->_size = 0;
}

void ct_vector_destroy(ct_vector_t* self) {
	if (!self) {
		return;
	}
	if (self->_all) {
		free(self->_all);
		self->_all = NULL;
	}
	self->_byte = self->_cap = self->_size = 0;
}

void ct_vector_clear(ct_vector_t* self) {
	if (!self) {
		return;
	}
	self->_size = 0;
}

size_t ct_vector_capacity(const ct_vector_t* self) {
	if (!self) {
		return 0;
	}
	return CT_VECTOR_CAP(self);
}

size_t ct_vector_size(const ct_vector_t* self) {
	if (!self) {
		return 0;
	}
	return CT_VECTOR_SIZE(self);
}

bool ct_vector_empty(const ct_vector_t* self) {
	if (!self) {
		return true;
	}
	return CT_VECTOR_SIZE(self) == 0;
}

bool ct_vector_resize(ct_vector_t* self, size_t new_size) {
	if (!self) {
		return false;
	}
	if (new_size > self->_cap) {
		if (!ct_vector_reserve(self, new_size)) {
			return false;
		}
	}
	if (new_size > self->_size) {
		memset(CT_VECTOR_ITEM(self, self->_size), 0, (new_size - self->_size) * self->_byte);
	}
	self->_size = new_size;
	return true;
}

bool ct_vector_reserve(ct_vector_t* self, size_t capacity) {
	if (!self) {
		return false;
	}
	if (self->_cap >= capacity) {
		return true;
	}
	size_t new_cap = self->_cap == 0 ? 1 : self->_cap;
	while (new_cap < capacity) {
		new_cap *= 2;
	}
	if (self->_byte * new_cap > CT_VECTOR_MEMORY_MAX) {
		return false; /* out of memory */
	}
	char* new_all = realloc(self->_all, self->_byte * new_cap);
	if (!new_all) {
		return false; /* realloc failed */
	}
	self->_all = new_all;
	self->_cap = new_cap;
	return true;
}

bool ct_vector_shrink(ct_vector_t* self) {
	if (!self) {
		return false;
	}
	if (self->_cap == self->_size) {
		return true;
	}
	if (self->_size == 0) {
		free(self->_all);
		self->_all = NULL;
		self->_cap = 0;
		return true;
	}
	char* new_all = realloc(self->_all, self->_byte * self->_size);
	if (!new_all) {
		return false;
	}
	self->_all = new_all;
	self->_cap = self->_size;
	return true;
}

bool ct_vector_insert(ct_vector_t* self, size_t idx, const void* data) {
	if (!self) {
		return false;
	}
	if (idx > self->_size) {
		return false;
	}
	if (!ct_vector_reserve(self, self->_size + 1)) {
		return false;
	}
	if (idx < self->_size) {
		memmove(CT_VECTOR_ITEM(self, idx + 1), CT_VECTOR_ITEM(self, idx), (self->_size - idx) * self->_byte);
	}
	memcpy(CT_VECTOR_ITEM(self, idx), data, self->_byte);
	++self->_size;
	return true;
}

bool ct_vector_push(ct_vector_t* self, const void* data) {
	if (!self) {
		return false;
	}
	if (self->_size >= self->_cap) {
		if (!ct_vector_reserve(self, self->_size + 1)) {
			return false;
		}
	}
	memcpy(CT_VECTOR_ITEM(self, self->_size), data, self->_byte);
	++self->_size;
	return true;
}

bool ct_vector_erase(ct_vector_t* self, size_t idx) {
	if (!self) {
		return false;
	}
	if (idx >= self->_size) {
		return false;
	}
	if (idx < self->_size - 1) {
		memmove(CT_VECTOR_ITEM(self, idx), CT_VECTOR_ITEM(self, idx + 1), (self->_size - idx - 1) * self->_byte);
	}
	--self->_size;
	return true;
}

bool ct_vector_pop(ct_vector_t* self) {
	if (!self) {
		return false;
	}
	if (self->_size == 0) {
		return false;
	}
	--self->_size;
	return true;
}

void* ct_vector_at(ct_vector_t* self, size_t idx) {
	if (!self) {
		return NULL;
	}
	return idx >= self->_size ? NULL : CT_VECTOR_ITEM(self, idx);
}

const void* ct_vector_value(const ct_vector_t* self, size_t idx) {
	if (!self) {
		return NULL;
	}
	return idx >= self->_size ? NULL : CT_VECTOR_ITEM(self, idx);
}

void* ct_vector_front(ct_vector_t* self) {
	if (!self) {
		return NULL;
	}
	return self->_size == 0 ? NULL : CT_VECTOR_ITEM(self, 0);
}

void* ct_vector_back(ct_vector_t* self) {
	if (!self) {
		return NULL;
	}
	return self->_size == 0 ? NULL : CT_VECTOR_ITEM(self, self->_size - 1);
}

// -------------------------[STATIC DEFINITION]-------------------------
