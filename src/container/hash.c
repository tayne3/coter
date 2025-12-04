/**
 * @file ct_hash.c
 * @brief 哈希表实现
 * @note
 * 哈希表是一种根据关键字直接访问数据元素的数据结构, 适用于要求高效检索的场景。
 *
 * 哈希表的优点是平均查询和插入时间都是常数时间O(1),这比直接遍历数组或链表等序列结构效率要高很多。
 * 此外,它还可以很方便实现根据关键字进行插入、删除和查找数据的操作。
 *
 * 名词解释:
 *  - 压缩映射:
 * 		通过算法将不定长的关键字转换为固定长度的索引值, 但索引位置的空间通常小于可能的键值空间, 这样就会产生碰撞。
 *  - 碰撞:
 * 		当两个键通过哈希函数映射到同一个索引位置时就会产生碰撞冲突。
 *  - 拉链法:
 * 		将可能发生碰撞的位置建立链表结构,同一个位置可能映射多个键值对,通过迭代链表查找目标键值对。这种方法利用指针记录冲突数据,查询速度较慢但利用空间小。
 */
#include "coter/container/hash.h"

#include "coter/algo/hashalgo.h"
#include "coter/base/platform.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define CT_HASH_SIZE(self)    ((self)->size)                 // 获取 哈希表 大小
#define CT_HASH_MAX(self)     ((self)->max)                  // 获取 哈希表 最大容量
#define CT_HASH_ISEMPTY(self) ((self)->size == 0)            // 判断 哈希表 是否为空
#define CT_HASH_ISFULL(self)  ((self)->size >= (self)->max)  // 判断 哈希表 是否已满
#define CT_HASH_DATA(self, i) ((self)->all[i])               // 获取 哈希表 指定元素

#define CT_HASH_DEFAULT_MAX               8                                           // 默认容量
#define CT_HASH_MEMORY_MAX                0x80000000                                  // 最大内存限制
#define CT_HASH_HASHALGO(key, key_length) ct_hashalgo_times33((key), (key_length))    // 根据key生成哈希值
#define CT_HASH_INDEX(self, key_hash)     (((size_t)(key_hash)) % CT_HASH_MAX(self))  // 根据哈希值生成索引值

// 哈希表-扩容
static inline bool ct_hash_resize(ct_hash_buf_t self, size_t max);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_hash_init(ct_hash_buf_t self) {
	if (!self) {
		return;
	}

	// 申请内存
	self->all = (ct_hash_pair_t **)calloc(CT_HASH_DEFAULT_MAX, sizeof(ct_hash_pair_t *));
	if (!self->all) {
		return;
	}

	self->methods      = ct_any_methods_default;
	self->max          = CT_HASH_DEFAULT_MAX;
	self->size         = 0;
	self->allow_resize = true;
}

void ct_hash_init_s(ct_hash_buf_t self, size_t max, bool allow_resize, ct_any_methods_t methods) {
	if (!self || !max) {
		return;
	}

	if (max > CT_HASH_MEMORY_MAX) {
		max = CT_HASH_MEMORY_MAX;
	}

	// 申请内存
	self->all = (ct_hash_pair_t **)calloc(max, sizeof(ct_hash_pair_t *));
	if (!self->all) {
		return;
	}

	self->methods      = methods;
	self->max          = max;
	self->size         = 0;
	self->allow_resize = allow_resize;
}

void ct_hash_destroy(ct_hash_buf_t self) {
	if (!self) {
		return;
	}

	// 清空元素
	ct_hash_clear(self);
	// 释放内存
	if (self->all) {
		free(self->all);
		self->all = NULL;
	}
}

void ct_hash_reserve(ct_hash_buf_t self, size_t max) {
	if (!self) {
		return;
	}
	if (self->max < max && max < CT_HASH_MEMORY_MAX) {
		ct_hash_resize(self, max);
	}
}

size_t ct_hash_size(const ct_hash_buf_t self) {
	return CT_HASH_SIZE(self);
}

bool ct_hash_isempty(const ct_hash_buf_t self) {
	return CT_HASH_ISEMPTY(self);
}

bool ct_hash_contains(const ct_hash_buf_t self, const char *key) {
	if (!self) {
		return false;
	}

	if (STR_ISEMPTY(key) || CT_HASH_ISEMPTY(self)) {
		return false;
	}

	const size_t key_length = strlen(key);

	if (!key_length) {
		return false;
	}

	// 生成索引值
	const ct_hash32_t key_hash   = CT_HASH_HASHALGO(key, key_length);
	const size_t      hash_index = CT_HASH_INDEX(self, key_hash);
	// 获取指定索引的元素
	const ct_hash_pair_t *pos = CT_HASH_DATA(self, hash_index);

	// 遍历链表, 检查key是否已经存在
	for (const ct_hash_pair_t *it = pos; it; it = it->next) {
		if (it->key_hash != key_hash || key_length != it->key_length || strncmp(key, it->key, key_length) != 0) {
			continue;
		}
		return true;
	}

	return false;
}

bool ct_hash_insert(ct_hash_buf_t self, const char *key, ct_any_t value) {
	if (!self) {
		return false;
	}
	if (STR_ISEMPTY(key)) {
		return false;
	}

	const size_t key_length = strlen(key);

	if (!key_length) {
		return false;
	}

	if (CT_HASH_ISFULL(self)) {
		if (!self->allow_resize) {
			return false;
		} else if (self->max >= CT_HASH_MEMORY_MAX) {
			return false;
		} else {
			size_t max = self->max * 2;
			if (max > CT_HASH_MEMORY_MAX) {
				max = CT_HASH_MEMORY_MAX;
			}
			ct_hash_resize(self, max);
		}
	}

	// 生成索引值
	const ct_hash32_t key_hash   = CT_HASH_HASHALGO(key, key_length);
	const size_t      hash_index = CT_HASH_INDEX(self, key_hash);
	// 获取指定索引的元素
	ct_hash_pair_t *pos = CT_HASH_DATA(self, hash_index);

	// 遍历链表, 检查key是否已经存在
	for (ct_hash_pair_t *it = pos; it; it = it->next) {
		if (it->key_hash != key_hash || key_length != it->key_length || strncmp(key, it->key, key_length) != 0) {
			continue;
		}
		ct_any_update(&self->methods, &it->value, &value);
		return true;
	}

	ct_hash_pair_t *new_pair = (ct_hash_pair_t *)malloc(sizeof(ct_hash_pair_t) + key_length);
	if (!new_pair) {
		return false;
	}

	ct_any_ctor(&self->methods, &new_pair->value, &value);
	strncpy(new_pair->key, key, key_length + 1);
	new_pair->key_length           = key_length;
	new_pair->key_hash             = key_hash;
	new_pair->next                 = pos;
	CT_HASH_DATA(self, hash_index) = new_pair;
	self->size++;
	return true;
}

bool ct_hash_remove(ct_hash_buf_t self, const char *key) {
	if (!self) {
		return false;
	}

	do {
		if (STR_ISEMPTY(key) || CT_HASH_ISEMPTY(self)) {
			break;
		}

		const size_t key_length = strlen(key);

		if (!key_length) {
			break;
		}

		// 生成索引值
		const ct_hash32_t key_hash   = CT_HASH_HASHALGO(key, key_length);
		const size_t      hash_index = CT_HASH_INDEX(self, key_hash);
		// 获取指定索引的元素
		ct_hash_pair_t *pos = CT_HASH_DATA(self, hash_index);

		// 遍历链表, 检查key是否已经存在
		for (ct_hash_pair_t *it = pos, *prev = NULL; it; prev = it, it = it->next) {
			if (it->key_hash != key_hash || key_length != it->key_length || strncmp(key, it->key, key_length) != 0) {
				continue;
			}
			if (prev) {
				prev->next = it->next;
			} else {
				CT_HASH_DATA(self, hash_index) = it->next;
			}
			ct_any_dtor(&self->methods, &it->value);
			self->size--;
			free(it);
			return true;
		}

	} while (0);

	return false;
}

void ct_hash_clear(ct_hash_buf_t self) {
	if (!self) {
		return;
	}

	ct_hash_pair_t *it, *pos;

	for (size_t i = 0; i < self->max && self->size > 0; i++) {
		pos = CT_HASH_DATA(self, i);
		if (!pos) {
			continue;
		}
		// 置空
		CT_HASH_DATA(self, i) = NULL;

		for (it = pos; it; it = pos) {
			if (pos) {
				pos = pos->next;
			}
			ct_any_dtor(&self->methods, &it->value);
			self->size--;
			free(it);
		}
	}
}

ct_any_t ct_hash_value(ct_hash_buf_t self, const char *key) {
	if (!self) {
		return ct_any_null;
	}

	do {
		if (STR_ISEMPTY(key) || CT_HASH_ISEMPTY(self)) {
			break;
		}

		const size_t key_length = strlen(key);

		if (!key_length) {
			break;
		}

		// 生成索引值
		const ct_hash32_t key_hash   = CT_HASH_HASHALGO(key, key_length);
		const size_t      hash_index = CT_HASH_INDEX(self, key_hash);
		// 获取指定索引的元素
		ct_hash_pair_t *pos = CT_HASH_DATA(self, hash_index);

		// 遍历链表, 检查key是否已经存在
		for (ct_hash_pair_t *it = pos; it; it = it->next) {
			if (it->key_hash != key_hash || key_length != it->key_length || strncmp(key, it->key, key_length) != 0) {
				continue;
			}
			return it->value;
		}
	} while (0);

	return ct_any_null;
}

bool ct_hash_value_r(ct_hash_buf_t self, const char *key, ct_any_t *value) {
	if (!self) {
		return false;
	}

	bool is_ok = false;

	do {
		if (STR_ISEMPTY(key) || CT_HASH_ISEMPTY(self)) {
			break;
		}

		const size_t key_length = strlen(key);

		if (!key_length) {
			break;
		}

		// 生成索引值
		const ct_hash32_t key_hash   = CT_HASH_HASHALGO(key, key_length);
		const size_t      hash_index = CT_HASH_INDEX(self, key_hash);
		// 获取指定索引的元素
		ct_hash_pair_t *pos = CT_HASH_DATA(self, hash_index);

		// 遍历链表, 检查key是否已经存在
		for (ct_hash_pair_t *it = pos; it; it = it->next) {
			if (it->key_hash != key_hash || key_length != it->key_length || strncmp(key, it->key, key_length) != 0) {
				continue;
			}
			if (value) {
				*value = it->value;
			}
			is_ok = true;
			break;
		}
	} while (0);

	if (!is_ok && value) {
		*value = ct_any_null;
	}
	return is_ok;
}

ct_any_t ct_hash_value_s(ct_hash_buf_t self, const char *key, ct_any_t default_value) {
	if (!self) {
		return ct_any_null;
	}

	do {
		if (STR_ISEMPTY(key) || CT_HASH_ISEMPTY(self)) {
			break;
		}

		const size_t key_length = strlen(key);

		if (!key_length) {
			break;
		}

		// 生成索引值
		const ct_hash32_t key_hash   = CT_HASH_HASHALGO(key, key_length);
		const size_t      hash_index = CT_HASH_INDEX(self, key_hash);
		// 获取指定索引的元素
		ct_hash_pair_t *pos = CT_HASH_DATA(self, hash_index);

		// 遍历链表, 检查key是否已经存在
		for (ct_hash_pair_t *it = pos; it; it = it->next) {
			if (it->key_hash != key_hash || key_length != it->key_length || strncmp(key, it->key, key_length) != 0) {
				continue;
			}
			return it->value;
		}
	} while (0);

	return default_value;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool ct_hash_resize(ct_hash_buf_t self, size_t max) {
	if (!self || !self->all) {
		return false;
	}

	ct_hash_pair_t **const old_all = self->all;
	const size_t           old_max = self->max;

	// 申请新的内存
	{
		ct_hash_pair_t **new_all;

		new_all = (ct_hash_pair_t **)calloc(max, sizeof(ct_hash_pair_t *));
		if (!new_all) {
			return false;
		}

		self->all  = new_all;
		self->max  = max;
		self->size = 0;
	}

	// 将所有元素重新插入到新的哈希表
	{
		ct_hash_pair_t *it, *pos;

		for (size_t i = 0, hash_index; i < old_max; i++) {
			pos = old_all[i];
			if (!pos) {
				self->all[i] = NULL;
				continue;
			}
			for (it = pos; it; it = pos) {
				if (pos) {
					pos = pos->next;
				}
				hash_index                     = CT_HASH_INDEX(self, it->key_hash);
				it->next                       = CT_HASH_DATA(self, hash_index);
				CT_HASH_DATA(self, hash_index) = it;
				self->size++;
			}
		}
	}

	// 释放旧的哈希表
	free(old_all);
	return true;
}
