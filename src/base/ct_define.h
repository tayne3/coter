/**
 * @file ct_define.h
 * @brief 基本定义和宏
 * @note 提供了基本定义和宏，包括基本类型、宏、通用宏和特定于平台的宏
 * @author tayne3@dingtalk.com
 * @date 2024.2.4
 */
#ifndef _CT_DEFINE_H
#define _CT_DEFINE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

// clang-format off

// build platform
# if defined(_WIN32) || defined(_WIN64)         // Windows 
#   define CT_OS_WIN
# elif defined(__linux__) || defined(__linux)   // Linux
#   define CT_OS_LINUX
# elif defined(__APPLE__) || defined(__MACH__)	// Mac
#   define CT_OS_MAC
# elif defined(__wasm__)                        // WebAssembly
#   define CT_OS_WASM
# else                                          // Other
#   error "unsupported platform"
# endif

// bool string
# ifndef STR_BOOL
#   define STR_BOOL(_b) 	((_b) ? "true" : "false")
# endif
// newline
# ifdef _MSC_VER
#   define STR_NEWLINE      "\r\n"
# else
#   define STR_NEWLINE      "\n"
# endif
// separator
# ifdef _MSC_VER
#   define STR_SEPARATOR    '\\'
# else
#   define STR_SEPARATOR    '/'
# endif
// empty string
# ifndef STR_NULL
#   define STR_NULL         ""
# endif
// string is empty
# ifndef STR_ISEMPTY
#   define STR_ISEMPTY(_s) 	(!(_s) || !*(const char *)(_s))
# endif

// no argument
# ifdef ct_noarg
#   warning "duplicate definition"
# else
#   ifdef __cplusplus
#       define ct_noarg
#   else
#       define ct_noarg				void
#   endif
# endif
// null
# ifdef ct_nullptr
#   warning "duplicate definition"
# else
#   ifdef __cplusplus
#       define ct_nullptr			nullptr
#   else
#       define ct_nullptr			NULL
#   endif
# endif
// forever
# ifdef ct_forever
#	warning "duplicate definition"
# else
#   define ct_forever				for(;;)
# endif
// array size
# ifdef ct_arrsize
#	warning "duplicate definition"
# else
#   define ct_arrsize(_arr)			(sizeof(_arr) / sizeof((_arr)[0]))
# endif
// variable swap
# ifdef ct_swap
#	warning "duplicate definition"
# else
#   define ct_swap(_v1, _v2)		do { (_v1) ^= (_v2); (_v2) ^= (_v1); (_v1) ^= (_v2); } while(0)
# endif
// variable unused
# ifdef ct_unused
#	warning "duplicate definition"
# else
#   define ct_unused(_var)			(void)(_var)
# endif

// offset of member
# ifndef OFFSET_OF
# 	ifdef __compiler_offsetof
#   	define OFFSET_OF(_type, _member)			offsetof(_type, _member)
# 	else
#   	define OFFSET_OF(_type, _member)			((size_t)(((char *)&((_type *)STR_NULL)->_member) - (char *)STR_NULL))
# 	endif
# endif
# ifdef ct_offset_of
#	warning "duplicate definition"
# else
#   define ct_offset_of(_type, _member) 			OFFSET_OF(_type, _member)
# endif
// container of
# ifndef CONTAINER_OF
# 	define CONTAINER_OF(_ptr, _type, _member)		(_type *)((_ptr) == ct_nullptr ? ct_nullptr : ((char *)(_ptr)-OFFSET_OF(_type, _member)))
# endif
# ifdef ct_container_of
#	warning "duplicate definition"
# else
#   define ct_container_of(_ptr, _type, _member)	CONTAINER_OF(_ptr, _type, _member)
# endif
// bit size
# ifdef CT_WORDSIZE
#   warning "duplicate definition"
# else
#	if defined(__LP64__) || defined(__64BIT__) || defined(_LP64) || defined(__x86_64) || defined(__x86_64__) ||	   	   \
	defined(__amd64) || defined(__amd64__) || defined(__arm64) || defined(__arm64__) || defined(__sparc64__) ||        \
	defined(__PPC64__) || defined(__ppc64__) || defined(__powerpc64__) || defined(__loongarch64) || defined(_M_X64) || \
	defined(_M_AMD64) || defined(_M_ARM64) || defined(_M_IA64) || defined(__ia64__) || defined(__ia64) ||              \
	(defined(__WORDSIZE) && (__WORDSIZE == 64)) || (defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 8)) ||       \
	defined(TCC_TARGET_X86_64)
#		define CT_WORDSIZE 64
#	else
#		define CT_WORDSIZE 32
#	endif
# endif

# define ct_isalnum(c)  isalnum(c)   // 检查给定字符是否是字母或数字
# define ct_isalpha(c)  isalpha(c)   // 检查给定字符是否是字母
# define ct_iscntrl(c)  iscntrl(c)   // 检查给定字符是否是控制字符
# define ct_isdigit(c)  isdigit(c)   // 检查给定字符是否是数字
# define ct_isgraph(c)  isgraph(c)   // 检查给定字符是否是可打印的非空白字符
# define ct_islower(c)  islower(c)   // 检查给定字符是否是小写字母
# define ct_isprint(c)  isprint(c)   // 检查给定字符是否是可打印字符
# define ct_ispunct(c)  ispunct(c)   // 检查给定字符是否是标点符号字符
# define ct_isspace(c)  isspace(c)   // 检查给定字符是否是空白字符
# define ct_isupper(c)  isupper(c)   // 检查给定字符是否是大写字母
# define ct_isxdigit(c) isxdigit(c)  // 检查给定字符是否是16进制数字
# define ct_isblank(c)  isblank(c)   // 检查给定字符是否是空格或制表符
# define ct_isascii(c)  isascii(c)   // 检查给定字符是否是ASCII字符

# define ct_tolower(c)  tolower(c)   // 将给定大写字母转换为小写
# define ct_toupper(c)  toupper(c)   // 将给定小写字母转换为大写
# define ct_toascii(c)  toascii(c)   // 将给定字符转换为ASCII字符

// current function name, file name, and line number
# if defined(__ct_func__) || defined(__ct_file__) || defined(__ct_line__)
#	warning "duplicate definition"
# else
# 	if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#		define __ct_func__       __func__
#		define __ct_file__       __FILE__
#		define __ct_line__       __LINE__
# 	elif defined(__GNUC__)
#		define __ct_func__       __FUNCTION__
#		define __ct_file__       __FILE__
#		define __ct_line__       __LINE__
# 	elif defined(_MSC_VER)
#		define __ct_func__       __FUNCTION__
#		define __ct_file__       __FILE__
#		define __ct_line__       __LINE__
# 	elif defined(__TINYC__)
#		define __ct_func__       __func__
#		define __ct_file__       __FILE__
#		define __ct_line__       __LINE__
# 	else
#		define __ct_func__       "(nil)"
#		define __ct_file__       "(nil)"
#		define __ct_line__       0
# 	endif
# endif

// force inline
# if defined(__ct_force_inline)
#	warning "duplicate definition"
# else
#	if defined(_MSC_VER)
#		define __ct_force_inline	__forceinline
#	elif defined(__GNUC__) && __GNUC_PREREQ (3,2)
#   	define __ct_force_inline	__always_inline
#	else
#    	define __ct_force_inline	inline
#	endif
# endif

# if defined(__GNUC__) && __GNUC__ >= 2
#   define __ct_attribute__(...)		__attribute__(__VA_ARGS__)				// GCC属性声明
#   define __ct_func_weak__ 			__attribute__((weak))					// 标记弱引用函数
#   define __ct_packed__ 				__attribute__((packed))					// 紧凑打包,平台默认对齐方式
#   define __ct_aligned__(_n)  			__attribute__((aligned(_n)))			// 允许成员间填充,n字节对齐
#   define __ct_packed_aligned__(_n)	__attribute__((packed, aligned(_n)))	// 紧凑打包,n字节对齐
# else
#   define __ct_attribute__(...)
#   define __ct_func_weak__
#   define __ct_packed__
#   define __ct_aligned__(_n) 
#   define __ct_packed_aligned__(_n)
# endif

// 标记函数不会抛出异常
# if defined(__GNUC__) && !defined __cplusplus && __GNUC_PREREQ (3,3)
#   define __ct_func_throw 				__THROW
# else
#   define __ct_func_throw
# endif
// 标记函数可能引发异常
// # if defined(__GNUC__) && !defined __cplusplus &&__GNUC_PREREQ (2,8)
// #   define __ct_func_thrownl 			__THROWNL
// # else
// #   define __ct_func_thrownl
// # endif
# if defined(__GNUC__) && !defined __cplusplus &&__GNUC_PREREQ (2,8)
#   define __ct_func_thrownl 			
# else
#   define __ct_func_thrownl
# endif
// 标记函数分配内存
# if defined(__GNUC__) && __GNUC_PREREQ (2,96)
#   define __ct_func_malloc__ 			__attribute_malloc__
# else
#   define __ct_func_malloc__
# endif
// 标记函数参数为可变长度
# if defined(__GNUC__) && __GNUC_PREREQ (4,3)
#   define __ct_func_alloc_size__(...)	__attribute_alloc_size__ (__VA_ARGS__)
# else
#   define __ct_func_alloc_size__(...)
# endif
// 标记纯函数 (在相同的输入下，总是返回相同的输出，且不会产生任何副作用)
#if defined(__GNUC__) && __GNUC_PREREQ (2,96)
# define __ct_func_pure__ 				__attribute_pure__
#else
# define __ct_func_pure__
#endif
// 标记常量函数
# if defined(__GNUC__) && __GNUC_PREREQ (2,5)
#   define __ct_func_const__ 			__attribute_const__
# else
#   define __ct_func_const__
# endif
// 即使某个函数或变量没有被使用, 也不要将其优化掉
# if defined(__GNUC__) && __GNUC_PREREQ (3,1)
#   define __ct_func_used__ 			__attribute_used__
# else
#   define __ct_func_used__
# endif
// 标记函数不内联
# if defined(__GNUC__) && __GNUC_PREREQ (3,1)
#   define __ct_func_noinline__ 		__attribute_noinline__
# else
#   define __ct_func_noinline__
# endif
// 标记函数已弃用
# if defined(__GNUC__) && __GNUC_PREREQ (3,2)
#   define __ct_func_deprecated__		__attribute_deprecated__
# else 
#   define __ct_func_deprecated__
# endif
// 标记函数已弃用, 并指定打印的消息
// # if defined(__GNUC__) && __GNUC_PREREQ (3,2)
// # 	if __GNUC_PREREQ (4,5)
// #   	define __ct_func_deprecated_msg__(msg)	__attribute_deprecated_msg__(msg)
// #	elif defined(__glibc_clang_has_extension) && __glibc_clang_has_extension (__attribute_deprecated_with_message__)
// #   	define __ct_func_deprecated_msg__(msg)	__attribute_deprecated_msg__(msg)
// # 	else
// #   	define __ct_func_deprecated_msg__(msg)	__attribute_deprecated__
// # 	endif 
// # else
// #   define __ct_func_deprecated_msg__(msg)
// # endif
# if defined(__GNUC__) && __GNUC_PREREQ (3,2)
# 	define __ct_func_deprecated_msg__(msg)	__attribute_deprecated__
# else
#   define __ct_func_deprecated_msg__(msg)
# endif
// 指定 format 参数
# if defined(__GNUC__) && __GNUC_PREREQ (2,8)
#   define __ct_format_arg__(x)			__attribute_format_arg__(x)
# else 
#   define __ct_format_arg__(x)
# endif
// 标记非空参数
# if defined(__GNUC__) && __GNUC_PREREQ (3,3)
#   define __ct_nonnull(...)			__nonnull((__VA_ARGS__))
# else
#   define __ct_nonnull(...)
# endif
// 标记函数返回值会被使用
# if defined(__GNUC__) && __GNUC_PREREQ (3,4)
#   define __ct_func_wur__ 				__attribute_warn_unused_result__
# else
#   define __ct_func_wur__
# endif
// 将错误消息与调用点的源位置相关联, 而不是与函数内的源位置相关联
# if defined(__GNUC__) && __GNUC_PREREQ (4,3)
#   define __ct_func_artificial__		__attribute_artificial__
# else
#   define __ct_func_artificial__
# endif
// restrict
# if defined(__GNUC__) && __GNUC_PREREQ (4,3)
#	define __ct_restrict				__restrict
# else
#   define __ct_restrict
# endif
// 标记函数不返回
# if defined(__GNUC__) && __GNUC_PREREQ (2,8) && defined(_Noreturn)
#   define __ct_func_noreturn 			_Noreturn
# else
#   define __ct_func_noreturn
# endif

#endif // _CT_DEFINE_H
