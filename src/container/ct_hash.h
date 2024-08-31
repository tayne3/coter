/**
 * @file ct_hash.h
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
 * @author tayne3@dingtalk.com
 * @date 2023.12.27
 */
#ifndef _CT_HASH_H
#define _CT_HASH_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_any.h"
#include "base/ct_platform.h"

// 哈希值类型
typedef uint32_t ct_hash32_t;

struct ct_hash_pair;

/**
 * @brief 哈希表-键值对
 */
typedef struct ct_hash_pair {
	struct ct_hash_pair *next;        // 下一个
	ct_any_buf_t         value;       // 值
	ct_hash32_t          key_hash;    // 键的哈希值
	size_t               key_length;  // 键的长度
	char                 key[1];      // 键
} ct_hash_pair_t, ct_hash_pair_buf_t[1];

/**
 * @brief 哈希表结构体
 */
typedef struct ct_hash {
	ct_hash_pair_t **all;           // 键值对数组
	ct_any_methods_t methods;       // Any函数组
	size_t           max;           // 最大容量
	size_t           size;          // 元素数量
	bool             allow_resize;  // 是否能够扩容
} ct_hash_t, ct_hash_buf_t[1];

// 调整是否能够扩容
#define ct_hash_set_allow_resize(self, allow) \
	do {                                      \
		((self)->allow_resize = (allow));     \
	} while (0)

/**
 * @brief 初始化哈希表
 * @param self 哈希表指针
 */
CT_API void ct_hash_init(ct_hash_buf_t self) __ct_nonnull(1);

/**
 * @brief 初始化哈希表
 * @param self 哈希表指针
 * @param max 最大容量
 * @param allow_resize 是否能够扩容
 * @param methods Any函数组
 */
CT_API void ct_hash_init_s(ct_hash_buf_t self, size_t max, bool allow_resize, ct_any_methods_t methods) __ct_nonnull(1);

/**
 * @brief 销毁哈希表
 * @param self 哈希表指针
 */
CT_API void ct_hash_destroy(ct_hash_buf_t self) __ct_nonnull(1);

/**
 * @brief 扩容哈希表
 * @param self 哈希表指针
 * @param max 最大容量
 */
CT_API void ct_hash_reserve(ct_hash_buf_t self, size_t max) __ct_nonnull(1);

/**
 * @brief 获取元素数量
 * @param self 哈希表指针
 * @return 元素数量
 */
CT_API size_t ct_hash_size(const ct_hash_buf_t self) __ct_nonnull(1);

/**
 * @brief 检查哈希表是否为空
 * @param self 哈希表指针
 * @return 是否为空
 */
CT_API bool ct_hash_isempty(const ct_hash_buf_t self) __ct_nonnull(1);

/**
 * @brief 检查 key 是否已经存在
 * @param self 哈希表指针
 * @param key 键
 * @return 是否存在
 */
CT_API bool ct_hash_contains(const ct_hash_buf_t self, const char *key) __ct_nonnull(1, 2);

/**
 * @brief 插入元素
 * @param self 哈希表指针
 * @param key 键
 * @param value 值
 * @return 是否成功插入
 */
CT_API bool ct_hash_insert(ct_hash_buf_t self, const char *key, ct_any_t value) __ct_nonnull(1, 2);

/**
 * @brief 删除元素
 * @param self 哈希表指针
 * @param key 键
 * @return 是否成功删除
 */
CT_API bool ct_hash_remove(ct_hash_buf_t self, const char *key) __ct_nonnull(1, 2);

/**
 * @brief 清空哈希表
 * @param self 哈希表指针
 */
CT_API void ct_hash_clear(ct_hash_buf_t self) __ct_nonnull(1);

/**
 * @brief 获取 key 对应的值
 * @param self 哈希表指针
 * @param key 键
 * @return 对应的值
 */
CT_API ct_any_t ct_hash_value(ct_hash_buf_t self, const char *key) __ct_nonnull(1, 2);

/**
 * @brief 获取 key 对应的值
 * @param self 哈希表指针
 * @param key 键
 * @param value 值的指针
 * @return 是否成功获取值
 */
CT_API bool ct_hash_value_r(ct_hash_buf_t self, const char *key, ct_any_buf_t value) __ct_nonnull(1, 2);

/**
 * @brief 获取 key 对应的值 (带默认值)
 * @param self 哈希表指针
 * @param key 键
 * @param default_value 默认值
 * @return 对应的值
 */
CT_API ct_any_t ct_hash_value_s(ct_hash_buf_t self, const char *key, ct_any_t default_value) __ct_nonnull(1, 2);

#ifdef __cplusplus
}
#endif
#endif  // _CT_HASH_H
