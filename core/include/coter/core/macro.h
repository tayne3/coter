/**
 * @file macro.h
 * @brief Basic definitions and macros
 * @note Provides fundamental types, safety macros, common utilities, and platform specifics.
 */
#ifndef COTER_CORE_MACRO_H
#define COTER_CORE_MACRO_H

#include <stddef.h>

#include "coter/core/config.h"

// clang-format off

#define CT_CXX_98 199711L
#define CT_CXX_03 199711L
#define CT_CXX_11 201103L
#define CT_CXX_14 201402L
#define CT_CXX_17 201703L
#define CT_CXX_20 202002L
#define CT_CXX_23 202302L

#ifndef __cplusplus
#undef CT_CPLUSPLUS
#elif defined(_MSVC_LANG)
#define CT_CPLUSPLUS _MSVC_LANG
#else
#define CT_CPLUSPLUS __cplusplus
#endif

// Detect __has_*.
#ifdef __has_feature
#define CT_HAS_FEATURE(x_) __has_feature(x_)
#else
#define CT_HAS_FEATURE(x_) 0
#endif
#ifdef __has_include
#define CT_HAS_INCLUDE(x_) __has_include(x_)
#else
#define CT_HAS_INCLUDE(x_) 0
#endif
#ifdef __has_builtin
#define CT_HAS_BUILTIN(x_) __has_builtin(x_)
#else
#define CT_HAS_BUILTIN(x_) 0
#endif

#ifdef __has_attribute
#define CT_HAS_ATTRIBUTE(x_) __has_attribute(x_)
#else
#define CT_HAS_ATTRIBUTE(x_) 0
#endif
#ifdef __has_c_attribute
#define CT_HAS_C_ATTRIBUTE(x_) __has_c_attribute(x_)
#else
#define CT_HAS_C_ATTRIBUTE(x_) 0
#endif
#ifdef __has_cpp_attribute
#define CT_HAS_CPP_ATTRIBUTE(x_) __has_cpp_attribute(x_)
#else
#define CT_HAS_CPP_ATTRIBUTE(x_) 0
#endif

#ifdef __GNUC_PREREQ
#define CT_GNUC_PREREQ(maj_, min_) __GNUC_PREREQ(maj_, min_)
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define CT_GNUC_PREREQ(maj_, min_) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj_) << 16) + (min_))
#else
#define CT_GNUC_PREREQ(maj_, min_) 0
#endif

#if defined(__clang__) && defined(__has_extension)
#define CT_CLANG_HAS_EXTENSION(x_) __has_extension(x_)
#else
#define CT_CLANG_HAS_EXTENSION(x_) 0
#endif

#if defined(__cplusplus) && (defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND))
#define CT_HAS_EXCEPTIONS 1
#else
#define CT_HAS_EXCEPTIONS 0
#endif

// COMPILER
#if defined(__clang__)
#define CT_COMPILER_CLANG
#elif defined(__GNUC__)
#define CT_COMPILER_GCC
#elif defined(_MSC_VER)
#define CT_COMPILER_MSVC
#else
#define CT_COMPILER_UNKNOWN
#warning "Untested compiler!"
#endif

// OS
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#	define CT_OS_WIN64
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#	define CT_OS_WIN32
#elif defined(WINCE) || defined(_WIN32_WCE)
#	define CT_OS_WINCE
#elif defined(linux) || defined(__linux__) || defined(__linux)
#	define CT_OS_LINUX
#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#    include <TargetConditionals.h>
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#   define CT_OS_IOS
#elif defined(TARGET_OS_OSX) && TARGET_OS_OSX
#   define CT_OS_MAC
#endif
#    define CT_OS_DARWIN
#else
#	error "Unsupported platform!"
#endif

#if defined(CT_OS_WIN64) || defined(CT_OS_WIN32) || defined(CT_OS_WINCE)
    #define CT_OS_WIN
#else
    #define CT_OS_UNIX
#endif

#if defined(__LP64__) || defined(__64BIT__) || defined(_LP64) || defined(__x86_64) || defined(__x86_64__) ||           \
	 defined(__amd64) || defined(__amd64__) || defined(__arm64) || defined(__arm64__) || defined(__sparc64__) ||        \
	 defined(__PPC64__) || defined(__ppc64__) || defined(__powerpc64__) || defined(__loongarch64) || defined(_M_X64) || \
	 defined(_M_AMD64) || defined(_M_ARM64) || defined(_M_IA64) || defined(__ia64__) || defined(__ia64) ||              \
	 (defined(__WORDSIZE) && (__WORDSIZE == 64)) || (defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 8)) ||       \
	 defined(TCC_TARGET_X86_64)
#	define CT_WORDSIZE 64
#else
#	define CT_WORDSIZE 32
#endif

#define CT_CONCAT_IMPL(a_, b_) a_##b_
#define CT_CONCAT(a_, b_)      CT_CONCAT_IMPL(a_, b_)

#define CT_STRINGIFY_ARG(x_) #x_
#define CT_STRINGIFY(x_)     CT_STRINGIFY_ARG(x_)

#ifdef __COUNTER__
#   define CT_UNIQUE_ID(prefix_) CT_CONCAT(prefix_, __COUNTER__)
#else
#   define CT_UNIQUE_ID(prefix_) CT_CONCAT(prefix_, __LINE__)
#endif

#ifndef CT_STATIC_ASSERT
#   if defined(__cplusplus) && \
       ((defined(CT_CPLUSPLUS) && CT_CPLUSPLUS >= CT_CXX_11) || (defined(_MSC_VER) && _MSC_VER >= 1600))
#     define CT_STATIC_ASSERT(expr_) static_assert((expr_), #expr_)
#   elif !defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#     define CT_STATIC_ASSERT(expr_) static_assert((expr_), #expr_)
#   elif !defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#     define CT_STATIC_ASSERT(expr_) _Static_assert((expr_), #expr_)
#   elif !defined(__cplusplus) && CT_CLANG_HAS_EXTENSION(c_static_assert)
#     define CT_STATIC_ASSERT(expr_) __extension__ _Static_assert((expr_), #expr_)
#   elif !defined(__cplusplus) && CT_GNUC_PREREQ(4, 6)
#     define CT_STATIC_ASSERT(expr_) __extension__ _Static_assert((expr_), #expr_)
#   else
#     define CT_STATIC_ASSERT(expr_) \
          typedef char CT_UNIQUE_ID(ct_static_assert_)[(expr_) ? 1 : -1]
#   endif
#endif

#ifndef __cplusplus
#	if HAVE_STDBOOL_H
#		include <stdbool.h>
#	else
#		ifndef bool
#			define bool int
#		endif
#		ifndef true
#			define true 1
#		endif
#		ifndef false
#			define false 0
#		endif
#	endif
#endif

#if defined(_MSC_VER) && _MSC_VER < 1700
    typedef __int8              int8_t;
    typedef __int16             int16_t;
    typedef __int32             int32_t;
    typedef __int64             int64_t;

    typedef unsigned __int8     uint8_t;
    typedef unsigned __int16    uint16_t;
    typedef unsigned __int32    uint32_t;
    typedef unsigned __int64    uint64_t;
#else
    #include <stdint.h>
#endif

// current function name
#if defined(__cplusplus) && CT_GNUC_PREREQ(2, 6)
#  define CT_STRFUNC __extension__ __PRETTY_FUNCTION__
#elif !defined(__cplusplus) && CT_GNUC_PREREQ(2, 4)
#  define CT_STRFUNC __extension__ __PRETTY_FUNCTION__
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define CT_STRFUNC __func__
#elif defined(__cplusplus) && CT_CPLUSPLUS >= CT_CXX_11
#  define CT_STRFUNC __func__
#elif defined(__FUNCTION__)
#  define CT_STRFUNC __FUNCTION__
#elif defined(__TINYC__)
#  define CT_STRFUNC __func__
#else
#  define CT_STRFUNC "(unknown)"
#endif

#define CT_FILE __FILE__
#define CT_LINE __LINE__

#define CT_STRLINE CT_STRINGIFY(__LINE__)
#define CT_STRLOC  CT_FILE ":" CT_STRLINE

// empty string
#ifndef STR_NULL
#   define STR_NULL ""
#endif
// string is empty
#ifndef STR_ISEMPTY
#   define STR_ISEMPTY(s_) 	(!(s_) || !*(const char *)(s_))
#endif
// newline
#ifndef STR_NEWLINE
#   define STR_NEWLINE "\n"
#endif

// newline and separator
#ifdef CT_OS_WIN
#   define STR_SEPARATOR "\\"
#   define STR_SEPARATOR_CHAR '\\'
#else
#   define STR_SEPARATOR "/"
#   define STR_SEPARATOR_CHAR '/'
#endif

// array size
#if defined(__cplusplus)
#if CT_CPLUSPLUS >= CT_CXX_11
    template <typename T, size_t N>
    constexpr size_t ct_arrsize_impl(const T (&)[N]) noexcept { return N; }
#else
    template <typename T, size_t N>
    size_t ct_arrsize_impl(const T (&)[N]) { return N; }
#endif
#define CT_ARRSIZE(arr_) (ct_arrsize_impl(arr_))
#elif defined(__GNUC__) || defined(__clang__)
#define CT__BUILD_BUG_ON_ZERO(e_) (sizeof(struct { int : -!!(e_); }))
#define CT__MUST_BE_ARRAY(arr_) \
    CT__BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(__typeof__(arr_), __typeof__(&(arr_)[0])))
#define CT_ARRSIZE(arr_) (sizeof(arr_) / sizeof((arr_)[0]) + CT__MUST_BE_ARRAY(arr_))
#else
#define CT_ARRSIZE(arr_) (sizeof(arr_) / sizeof((arr_)[0]))
#endif

// variable unused
#define CT_UNUSED(var_) (void)(var_)

// likely / unlikely
#if defined(__GNUC__) || defined(__clang__)
#   define CT_LIKELY(x_)   __builtin_expect(!!(x_), 1)
#   define CT_UNLIKELY(x_) __builtin_expect(!!(x_), 0)
#else
#   define CT_LIKELY(x_)   (x_)
#   define CT_UNLIKELY(x_) (x_)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CT_ABS_IMPL(n_, id_)                           \
    __extension__({                                    \
        __typeof__(n_) id_ = (n_);                     \
        id_ > 0 ? id_ : -id_;                          \
    })
#define CT_ABS(n_) CT_ABS_IMPL(n_, CT_UNIQUE_ID(ct_n_))

#define CT_NABS_IMPL(n_, id_)                          \
    __extension__({                                    \
        __typeof__(n_) id_ = (n_);                     \
        id_ < 0 ? id_ : -id_;                          \
    })
#define CT_NABS(n_) CT_NABS_IMPL(n_, CT_UNIQUE_ID(ct_n_))

#define CT_MIN_IMPL(a_, b_, a_id_, b_id_)              \
    __extension__({                                    \
        __typeof__(a_) a_id_ = (a_);                   \
        __typeof__(b_) b_id_ = (b_);                   \
        a_id_ < b_id_ ? a_id_ : b_id_;                 \
    })
#define CT_MIN(a_, b_) CT_MIN_IMPL(a_, b_, CT_UNIQUE_ID(ct_a_), CT_UNIQUE_ID(ct_b_))

#define CT_MAX_IMPL(a_, b_, a_id_, b_id_)              \
    __extension__({                                    \
        __typeof__(a_) a_id_ = (a_);                   \
        __typeof__(b_) b_id_ = (b_);                   \
        a_id_ > b_id_ ? a_id_ : b_id_;                 \
    })
#define CT_MAX(a_, b_) CT_MAX_IMPL(a_, b_, CT_UNIQUE_ID(ct_a_), CT_UNIQUE_ID(ct_b_))

#define CT_CLAMP_IMPL(var_, min_, max_, v_id_, min_id_, max_id_)                 \
    __extension__({                                                              \
        __typeof__(var_) v_id_   = (var_);                                       \
        __typeof__(min_) min_id_ = (min_);                                       \
        __typeof__(max_) max_id_ = (max_);                                       \
        v_id_ < min_id_ ? min_id_ : (v_id_ > max_id_ ? max_id_ : v_id_);         \
    })
#define CT_CLAMP(var_, min_, max_) CT_CLAMP_IMPL(var_, min_, max_, CT_UNIQUE_ID(ct_v_), CT_UNIQUE_ID(ct_min_), CT_UNIQUE_ID(ct_max_))
#else
#define CT_ABS(n_)                 ((n_) > 0 ? (n_) : -(n_))
#define CT_NABS(n_)                ((n_) < 0 ? (n_) : -(n_))
#define CT_MIN(a_, b_)             ((a_) < (b_) ? (a_) : (b_))
#define CT_MAX(a_, b_)             ((a_) > (b_) ? (a_) : (b_))
#define CT_CLAMP(var_, min_, max_) CT_MIN(CT_MAX(var_, min_), max_)
#endif

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
    #if defined(__GNUC__) || defined(__clang__)
        #define CT_PAUSE() __asm__ volatile("pause" ::: "memory")
    #elif defined(_MSC_VER)
        #include <intrin.h>
        #define CT_PAUSE() _mm_pause()
    #endif
#elif defined(_MSC_VER) && (defined(_M_ARM64) || defined(_M_ARM))
    #include <intrin.h>
    #define CT_PAUSE() __yield()
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
    #define CT_PAUSE() __asm__ volatile(".word 0x0100000f" ::: "memory")
#elif defined(__loongarch__)
    #define CT_PAUSE() __asm__ volatile("dbar 0" ::: "memory")
#else
    #define CT_PAUSE() do {} while (0)
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
#elif defined(__APPLE__)
#include <machine/endian.h>
#if defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_BIG
#elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#define CT_ENDIAN_SYSTEM CT_ENDIAN_LITTLE
#endif
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
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

// offset of member
#ifdef offsetof
#   define CT_OFFSET_OF(t_, f_) offsetof(t_, f_)
#elif defined(__GNUC__) || defined(__clang__)
#   define CT_OFFSET_OF(t_, f_) __builtin_offsetof(t_, f_)
#else
#   define CT_OFFSET_OF(t_, f_) ((size_t)((char *)&((t_ *)0)->f_ - (char *)0))
#endif
// container of
#define CT_CONTAINER_OF(ptr_, type_, member_) (type_ *)((char *)(void *)(ptr_)-CT_OFFSET_OF(type_, member_))

// Provide 'inline' keyword for C89/C90 compatibility.
# if !defined(__cplusplus) && !defined(inline) && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L)
#   define inline __inline
# endif
// Default inline modifier (internal linkage).
# ifndef CT_INLINE
#   define CT_INLINE static inline
# endif
// Compiler-specific directive to force function inlining.
#if defined(_MSC_VER)
#	define CT_FORCE_INLINE __forceinline
#elif CT_GNUC_PREREQ(3,2)
#  	define CT_FORCE_INLINE __attribute__((always_inline)) inline
#else
#  	define CT_FORCE_INLINE inline
#endif

// fall through
#ifdef __cplusplus
#if CT_HAS_CPP_ATTRIBUTE(fallthrough) >= 201603L
#define CT_FALLTHROUGH [[fallthrough]]
#elif CT_HAS_CPP_ATTRIBUTE(clang::fallthrough)
#define CT_FALLTHROUGH [[clang::fallthrough]]
#elif CT_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#define CT_FALLTHROUGH [[gnu::fallthrough]]
#elif CT_HAS_ATTRIBUTE(fallthrough) || CT_GNUC_PREREQ(7, 0)
#define CT_FALLTHROUGH __attribute__((fallthrough))
#elif defined(_MSC_VER)
#define CT_FALLTHROUGH __pragma(warning(suppress: 26819))
#else
#define CT_FALLTHROUGH do {} while(0)
#endif
#else
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L && CT_HAS_C_ATTRIBUTE(fallthrough)
#define CT_FALLTHROUGH [[fallthrough]]
#elif CT_HAS_ATTRIBUTE(fallthrough) || CT_GNUC_PREREQ(7, 0)
#define CT_FALLTHROUGH __attribute__((fallthrough))
#elif defined(_MSC_VER)
#define CT_FALLTHROUGH __pragma(warning(suppress: 26819))
#else
#define CT_FALLTHROUGH do {} while (0)
#endif
#endif

// no discard
#if defined(__cplusplus) && (CT_CPLUSPLUS >= CT_CXX_17)
#   define CT_ATTR_NODISCARD [[nodiscard]]
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
#   define CT_ATTR_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#   define CT_ATTR_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
#   define CT_ATTR_NODISCARD _Check_return_
#else
#   define CT_ATTR_NODISCARD
#endif

// maybe unused
#if defined(__cplusplus) && (CT_CPLUSPLUS >= CT_CXX_17) /* C++17 */
#   define CT_MAYBE_UNUSED [[maybe_unused]]
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L) /* C23 */
#   define CT_MAYBE_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
#   define CT_MAYBE_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
#   define CT_MAYBE_UNUSED __pragma(warning(suppress: 4100 4101))
#else
#   define CT_MAYBE_UNUSED /* not supported */
#endif

// throw
#if !defined __cplusplus && CT_GNUC_PREREQ(3,3) && defined(__THROW)
#   define CT_ATTR_THROW __THROW
#else
#   define CT_ATTR_THROW
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define CT_ATTRIBUTE(...)   __attribute__(__VA_ARGS__)   // GCC/Clang attribute declaration
#   define CT_ATTR_WEAK        __attribute__((weak))        // Mark a weak-linkage symbol
#   define CT_ATTR_PURE        __attribute__((pure))
#   define CT_ATTR_ALIGNED(n_) __attribute__((aligned(n_)))
#   define CT_ATTR_NORETURN    __attribute__((noreturn))
#elif defined(_MSC_VER)
#   define CT_ATTRIBUTE(...)
#   define CT_ATTR_WEAK
#   define CT_ATTR_PURE
#   define CT_ATTR_ALIGNED(n_) __declspec(align(n_))
#   define CT_ATTR_NORETURN    __declspec(noreturn)
#else
#   define CT_ATTRIBUTE(...)
#   define CT_ATTR_WEAK
#   define CT_ATTR_PURE
#   define CT_ATTR_ALIGNED(n_)
#   define CT_ATTR_NORETURN
#endif

// MSVC-compatible packing macros (use around struct definitions)
#ifdef _MSC_VER
#   define CT_PACK_PUSH(n_)         __pragma(pack(push, n_))
#   define CT_PACK_POP()            __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
#   define CT__DO_PRAGMA_IMPL(_str) _Pragma(#_str)
#   define CT_PACK_PUSH(n_)         CT__DO_PRAGMA_IMPL(pack(push, n_))
#   define CT_PACK_POP()            CT__DO_PRAGMA_IMPL(pack(pop))
#else
#   define CT_PACK_PUSH(n_)
#   define CT_PACK_POP()
#endif

#ifndef CT_API
#   if defined(CT_OS_WIN) || defined(__CYGWIN__)
#       if defined(CT_LIB_EXPORT)
#           define CT_API __declspec(dllexport)
#       elif defined(CT_SHARED)
#           define CT_API __declspec(dllimport)
#       endif
#   elif defined(CT_COMPILER_GCC) || defined(CT_COMPILER_CLANG)
#       if defined(CT_LIB_EXPORT) || defined(CT_SHARED)
#           define CT_API __attribute__((visibility("default")))
#       endif
#   endif
#endif
#ifndef CT_API
#   define CT_API
#endif

// clang-format on

#endif  // COTER_CORE_MACRO_H
