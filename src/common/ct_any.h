/**
 * @file ct_any.h
 * @brief 定义 ct_any_t 类型
 * @author tayne3@dingtalk.com
 * @date 2023.11.29
 * @note
 * Any 是一种任意类型的数据, 支持各种内置类型和指针;
 * Any 允许传递任意内置类型的数据, 但取出时的类型需要与放入时的类型一致;
 */
#ifndef _CT_ANY_H
#define _CT_ANY_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

// Any 数据类型
enum ct_any_type {
	CTAny_TypeInvalid = 0,  // 无效类型
	CTAny_TypeBool,         // 布尔类型
	CTAny_TypeFloat,        // 浮点类型
	CTAny_TypeDouble,       // 双精度浮点类型
	CTAny_TypeString,       // 字符串类型
	CTAny_TypePointer,      // 指针类型
	CTAny_TypeInt,          // 整型
	CTAny_TypeInt8,         // 8位整型
	CTAny_TypeInt16,        // 16位整型
	CTAny_TypeInt32,        // 32位整型
	CTAny_TypeInt64,        // 64位整型
	CTAny_TypeUint,         // 无符号整型
	CTAny_TypeUint8,        // 8位无符号整型
	CTAny_TypeUint16,       // 16位无符号整型
	CTAny_TypeUint32,       // 32位无符号整型
	CTAny_TypeUint64,       // 64位无符号整型
};

// ct_any_type_t 类型定义
typedef int32_t ct_any_type_t;

/**
 * @brief 任何类型的变量
 */
typedef struct {
	union {
		bool        b;    // 布尔类型
		float       f;    // 浮点类型
		double      d;    // 双精度浮点类型
		const char *str;  // 字符串类型
		void       *ptr;  // 指针类型
		int         i;    // 整型
		int8_t      i8;   // 8位整型
		int16_t     i16;  // 16位整型
		int32_t     i32;  // 32位整型
		int64_t     i64;  // 64位整型
		unsigned    u;    // 无符号整型
		uint8_t     u8;   // 8位无符号整型
		uint16_t    u16;  // 16位无符号整型
		uint32_t    u32;  // 32位无符号整型
		uint64_t    u64;  // 64位无符号整型
	} d[1];               // 数据
	ct_any_type_t type;   // 数据类型
} ct_any_t, ct_any_buf_t[1];

// ct_any_null
CT_API const ct_any_t ct_any_null;

// 初始化
#define CT_ANY_INIT_SPECIFY(_x, _t) {.d[0] = {.u64 = (uint64_t)(_x)}, .type = (_t)}

/**
 * @brief Any 函数组
 * @note 用于在容器中能使用 any 变量存储各种自定义数据
 */
typedef struct ct_any_methods {
	// 构造
	void (*ctor)(ct_any_buf_t src, const ct_any_buf_t value);
	// 析构
	void (*dtor)(ct_any_buf_t src);
	// 更新值
	void (*update)(ct_any_buf_t src, const ct_any_buf_t value);
} ct_any_methods_t, ct_any_methods_buf_t[1];

// 默认函数组
CT_API const ct_any_methods_t ct_any_methods_default;

// 构造
CT_API void ct_any_ctor(const ct_any_methods_buf_t methods, ct_any_buf_t src, const ct_any_buf_t value);
// 析构
CT_API void ct_any_dtor(const ct_any_methods_buf_t methods, ct_any_buf_t src);
// 更新值
CT_API void ct_any_update(const ct_any_methods_buf_t methods, ct_any_buf_t src, const ct_any_buf_t value);

// clang-format off
#define CT_ANY_INIT_INVALID 	{.d[0]={.u64 = 0}, .type = CTAny_TypeInvalid}				// 初始化无效类型
#define CT_ANY_INIT_BOOL(_x)    {.d[0]={.b = (bool)(_x)}, .type = CTAny_TypeBool}     		// 初始化布尔类型
#define CT_ANY_INIT_FLOAT(_x)   {.d[0]={.f = (float)(_x)}, .type = CTAny_TypeFloat}   		// 初始化浮点类型
#define CT_ANY_INIT_DOUBLE(_x)  {.d[0]={.d = (double)(_x)}, .type = CTAny_TypeDouble}  		// 初始化双精度浮点类型
#define CT_ANY_INIT_STRING(_x)  {.d[0]={.str = (char *)(_x)}, .type = CTAny_TypeString}  	// 初始化字符串类型
#define CT_ANY_INIT_POINTER(_x) {.d[0]={.ptr = (void *)(_x)}, .type = CTAny_TypePointer}	// 初始化指针类型
#define CT_ANY_INIT_INT(_x)     {.d[0]={.i = (int)(_x)}, .type = CTAny_TypeInt}     		// 初始化整型
#define CT_ANY_INIT_INT8(_x)    {.d[0]={.i8 = (int8_t)(_x)}, .type = CTAny_TypeInt8}    	// 初始化8位整型
#define CT_ANY_INIT_INT16(_x)   {.d[0]={.i16 = (int16_t)(_x)}, .type = CTAny_TypeInt16}   	// 初始化16位整型
#define CT_ANY_INIT_INT32(_x)   {.d[0]={.i32 = (int32_t)(_x)}, .type = CTAny_TypeInt32}   	// 初始化32位整型
#define CT_ANY_INIT_INT64(_x)   {.d[0]={.i64 = (int64_t)(_x)}, .type = CTAny_TypeInt64}   	// 初始化64位整型
#define CT_ANY_INIT_UINT(_x)    {.d[0]={.u = (unsigned)(_x)}, .type = CTAny_TypeUint}    	// 初始化无符号整型
#define CT_ANY_INIT_UINT8(_x)   {.d[0]={.u8 = (uint8_t)(_x)}, .type = CTAny_TypeUint8}   	// 初始化8位无符号整型
#define CT_ANY_INIT_UINT16(_x)  {.d[0]={.u16 = (uint16_t)(_x)}, .type = CTAny_TypeUint16}	// 初始化16位无符号整型
#define CT_ANY_INIT_UINT32(_x)  {.d[0]={.u32 = (uint32_t)(_x)}, .type = CTAny_TypeUint32}  	// 初始化32位无符号整型
#define CT_ANY_INIT_UINT64(_x)  {.d[0]={.u64 = (uint64_t)(_x)}, .type = CTAny_TypeUint64}  	// 初始化64位无符号整型
// clang-format on

#define CT_ANY_SPECIFY(_x, _t) ((ct_any_t)CT_ANY_INIT_SPECIFY(_x, _t))  // 指定类型
#define CT_ANY_BOOL(_x)        ((ct_any_t)CT_ANY_INIT_BOOL(_x))         // 布尔类型
#define CT_ANY_FLOAT(_x)       ((ct_any_t)CT_ANY_INIT_FLOAT(_x))        // 浮点类型
#define CT_ANY_DOUBLE(_x)      ((ct_any_t)CT_ANY_INIT_DOUBLE(_x))       // 双精度浮点类型
#define CT_ANY_STRING(_x)      ((ct_any_t)CT_ANY_INIT_STRING(_x))       // 字符串类型
#define CT_ANY_POINTER(_x)     ((ct_any_t)CT_ANY_INIT_POINTER(_x))      // 指针类型
#define CT_ANY_INT(_x)         ((ct_any_t)CT_ANY_INIT_INT(_x))          // 整型
#define CT_ANY_INT8(_x)        ((ct_any_t)CT_ANY_INIT_INT8(_x))         // 8位整型
#define CT_ANY_INT16(_x)       ((ct_any_t)CT_ANY_INIT_INT16(_x))        // 16位整型
#define CT_ANY_INT32(_x)       ((ct_any_t)CT_ANY_INIT_INT32(_x))        // 32位整型
#define CT_ANY_INT64(_x)       ((ct_any_t)CT_ANY_INIT_INT64(_x))        // 64位整型
#define CT_ANY_UINT(_x)        ((ct_any_t)CT_ANY_INIT_UINT(_x))         // 无符号整型
#define CT_ANY_UINT8(_x)       ((ct_any_t)CT_ANY_INIT_UINT8(_x))        // 8位无符号整型
#define CT_ANY_UINT16(_x)      ((ct_any_t)CT_ANY_INIT_UINT16(_x))       // 16位无符号整型
#define CT_ANY_UINT32(_x)      ((ct_any_t)CT_ANY_INIT_UINT32(_x))       // 32位无符号整型
#define CT_ANY_UINT64(_x)      ((ct_any_t)CT_ANY_INIT_UINT64(_x))       // 64位无符号整型

#define ct_any_value_bool(_any)    ((_any).d->b)    // 获取布尔类型的值
#define ct_any_value_float(_any)   ((_any).d->f)    // 获取浮点类型的值
#define ct_any_value_double(_any)  ((_any).d->d)    // 获取双精度浮点类型的值
#define ct_any_value_string(_any)  ((_any).d->str)  // 获取字符串类型的值
#define ct_any_value_pointer(_any) ((_any).d->ptr)  // 获取指针类型的值
#define ct_any_value_int(_any)     ((_any).d->i)    // 获取整型的值
#define ct_any_value_int8(_any)    ((_any).d->i8)   // 获取8位整型的值
#define ct_any_value_int16(_any)   ((_any).d->i16)  // 获取16位整型的值
#define ct_any_value_int32(_any)   ((_any).d->i32)  // 获取32位整型的值
#define ct_any_value_int64(_any)   ((_any).d->i64)  // 获取64位整型的值
#define ct_any_value_uint(_any)    ((_any).d->u)    // 获取无符号整型的值
#define ct_any_value_uint8(_any)   ((_any).d->u8)   // 获取8位无符号整型的值
#define ct_any_value_uint16(_any)  ((_any).d->u16)  // 获取16位无符号整型的值
#define ct_any_value_uint32(_any)  ((_any).d->u32)  // 获取32位无符号整型的值
#define ct_any_value_uint64(_any)  ((_any).d->u64)  // 获取64位无符号整型的值

/**
 * @brief 检查 Any 对象 是否有效
 * @param self Any 对象指针
 * @return 如果有效，返回 true；否则，返回 false
 */
CT_API bool ct_any_isvalid(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 将 Any 对象 转换为字符串
 * @param self Any 对象指针
 * @param buf 字符串缓冲区
 * @param max 最大长度
 * @return 字符串长度
 */
CT_API size_t ct_any_tostring(const ct_any_buf_t self, char *buf, size_t max) __ct_throw __ct_nonnull(1);

/**
 * @brief 比较两个 Any 对象 的大小
 * @param l 左操作数
 * @param r 右操作数
 * @return -1=小于; 0=等于; 1=大于; -2=错误
 */
CT_API int ct_any_compare(const ct_any_buf_t l, const ct_any_buf_t r) __ct_throw __ct_nonnull(1);

/**
 * @brief 交换两个 Any 对象 的值
 * @param l 左操作数
 * @param r 右操作数
 */
CT_API void ct_any_swap(ct_any_buf_t l, ct_any_buf_t r) __ct_throw __ct_nonnull(1);

/**
 * @brief 拷贝 Any 对象 的值
 * @param self 目标 Any 对象
 * @param other 源 Any 对象
 */
CT_API void ct_any_copy(ct_any_buf_t self, const ct_any_buf_t other) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 类型
 * @param self Any 对象指针
 * @return 数据类型
 */
CT_API ct_any_type_t ct_any_type(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为布尔类型
 * @param self Any 对象指针
 * @param value 布尔值
 */
CT_API void ct_any_set_bool(ct_any_buf_t self, bool value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为浮点类型
 * @param self Any 对象指针
 * @param value 浮点值
 */
CT_API void ct_any_set_float(ct_any_buf_t self, float value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为双精度浮点类型
 * @param self Any 对象指针
 * @param value 双精度浮点值
 */
CT_API void ct_any_set_double(ct_any_buf_t self, double value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为字符串类型
 * @param self Any 对象指针
 * @param value 字符串值
 */
CT_API void ct_any_set_string(ct_any_buf_t self, const char *value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为指针类型
 * @param self Any 对象指针
 * @param value 指针值
 */
CT_API void ct_any_set_pointer(ct_any_buf_t self, void *value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为整型
 * @param self Any 对象指针
 * @param value 整型值
 */
CT_API void ct_any_set_int(ct_any_buf_t self, int value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为8位整型
 * @param self Any 对象指针
 * @param value 8位整型值
 */
CT_API void ct_any_set_int8(ct_any_buf_t self, int8_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为16位整型
 * @param self Any 对象指针
 * @param value 16位整型值
 */
CT_API void ct_any_set_int16(ct_any_buf_t self, int16_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为32位整型
 * @param self Any 对象指针
 * @param value 32位整型值
 */
CT_API void ct_any_set_int32(ct_any_buf_t self, int32_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为64位整型
 * @param self Any 对象指针
 * @param value 64位整型值
 */
CT_API void ct_any_set_int64(ct_any_buf_t self, int64_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为无符号整型
 * @param self Any 对象指针
 * @param value 无符号整型值
 */
CT_API void ct_any_set_uint(ct_any_buf_t self, unsigned value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为8位无符号整型
 * @param self Any 对象指针
 * @param value 8位无符号整型值
 */
CT_API void ct_any_set_uint8(ct_any_buf_t self, uint8_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为16位无符号整型
 * @param self Any 对象指针
 * @param value 16位无符号整型值
 */
CT_API void ct_any_set_uint16(ct_any_buf_t self, uint16_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为32位无符号整型
 * @param self Any 对象指针
 * @param value 32位无符号整型值
 */
CT_API void ct_any_set_uint32(ct_any_buf_t self, uint32_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 赋值为64位无符号整型
 * @param self Any 对象指针
 * @param value 64位无符号整型值
 */
CT_API void ct_any_set_uint64(ct_any_buf_t self, uint64_t value) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 布尔值
 * @param self Any 对象指针
 * @return 布尔值
 */
CT_API bool ct_any_get_bool(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 浮点值
 * @param self Any 对象指针
 * @return 浮点值
 */
CT_API float ct_any_get_float(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 双精度浮点值
 * @param self Any 对象指针
 * @return 双精度浮点值
 */
CT_API double ct_any_get_double(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 字符串值
 * @param self Any 对象指针
 * @return 字符串值
 */
CT_API const char *ct_any_get_string(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 指针值
 * @param self Any 对象指针
 * @return 指针值
 */
CT_API void *ct_any_get_pointer(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 整型值
 * @param self Any 对象指针
 * @return 整型值
 */
CT_API int ct_any_get_int(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 8位整型值
 * @param self Any 对象指针
 * @return 8位整型值
 */
CT_API int8_t ct_any_get_int8(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 16位整型值
 * @param self Any 对象指针
 * @return 16位整型值
 */
CT_API int16_t ct_any_get_int16(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 32位整型值
 * @param self Any 对象指针
 * @return 32位整型值
 */
CT_API int32_t ct_any_get_int32(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 64位整型值
 * @param self Any 对象指针
 * @return 64位整型值
 */
CT_API int64_t ct_any_get_int64(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 无符号整型值
 * @param self Any 对象指针
 * @return 无符号整型值
 */
CT_API unsigned ct_any_get_uint(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 8位无符号整型值
 * @param self Any 对象指针
 * @return 8位无符号整型值
 */
CT_API uint8_t ct_any_get_uint8(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 16位无符号整型值
 * @param self Any 对象指针
 * @return 16位无符号整型值
 */
CT_API uint16_t ct_any_get_uint16(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 32位无符号整型值
 * @param self Any 对象指针
 * @return 32位无符号整型值
 */
CT_API uint32_t ct_any_get_uint32(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * @brief 获取 64位无符号整型值
 * @param self Any 对象指针
 * @return 64位无符号整型值
 */
CT_API uint64_t ct_any_get_uint64(const ct_any_buf_t self) __ct_throw __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_ANY_H
