/**
 * @file ct_macro.h
 * @brief 基本定义和宏
 * @note 提供了基本定义和宏，包括基本类型、宏、通用宏和特定于平台的宏
 * @author tayne3@dingtalk.com
 * @date 2024.2.4
 */
#ifndef _CT_MACRO_H
#define _CT_MACRO_H

#include "base/prefix/ct_config.h"

// clang-format off

#ifndef __cplusplus
#if HAVE_STDBOOL_H
#include <stdbool.h>
#else
    #ifndef bool
    #define bool char
    #endif

    #ifndef true
    #define true 1
    #endif

    #ifndef false
    #define false 0
    #endif
#endif
#endif

#if HAVE_STDINT_H
#include <stdint.h>
#elif defined(_MSC_VER) && _MSC_VER < 1700
typedef __int8              int8_t;
typedef __int16             int16_t;
typedef __int32             int32_t;
typedef __int64             int64_t;
typedef unsigned __int8     uint8_t;
typedef unsigned __int16    uint16_t;
typedef unsigned __int32    uint32_t;
typedef unsigned __int64    uint64_t;
#endif

// OS
#if defined(WIN64) || defined(_WIN64)
    #define CT_OS_WIN64
    #define CT_OS_WIN32
#elif defined(WIN32)|| defined(_WIN32)
    #define CT_OS_WIN32
#elif defined(ANDROID) || defined(__ANDROID__)
    #define CT_OS_ANDROID
    #define CT_OS_LINUX
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #define CT_OS_LINUX
#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_MAC) && TARGET_OS_MAC
        #define CT_OS_MAC
    #elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
        #define CT_OS_IOS
    #endif
    #define CT_OS_DARWIN
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    #define CT_OS_FREEBSD
    #define CT_OS_BSD
#elif defined(__NetBSD__)
    #define CT_OS_NETBSD
    #define CT_OS_BSD
#elif defined(__OpenBSD__)
    #define CT_OS_OPENBSD
    #define CT_OS_BSD
#elif defined(sun) || defined(__sun) || defined(__sun__)
    #define CT_OS_SOLARIS
#elif defined(__wasm__)
    #define CT_OS_WASM
#else
    #warning "Untested operating system platform!"
#endif

#if defined(CT_OS_WIN32) || defined(CT_OS_WIN64)
    #define CT_OS_WIN
#else
    #define CT_OS_UNIX
#endif

// no argument
// # ifdef ct_noarg
// #   warning "duplicate definition"
// # else
// #   ifdef __cplusplus
// #       define ct_noarg
// #   else
// #       define ct_noarg				void
// #   endif
// # endif

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
// # ifdef ct_swap
// #	warning "duplicate definition"
// # else
// #   define ct_swap(_v1, _v2)		do { (_v1) ^= (_v2); (_v2) ^= (_v1); (_v1) ^= (_v2); } while(0)
// # endif

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
#       define OFFSET_OF(_type, _member)            (size_t)((char*)(&((_type*)0)->_member))
# 	endif
# endif
// container of
# ifndef CONTAINER_OF
# 	define CONTAINER_OF(_ptr, _type, _member)		(_type *)((_ptr) == ct_nullptr ? ct_nullptr : ((char *)(_ptr)-OFFSET_OF(_type, _member)))
# endif

// # ifndef OFFSET_OF
// # 	ifdef __compiler_offsetof
// #   	define OFFSET_OF(_type, _member)			offsetof(_type, _member)
// # 	else
// #   	define OFFSET_OF(_type, _member)			((size_t)(((char *)&((_type *)STR_NULL)->_member) - (char *)STR_NULL))
// # 	endif
// # endif
// # ifdef ct_offset_of
// #	warning "duplicate definition"
// # else
// #   define ct_offset_of(_type, _member) 			OFFSET_OF(_type, _member)
// # endif
// # ifdef ct_container_of
// #	warning "duplicate definition"
// # else
// #   define ct_container_of(_ptr, _type, _member)	CONTAINER_OF(_ptr, _type, _member)
// # endif
// bit size

# ifdef __CT_WORDSIZE
#   warning "duplicate definition"
# else
#	if defined(__LP64__) || defined(__64BIT__) || defined(_LP64) || defined(__x86_64) || defined(__x86_64__) ||	   	   \
	defined(__amd64) || defined(__amd64__) || defined(__arm64) || defined(__arm64__) || defined(__sparc64__) ||        \
	defined(__PPC64__) || defined(__ppc64__) || defined(__powerpc64__) || defined(__loongarch64) || defined(_M_X64) || \
	defined(_M_AMD64) || defined(_M_ARM64) || defined(_M_IA64) || defined(__ia64__) || defined(__ia64) ||              \
	(defined(__WORDSIZE) && (__WORDSIZE == 64)) || (defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 8)) ||       \
	defined(TCC_TARGET_X86_64)
#		define __CT_WORDSIZE 64
#	else
#		define __CT_WORDSIZE 32
#	endif
# endif

// max
# undef MAX
# define MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))
# ifdef CT_MAX
#	warning "duplicate definition"
# else
#   define CT_MAX(_a, _b) MAX((_a), (_b))
# endif

// min
# undef MIN
# define MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))
# ifdef CT_MIN
#	warning "duplicate definition"
# else
#   define CT_MIN(_a, _b) MIN((_a), (_b))
# endif

// #include <ctype.h>
// # define ct_isalnum(c)  isalnum(c)   // 检查给定字符是否是字母或数字
// # define ct_isalpha(c)  isalpha(c)   // 检查给定字符是否是字母
// # define ct_iscntrl(c)  iscntrl(c)   // 检查给定字符是否是控制字符
// # define ct_isdigit(c)  isdigit(c)   // 检查给定字符是否是数字
// # define ct_isgraph(c)  isgraph(c)   // 检查给定字符是否是可打印的非空白字符
// # define ct_islower(c)  islower(c)   // 检查给定字符是否是小写字母
// # define ct_isprint(c)  isprint(c)   // 检查给定字符是否是可打印字符
// # define ct_ispunct(c)  ispunct(c)   // 检查给定字符是否是标点符号字符
// # define ct_isspace(c)  isspace(c)   // 检查给定字符是否是空白字符
// # define ct_isupper(c)  isupper(c)   // 检查给定字符是否是大写字母
// # define ct_isxdigit(c) isxdigit(c)  // 检查给定字符是否是16进制数字
// # define ct_isblank(c)  isblank(c)   // 检查给定字符是否是空格或制表符
// # define ct_isascii(c)  isascii(c)   // 检查给定字符是否是ASCII字符

// # define ct_tolower(c)  tolower(c)   // 将给定大写字母转换为小写
// # define ct_toupper(c)  toupper(c)   // 将给定小写字母转换为大写
// # define ct_toascii(c)  toascii(c)   // 将给定字符转换为ASCII字符

#ifdef WIN32
#ifdef __cplusplus
#define DLL_EXPORT_C_DECL     extern "C" __declspec(dllexport)
#define DLL_IMPORT_C_DECL     extern "C" __declspec(dllimport)
#define DLL_EXPORT_DECL       extern __declspec(dllexport)
#define DLL_IMPORT_DECL       extern __declspec(dllimport)
#define DLL_EXPORT_CLASS_DECL __declspec(dllexport)
#define DLL_IMPORT_CLASS_DECL __declspec(dllimport)
#else
#define DLL_EXPORT_DECL __declspec(dllexport)
#define DLL_IMPORT_DECL __declspec(dllimport)
#endif
#else
#ifdef __cplusplus
#define DLL_EXPORT_C_DECL extern "C"
#define DLL_IMPORT_C_DECL extern "C"
#define DLL_EXPORT_DECL   extern
#define DLL_IMPORT_DECL   extern
#define DLL_EXPORT_CLASS_DECL
#define DLL_IMPORT_CLASS_DECL
#else
#define DLL_EXPORT_DECL extern
#define DLL_IMPORT_DECL extern
#endif
#endif

#ifdef COTER_EXPORTS
#define COTER_API DLL_EXPORT_DECL
#else
#define COTER_API DLL_IMPORT_DECL
#endif

# ifndef __GNUC_PREREQ
# 	define __GNUC_PREREQ(a, b)	0
# endif

# ifndef __glibc_clang_has_extension
# 	define __glibc_clang_has_extension(...)	0
# endif

// current function name, file name, and line number
# if defined(__ct_func__) || defined(__ct_file__) || defined(__ct_line__)
#	warning "duplicate definition"
# else
# 	if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   	define __ct_func__		__extension__ __PRETTY_FUNCTION__
#		define __ct_file__		__FILE__
#		define __ct_line__		__LINE__
# 	elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#		define __ct_func__		__func__
#		define __ct_file__		__FILE__
#		define __ct_line__		__LINE__
# 	elif defined(__GNUC__)
#		define __ct_func__		__FUNCTION__
#		define __ct_file__		__FILE__
#		define __ct_line__		__LINE__
# 	elif defined(_MSC_VER)
#		define __ct_func__		__FUNCTION__
#		define __ct_file__		__FILE__
#		define __ct_line__		__LINE__
# 	elif defined(__TINYC__)
#		define __ct_func__		__func__
#		define __ct_file__		__FILE__
#		define __ct_line__		__LINE__
# 	else
#		define __ct_func__		"(nil)"
#		define __ct_file__		"(nil)"
#		define __ct_line__		0
# 	endif
# endif

# ifndef __THROW
# 	define __THROW
# endif

# ifndef __THROWNL
# 	define __THROWNL
# endif

// force inline
# if defined(__ct_force_inline)
#	warning "duplicate definition"
# else
#	if defined(_MSC_VER)
#		define __ct_force_inline		__forceinline
#	elif __GNUC_PREREQ (3,2)
#   	define __ct_force_inline		__always_inline
#	else
#    	define __ct_force_inline		inline
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
# if !defined __cplusplus && __GNUC_PREREQ (3,3)
#   define __ct_func_throw 				__THROW
# else
#   define __ct_func_throw
# endif
// 标记函数可能引发异常
# if !defined __cplusplus  && __GNUC_PREREQ (2,8)
#   define __ct_func_thrownl 			__THROWNL
# else
#   define __ct_func_thrownl
# endif
// 标记函数分配内存
# if __GNUC_PREREQ(2,96) 
#   define __ct_func_malloc__ 			__attribute_malloc__
# else
#   define __ct_func_malloc__
# endif
// 标记函数参数为可变长度
# if __GNUC_PREREQ (4,3) 
#   define __ct_func_alloc_size__(...)	__attribute_alloc_size__ (__VA_ARGS__)
# else
#   define __ct_func_alloc_size__(...)
# endif
// 标记纯函数 (在相同的输入下，总是返回相同的输出，且不会产生任何副作用)
#if __GNUC_PREREQ (2,96)
#   define __ct_func_pure__ 			__attribute_pure__
# else
# define __ct_func_pure__
#endif
// 标记常量函数
# if __GNUC_PREREQ (2,5) 
#   define __ct_func_const__ 			__attribute_const__
# else
#   define __ct_func_const__
# endif
// 即使某个函数或变量没有被使用, 也不要将其优化掉
# if __GNUC_PREREQ (3,1)
#   define __ct_func_used__ 			__attribute_used__
# else
#   define __ct_func_used__
# endif
// 标记函数不内联
# if __GNUC_PREREQ (3,1)
#   define __ct_func_noinline__ 		__attribute_noinline__
# else
#   define __ct_func_noinline__
# endif
// 标记函数已弃用
# if __GNUC_PREREQ (3,2)
#   define __ct_func_deprecated__		__attribute_deprecated__
# else 
#   define __ct_func_deprecated__
# endif
// 标记函数已弃用, 并指定打印的消息
# if __GNUC_PREREQ (3,2) 
# 	if __GNUC_PREREQ (4,5)
#   	define __ct_func_deprecated_msg__(msg)	__attribute_deprecated_msg__(msg)
#	elif __glibc_clang_has_extension (__attribute_deprecated_with_message__)
#   	define __ct_func_deprecated_msg__(msg)	__attribute_deprecated_msg__(msg)
# 	else
#   	define __ct_func_deprecated_msg__(msg)	__attribute_deprecated__
# 	endif
# else
#   define __ct_func_deprecated_msg__(msg)
# endif
// 指定 format 参数
# if __GNUC_PREREQ (2,8)
#   define __ct_format_arg__(x)			__attribute_format_arg__(x)
# else 
#   define __ct_format_arg__(x)
# endif
// 标记非空参数
# if __GNUC_PREREQ (3,3)
#   define __ct_nonnull(...)			__nonnull((__VA_ARGS__))
# else
#   define __ct_nonnull(...)
# endif
// 标记函数返回值会被使用
# if __GNUC_PREREQ (3,4)  
#   define __ct_func_wur__ 				__attribute_warn_unused_result__
# else
#   define __ct_func_wur__
# endif
// 将错误消息与调用点的源位置相关联, 而不是与函数内的源位置相关联
# if __GNUC_PREREQ (4,3) 
#   define __ct_func_artificial__		__attribute_artificial__
# else
#   define __ct_func_artificial__
# endif
// restrict
# if __GNUC_PREREQ (4,3)
#   define __ct_restrict				__restrict
# else
#   define __ct_restrict
# endif
// 标记函数不返回
# if __GNUC_PREREQ (2,8) && defined(_Noreturn)
#   define __ct_func_noreturn 			_Noreturn
# else
#   define __ct_func_noreturn
# endif

#endif // _CT_MACRO_H
