/**
 * @file macro.h
 * @brief Basic definitions and macros
 * @note Provides fundamental types, safety macros, common utilities, and platform specifics.
 */
#ifndef COTER_CORE_MACRO_H
#define COTER_CORE_MACRO_H

#include "coter/core/config.h"

// ANSI C
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// clang-format off

# ifndef COTER_API
#   if defined(_WIN32) || defined(__CYGWIN__)
#       if defined(COTER_LIB_EXPORT)
#           define COTER_API __declspec(dllexport)
#       elif defined(COTER_SHARED)
#           define COTER_API __declspec(dllimport)
#       endif
#   elif defined(__GNUC__) || defined(__clang__)
#       if defined(COTER_LIB_EXPORT) || defined(COTER_SHARED)
#           define COTER_API __attribute__((visibility("default")))
#       endif
#   endif
# endif
# ifndef COTER_API
#   define COTER_API
# endif

// OS
# if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#	define CT_OS_WIN64
# elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#	define CT_OS_WIN32
# elif defined(WINCE) || defined(_WIN32_WCE)
#	define CT_OS_WINCE
# elif defined(linux) || defined(__linux__) || defined(__linux)
#	define CT_OS_LINUX
# elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#    include <TargetConditionals.h>
#    if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#        define CT_OS_MAC
#    elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#        define CT_OS_IOS
#    endif
#    define CT_OS_DARWIN
# else
#	error "Unsupported platform!"
# endif

# if defined(__LP64__) || defined(__64BIT__) || defined(_LP64) || defined(__x86_64) || defined(__x86_64__) ||           \
	 defined(__amd64) || defined(__amd64__) || defined(__arm64) || defined(__arm64__) || defined(__sparc64__) ||        \
	 defined(__PPC64__) || defined(__ppc64__) || defined(__powerpc64__) || defined(__loongarch64) || defined(_M_X64) || \
	 defined(_M_AMD64) || defined(_M_ARM64) || defined(_M_IA64) || defined(__ia64__) || defined(__ia64) ||              \
	 (defined(__WORDSIZE) && (__WORDSIZE == 64)) || (defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 8)) ||       \
	 defined(TCC_TARGET_X86_64)
#	define __CT_WORDSIZE 64
# else
#	define __CT_WORDSIZE 32
# endif

# ifndef __GNUC_PREREQ
#   if defined(__GNUC__) && defined(__GNUC_MINOR__)
#     define __GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#   else
#     define __GNUC_PREREQ(maj, min) 0
#   endif
# endif

# ifndef __cplusplus
#	if HAVE_STDBOOL_H
#		include <stdbool.h>
#	else
#		ifndef bool
#			define bool char
#		endif
#		ifndef true
#			define true 1
#		endif
#		ifndef false
#			define false 0
#		endif
#	endif
# endif

# if HAVE_STDINT_H
#	include <stdint.h>
# elif defined(_MSC_VER) && _MSC_VER < 1700
	typedef __int8              int8_t;
	typedef __int16             int16_t;
	typedef __int32             int32_t;
	typedef __int64             int64_t;
	typedef unsigned __int8     uint8_t;
	typedef unsigned __int16    uint16_t;
	typedef unsigned __int32    uint32_t;
	typedef unsigned __int64    uint64_t;
# endif

# ifdef _MSC_VER
#   define strcasecmp  _stricmp
#   define strncasecmp _strnicmp
# else
#	include <strings.h>
#	define stricmp     strcasecmp
#	define strnicmp    strncasecmp
# endif

// empty string
# ifndef STR_NULL
#   define STR_NULL ""
# endif

// string is empty
# ifndef STR_ISEMPTY
#   define STR_ISEMPTY(_s) 	(!(_s) || !*(const char *)(_s))
# endif

#if defined(CT_OS_WIN64) || defined(CT_OS_WIN32) || defined(CT_OS_WINCE)
    #define CT_OS_WIN
#else
    #define CT_OS_UNIX
#endif

// newline and separator
# ifdef CT_OS_WIN
#   define STR_NEWLINE "\r\n"
#   define STR_SEPARATOR "\\"
#   define STR_SEPARATOR_CHAR '\\'
# else
#   define STR_NEWLINE "\n"
#   define STR_SEPARATOR "/"
#   define STR_SEPARATOR_CHAR '/'
# endif

#if defined(__x86_64__) || defined(__i386__)
    #if defined(__GNUC__) || defined(__clang__)
        #define CT_PAUSE() __asm__ volatile("pause" ::: "memory")
    #elif defined(_MSC_VER)
        #include <intrin.h>
        #define CT_PAUSE() _mm_pause()
    #endif
#elif (defined(__aarch64__) || defined(__arm__)) && defined(__ARM_ARCH)
    #if __ARM_ARCH >= 7
        #define CT_PAUSE() __asm__ volatile("yield" ::: "memory")
    #else
        #define CT_PAUSE() __asm__ volatile("nop" ::: "memory")
    #endif
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    #define CT_PAUSE() __asm__ volatile("or 27,27,27" ::: "memory")
#elif defined(__ia64__)
    #define CT_PAUSE() __asm__ volatile ("hint @pause" ::: "memory")
#elif defined(__sparc__)
    #define CT_PAUSE() __asm__ volatile("rd %ccr, %g0" ::: "memory")
#elif defined(__mips__)
    #define CT_PAUSE() __asm__ volatile(".word 0x00000140" ::: "memory")
#elif defined(__riscv)
    #if __riscv_xlen == 32
        #define CT_PAUSE() __asm__ volatile(".word 0x0100000f" ::: "memory")
    #elif __riscv_xlen == 64
        #define CT_PAUSE() __asm__ volatile(".dword 0x0100000f" ::: "memory")
    #endif
#elif defined(__loongarch__)
    #define CT_PAUSE() __asm__ volatile("dbar 0" ::: "memory")
#else
    #include <sched.h>
    #define CT_PAUSE() sched_yield()
#endif

typedef int ct_endian_t;

#define CT_ENDIAN_BIG    0
#define CT_ENDIAN_LITTLE 1

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_BIG
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_LITTLE
#elif defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || defined(_MIBSEB) || defined(__MIBSEB) || \
	defined(__MIBSEB__) || defined(__sparc) || defined(__sparc__)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_BIG
#elif defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) || defined(__MIPSEL) || \
	defined(__MIPSEL__) || defined(_WIN32) || defined(_WIN64) || defined(__i386__) || defined(__x86_64__) ||           \
	defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_LITTLE
#elif defined(__linux__) || defined(__CYGWIN__) || defined(__GNU__) || defined(__GLIBC__)
#include <endian.h>
#if defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_BIG
#elif defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_LITTLE
#endif
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <machine/endian.h>
#if defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_BIG
#elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_LITTLE
#endif
#endif

#if !defined(CT_ENDIAN_SYSTEM)
#error "Platform endianness could not be detected. Please define CT_ENDIAN_SYSTEM manually."
#endif

#define CT_ENDIAN_IS_BIG    (CT_ENDIAN_SYSTEM == CT_ENDIAN_BIG)
#define CT_ENDIAN_IS_LITTLE (CT_ENDIAN_SYSTEM == CT_ENDIAN_LITTLE)

// array size
# define ct_arrsize(_arr) (sizeof(_arr) / sizeof((_arr)[0]))
# define CT_ARRSIZE(_arr) (sizeof(_arr) / sizeof((_arr)[0]))

// variable unused
# define ct_unused(_var) (void)(_var)
# define CT_UNUSED(_var) (void)(_var)

// min and max
#ifdef __cplusplus
#define CT_MIN(_a, _b)             std::min(_a, _b)
#define CT_MAX(_a, _b)             std::max(_a, _b)
#define CT_CLAMP(_val, _min, _max) std::clamp(_val, _min, _max)
#elif defined(__GNUC__) || defined(__clang__)
# define CT_MIN(_a, _b)         ({ __typeof__(_a) _a_ = (_a); __typeof__(_b) _b_ = (_b); _a_ < _b_ ? _a_ : _b_; })
# define CT_MAX(_a, _b)         ({ __typeof__(_a) _a_ = (_a); __typeof__(_b) _b_ = (_b); _a_ > _b_ ? _a_ : _b_; })
#define CT_CLAMP(_val, _min, _max)                                   \
	({                                                               \
		__typeof__(_val) _val_ = (_val);                             \
		__typeof__(_min) _min_ = (_min);                             \
		__typeof__(_max) _max_ = (_max);                             \
		(_val_ < _min_) ? _min_ : ((_val_ > _max_) ? _max_ : _val_); \
	})
#else
# define CT_MIN(_a, _b)             ((_a) < (_b) ? (_a) : (_b))
# define CT_MAX(_a, _b)             ((_a) > (_b) ? (_a) : (_b))
# define CT_CLAMP(_val, _min, _max) CT_MIN(CT_MAX(_val, _min), _max)
#endif

// offset of member
# ifndef OFFSET_OF
#   define OFFSET_OF(_type, _member)				offsetof(_type, _member)
# endif
// container of
# ifndef CONTAINER_OF
# 	define CONTAINER_OF(_ptr, _type, _member)		(_type *)((_ptr) == NULL ? NULL : ((char *)(_ptr)-OFFSET_OF(_type, _member)))
# endif

// current function name, file name, and line number
# if defined __cplusplus ? __GNUC_PREREQ(2, 6) : __GNUC_PREREQ(2, 4)
#	define __ct_func__		__extension__ __PRETTY_FUNCTION__
#	define __ct_file__		__FILE__
#	define __ct_line__		__LINE__
# elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#	define __ct_func__		__func__
#	define __ct_file__		__FILE__
#	define __ct_line__		__LINE__
# elif defined(__GNUC__)
#	define __ct_func__		__FUNCTION__
#	define __ct_file__		__FILE__
#	define __ct_line__		__LINE__
# elif defined(_MSC_VER)
#	define __ct_func__		__FUNCTION__
#	define __ct_file__		__FILE__
#	define __ct_line__		__LINE__
# elif defined(__TINYC__)
#	define __ct_func__		__func__
#	define __ct_file__		__FILE__
#	define __ct_line__		__LINE__
# else
#	define __ct_func__		"(nil)"
#	define __ct_file__		"(nil)"
#	define __ct_line__		0
# endif

// filename without path (prefer compile-time when available)
# if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 12)
#   define __ct_filename__  __FILE_NAME__
# else
#   define __ct_filename__  (strrchr(STR_SEPARATOR __ct_file__, STR_SEPARATOR_CHAR) + 1)
# endif

// inline keyword compatibility
# ifndef __cplusplus
#   if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#       ifndef inline // Only needed in C89/C90 mode (C99+ has inline as keyword)
#           define inline __inline
#       endif
#   endif
# endif
# ifndef CT_INLINE
#   define CT_INLINE 				static inline
# endif
// force inline
# if defined(_MSC_VER)
#	define __ct_force_inline		__forceinline
# elif __GNUC_PREREQ(3,2)
#  	define __ct_force_inline		__always_inline
# else
#  	define __ct_force_inline		inline
# endif

# if defined(__GNUC__) || defined(__clang__)
#   define __ct_attribute__(...)		__attribute__(__VA_ARGS__)				// GCC/Clang attribute declaration
#   define __ct_attribute_weak__ 		__attribute__((weak))					// Mark weak reference function
#   define __ct_packed__ 				__attribute__((packed))					// Packed struct (GCC/Clang)
#   define __ct_aligned__(_n)  			__attribute__((aligned(_n)))			// Aligned struct
#   define __ct_packed_aligned__(_n)	__attribute__((packed, aligned(_n)))	// Packed and aligned
# elif defined(_MSC_VER)
#   define __ct_attribute__(...)
#   define __ct_attribute_weak__
#   define __ct_packed__                /* WARNING: Use CT_PACK_PUSH/POP for MSVC packing */
#   define __ct_aligned__(_n)  			__declspec(align(_n))
#   define __ct_packed_aligned__(_n)	__declspec(align(_n))
# else
#   define __ct_attribute__(...)
#   define __ct_attribute_weak__
#   define __ct_packed__
#   define __ct_aligned__(_n)
#   define __ct_packed_aligned__(_n)
# endif

// MSVC-compatible packing macros (use around struct definitions)
# ifdef _MSC_VER
#   define CT_PACK_PUSH(_n)	    __pragma(pack(push, _n))
#   define CT_PACK_POP()		__pragma(pack(pop))
# elif defined(__GNUC__) || defined(__clang__)
#   define CT_PACK_PUSH(_n)	    _Pragma("pack(push, " #_n ")")
#   define CT_PACK_POP()		_Pragma("pack(pop)")
# else
#   define CT_PACK_PUSH(_n)
#   define CT_PACK_POP()
# endif

# if !defined __cplusplus && __GNUC_PREREQ(3,3) && defined(__THROW)
#   define __ct_throw __THROW
# else
#   define __ct_throw
# endif

#endif // COTER_CORE_MACRO_H
