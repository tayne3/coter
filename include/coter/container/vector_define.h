/**
 * @file vector_define.h
 * @brief 提供用于声明和定义泛型动态数组的宏。
 */
#ifndef COTER_VECTOR_DEFINE_H
#define COTER_VECTOR_DEFINE_H

#include "coter/core/platform.h"
#include "coter/types/any.h"

#ifdef __cplusplus
extern "C" {
#endif

// 动态数组-最大内存限制 (2GB)
#define CT_VECTOR_MEMORY_MAX 0x80000000

/**
 * @brief 声明一个类型安全的动态数组。
 *
 * @details
 * 此宏生成特定类型动态数组的类型定义和函数原型。
 * 通常应在头文件中使用，以便在多个源文件中访问。
 *
 * @param TYPE 动态数组中存储的元素类型。
 * @param NAME 用于生成类型和函数名的唯一标识符。
 *
 * @note 例如 `CT_VECTOR_DECLARE(int, my_int)` 将会生成：
 *       - `ct_vector_my_int_t` 结构体类型。
 *       - `ct_vector_my_int_init`, `ct_vector_my_int_destroy` 等函数的原型。
 */
#define CT_VECTOR_DECLARE(TYPE, NAME)                                                         \
	typedef struct ct_vector_##NAME {                                                         \
		TYPE*  _all;  /* item buffer */                                                       \
		size_t _size; /* current size */                                                      \
		size_t _cap;  /* max size */                                                          \
	} ct_vector_##NAME##_t;                                                                   \
	void        ct_vector_##NAME##_init(ct_vector_##NAME##_t* self, size_t max);              \
	void        ct_vector_##NAME##_destroy(ct_vector_##NAME##_t* self);                       \
	void        ct_vector_##NAME##_clear(ct_vector_##NAME##_t* self);                         \
	size_t      ct_vector_##NAME##_capacity(const ct_vector_##NAME##_t* self);                \
	size_t      ct_vector_##NAME##_size(const ct_vector_##NAME##_t* self);                    \
	bool        ct_vector_##NAME##_empty(const ct_vector_##NAME##_t* self);                   \
	bool        ct_vector_##NAME##_resize(ct_vector_##NAME##_t* self, size_t new_size);       \
	bool        ct_vector_##NAME##_reserve(ct_vector_##NAME##_t* self, size_t capacity);      \
	bool        ct_vector_##NAME##_shrink(ct_vector_##NAME##_t* self);                        \
	bool        ct_vector_##NAME##_insert(ct_vector_##NAME##_t* self, size_t idx, TYPE data); \
	bool        ct_vector_##NAME##_push(ct_vector_##NAME##_t* self, TYPE data);               \
	bool        ct_vector_##NAME##_erase(ct_vector_##NAME##_t* self, size_t idx);             \
	bool        ct_vector_##NAME##_pop(ct_vector_##NAME##_t* self);                           \
	TYPE*       ct_vector_##NAME##_at(ct_vector_##NAME##_t* self, size_t idx);                \
	const TYPE* ct_vector_##NAME##_value(const ct_vector_##NAME##_t* self, size_t idx);       \
	TYPE*       ct_vector_##NAME##_front(ct_vector_##NAME##_t* self);                         \
	TYPE*       ct_vector_##NAME##_back(ct_vector_##NAME##_t* self);

/**
 * @brief 定义一个类型安全的动态数组。
 *
 * @details
 * 此宏生成由 `CT_VECTOR_DECLARE` 声明的动态数组函数的完整实现。
 *
 * @param TYPE 动态数组中存储的元素类型。
 * @param NAME 用于生成函数名的唯一标识符，必须与 `CT_VECTOR_DECLARE` 中使用的 `NAME` 保持一致。
 *
 * @warning 为避免链接时出现重复定义错误，此宏必须且仅能在一个源文件 (.c 文件) 中使用。
 */
#define CT_VECTOR_DEFINE(TYPE, NAME)                                                                 \
	void ct_vector_##NAME##_init(ct_vector_##NAME##_t* self, size_t max) {                           \
		assert(self);                                                                                \
		assert(max > 0);                                                                             \
		assert(sizeof(TYPE) * max <= CT_VECTOR_MEMORY_MAX);                                          \
		self->_all = (TYPE*)calloc(max, sizeof(TYPE));                                               \
		assert(self->_all);                                                                          \
		self->_cap  = max;                                                                           \
		self->_size = 0;                                                                             \
	}                                                                                                \
	void ct_vector_##NAME##_destroy(ct_vector_##NAME##_t* self) {                                    \
		assert(self);                                                                                \
		if (self->_all) {                                                                            \
			free(self->_all);                                                                        \
			self->_all = NULL;                                                                       \
		}                                                                                            \
		self->_size = self->_cap = 0;                                                                \
	}                                                                                                \
	void ct_vector_##NAME##_clear(ct_vector_##NAME##_t* self) {                                      \
		assert(self);                                                                                \
		self->_size = 0;                                                                             \
	}                                                                                                \
	size_t ct_vector_##NAME##_capacity(const ct_vector_##NAME##_t* self) {                           \
		assert(self);                                                                                \
		return self->_cap;                                                                           \
	}                                                                                                \
	size_t ct_vector_##NAME##_size(const ct_vector_##NAME##_t* self) {                               \
		assert(self);                                                                                \
		return self->_size;                                                                          \
	}                                                                                                \
	bool ct_vector_##NAME##_empty(const ct_vector_##NAME##_t* self) {                                \
		assert(self);                                                                                \
		return self->_size == 0;                                                                     \
	}                                                                                                \
	bool ct_vector_##NAME##_resize(ct_vector_##NAME##_t* self, size_t new_size) {                    \
		assert(self);                                                                                \
		if (new_size > self->_cap) {                                                                 \
			if (!ct_vector_##NAME##_reserve(self, new_size)) {                                       \
				return false;                                                                        \
			}                                                                                        \
		}                                                                                            \
		if (new_size > self->_size) {                                                                \
			memset(&self->_all[self->_size], 0, (new_size - self->_size) * sizeof(TYPE));            \
		}                                                                                            \
		self->_size = new_size;                                                                      \
		return true;                                                                                 \
	}                                                                                                \
	bool ct_vector_##NAME##_reserve(ct_vector_##NAME##_t* self, size_t capacity) {                   \
		assert(self);                                                                                \
		if (self->_cap >= capacity) {                                                                \
			return true;                                                                             \
		}                                                                                            \
		size_t new_cap = self->_cap == 0 ? 1 : self->_cap;                                           \
		while (new_cap < capacity) {                                                                 \
			new_cap *= 2;                                                                            \
		}                                                                                            \
		if (sizeof(TYPE) * new_cap > CT_VECTOR_MEMORY_MAX) {                                         \
			return false; /* out of memory */                                                        \
		}                                                                                            \
		TYPE* new_all = (TYPE*)realloc(self->_all, sizeof(TYPE) * new_cap);                          \
		if (!new_all) {                                                                              \
			return false; /* realloc failed */                                                       \
		}                                                                                            \
		self->_all = new_all;                                                                        \
		self->_cap = new_cap;                                                                        \
		return true;                                                                                 \
	}                                                                                                \
	bool ct_vector_##NAME##_shrink(ct_vector_##NAME##_t* self) {                                     \
		assert(self);                                                                                \
		if (self->_size == self->_cap) {                                                             \
			return true;                                                                             \
		}                                                                                            \
		if (self->_size == 0) {                                                                      \
			free(self->_all);                                                                        \
			self->_all = NULL;                                                                       \
			self->_cap = 0;                                                                          \
			return true;                                                                             \
		}                                                                                            \
		TYPE* new_all = (TYPE*)realloc(self->_all, sizeof(TYPE) * self->_size);                      \
		if (!new_all) {                                                                              \
			return false;                                                                            \
		}                                                                                            \
		self->_all = new_all;                                                                        \
		self->_cap = self->_size;                                                                    \
		return true;                                                                                 \
	}                                                                                                \
	bool ct_vector_##NAME##_insert(ct_vector_##NAME##_t* self, size_t idx, TYPE data) {              \
		assert(self);                                                                                \
		if (idx > self->_size) {                                                                     \
			return false;                                                                            \
		}                                                                                            \
		if (!ct_vector_##NAME##_reserve(self, self->_size + 1)) {                                    \
			return false;                                                                            \
		}                                                                                            \
		if (idx < self->_size) {                                                                     \
			memmove(&self->_all[idx + 1], &self->_all[idx], (self->_size - idx) * sizeof(TYPE));     \
		}                                                                                            \
		self->_all[idx] = data;                                                                      \
		++self->_size;                                                                               \
		return true;                                                                                 \
	}                                                                                                \
	bool ct_vector_##NAME##_push(ct_vector_##NAME##_t* self, TYPE data) {                            \
		assert(self);                                                                                \
		if (self->_size >= self->_cap) {                                                             \
			if (!ct_vector_##NAME##_reserve(self, self->_size + 1)) {                                \
				return false;                                                                        \
			}                                                                                        \
		}                                                                                            \
		self->_all[self->_size++] = data;                                                            \
		return true;                                                                                 \
	}                                                                                                \
	bool ct_vector_##NAME##_erase(ct_vector_##NAME##_t* self, size_t idx) {                          \
		assert(self);                                                                                \
		if (idx >= self->_size) {                                                                    \
			return false;                                                                            \
		}                                                                                            \
		if (idx < self->_size - 1) {                                                                 \
			memmove(&self->_all[idx], &self->_all[idx + 1], (self->_size - idx - 1) * sizeof(TYPE)); \
		}                                                                                            \
		--self->_size;                                                                               \
		return true;                                                                                 \
	}                                                                                                \
	bool ct_vector_##NAME##_pop(ct_vector_##NAME##_t* self) {                                        \
		assert(self);                                                                                \
		if (self->_size == 0) {                                                                      \
			return false;                                                                            \
		}                                                                                            \
		--self->_size;                                                                               \
		return true;                                                                                 \
	}                                                                                                \
	TYPE* ct_vector_##NAME##_at(ct_vector_##NAME##_t* self, size_t idx) {                            \
		assert(self);                                                                                \
		assert(self->_all);                                                                          \
		return idx >= self->_size ? NULL : &self->_all[idx];                                         \
	}                                                                                                \
	const TYPE* ct_vector_##NAME##_value(const ct_vector_##NAME##_t* self, size_t idx) {             \
		assert(self);                                                                                \
		assert(self->_all);                                                                          \
		return idx >= self->_size ? NULL : &self->_all[idx];                                         \
	}                                                                                                \
	TYPE* ct_vector_##NAME##_front(ct_vector_##NAME##_t* self) {                                     \
		assert(self);                                                                                \
		assert(self->_all);                                                                          \
		return self->_size == 0 ? NULL : &self->_all[0];                                             \
	}                                                                                                \
	TYPE* ct_vector_##NAME##_back(ct_vector_##NAME##_t* self) {                                      \
		assert(self);                                                                                \
		assert(self->_all);                                                                          \
		return self->_size == 0 ? NULL : &self->_all[self->_size - 1];                               \
	}

#ifdef __cplusplus
}
#endif
#endif  // COTER_VECTOR_DEFINE_H
