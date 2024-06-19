/**
 * @file ct_types.h
 * @brief 基本类型定义
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_TYPES_H
#define _CT_TYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <stdint.h>

#include "ct_define.h"

// clang-format off
typedef unsigned short int      ushort_t;
typedef unsigned int            uint_t;
typedef unsigned long int       ulong_t;

typedef bool                    ct_bool_t;
typedef size_t                  ct_size_t;
typedef float                   ct_float_t;
typedef double                  ct_double_t;
typedef void *                  ct_pointer_t;
typedef ptrdiff_t               ct_ptrdiff_t;

typedef char                    ct_char_t;
typedef short                   ct_short_t;
typedef int                     ct_int_t;
typedef long                    ct_long_t;

typedef unsigned short          ct_ushort_t;
typedef unsigned int            ct_uint_t;
typedef unsigned long int       ct_ulong_t;

typedef int8_t                  ct_int8_t;
typedef int16_t                 ct_int16_t;
typedef int32_t                 ct_int32_t;
typedef int64_t                 ct_int64_t;

typedef uint8_t                 ct_uint8_t;
typedef uint16_t                ct_uint16_t;
typedef uint32_t                ct_uint32_t;
typedef uint64_t                ct_uint64_t;

# ifdef CT_MAX_SINT8
#   warning "duplicate definition"
# else
#   define CT_MAX_SINT8 __SCHAR_MAX__
# endif
# ifdef CT_MAX_UINT8
#   warning "duplicate definition"
# else
#   if __SCHAR_MAX__ == __INT_MAX__
#       define CT_MAX_UINT8 (CT_MAX_SINT8 * 2U + 1U)
#   else
#       define CT_MAX_UINT8 (CT_MAX_SINT8 * 2 + 1)
#   endif
# endif
# ifdef CT_MIN_SINT8
#   warning "duplicate definition"
# else
#   define CT_MIN_SINT8 (-CT_MAX_SINT8 - 1)
# endif
# ifdef CT_MIN_UINT8
#   warning "duplicate definition"
# else
#   define CT_MIN_UINT8 0
# endif

# ifdef CT_MAX_SINT16
#   warning "duplicate definition"
# else
#   define CT_MAX_SINT16 __SHRT_MAX__
# endif
# ifdef CT_MAX_UINT16
#   warning "duplicate definition"
# else
#   if __SHRT_MAX__ == __INT_MAX__
#       define CT_MAX_UINT16 (CT_MAX_SINT16 * 2U + 1U)
#   else
#       define CT_MAX_UINT16 (CT_MAX_SINT16 * 2 + 1)
#   endif
# endif
# ifdef CT_MIN_SINT16
#   warning "duplicate definition"
# else
#   define CT_MIN_SINT16 (-CT_MAX_SINT16 - 1)
# endif
# ifdef CT_MIN_UINT16
#   warning "duplicate definition"
# else
#   define CT_MIN_UINT16 0
# endif

# ifdef CT_MAX_SINT32
#   warning "duplicate definition"
# else
#   define CT_MAX_SINT32 __INT_MAX__
# endif
# ifdef CT_MAX_UINT32
#   warning "duplicate definition"
# else
#   define CT_MAX_UINT32 (CT_MAX_SINT32 * 2U + 1U)
# endif
# ifdef CT_MIN_SINT32
#   warning "duplicate definition"
# else
#   define CT_MIN_SINT32 (-CT_MAX_SINT32 - 1)
# endif
# ifdef CT_MIN_UINT32
#   warning "duplicate definition"
# else
#   define CT_MIN_UINT32 0
# endif

# ifdef CT_MAX_SLONG
#   warning "duplicate definition"
# else
#   define CT_MAX_SLONG __LONG_MAX__
# endif
# ifdef CT_MAX_ULONG
#   warning "duplicate definition"
# else
#   define CT_MAX_ULONG (CT_MAX_SLONG * 2UL + 1UL)
# endif
# ifdef CT_MIN_SLONG
#   warning "duplicate definition"
# else
#   define CT_MIN_SLONG (-CT_MAX_SLONG - 1L)
# endif
# ifdef CT_MIN_ULONG
#   warning "duplicate definition"
# else
#   define CT_MIN_ULONG 0
# endif

# ifdef CT_MAX_SINT64
#   warning "duplicate definition"
# else
#   define CT_MAX_SINT64 __LONG_LONG_MAX__
# endif
# ifdef CT_MAX_UINT64
#   warning "duplicate definition"
# else
#   define CT_MAX_UINT64 (CT_MAX_SINT64 * 2ULL + 1ULL)
# endif
# ifdef CT_MIN_SINT64
#   warning "duplicate definition"
# else
#   define CT_MIN_SINT64 (-CT_MAX_SINT64 - 1LL)
# endif
# ifdef CT_MIN_UINT64
#   warning "duplicate definition"
# else
#   define CT_MIN_UINT64 0
# endif

# ifdef CT_PRId
#   warning "duplicate definition"
# else
# 	define CT_PRId PRIi32
# endif
# ifdef CT_PRId8
#   warning "duplicate definition"
# else
# 	define CT_PRId8 PRIi8
# endif
# ifdef CT_PRId16
#   warning "duplicate definition"
# else
# 	define CT_PRId16 PRIi16
# endif
# ifdef CT_PRId32
#   warning "duplicate definition"
# else
# 	define CT_PRId32 PRIi32
# endif
# ifdef CT_PRId64
#   warning "duplicate definition"
# else
#	define CT_PRId64 PRIi64
# endif

# ifdef CT_PRIi
#   warning "duplicate definition"
# else
# 	define CT_PRIi PRIi32
# endif
# ifdef CT_PRIi8
#   warning "duplicate definition"
# else
# 	define CT_PRIi8 PRIi8
# endif
# ifdef CT_PRIi16
#   warning "duplicate definition"
# else
# 	define CT_PRIi16 PRIi16
# endif
# ifdef CT_PRIi32
#   warning "duplicate definition"
# else
# 	define CT_PRIi32 PRIi32
# endif
# ifdef CT_PRIi64
#   warning "duplicate definition"
# else
#	define CT_PRIi64 PRIi64
# endif

# ifdef CT_PRIo
#   warning "duplicate definition"
# else
# 	define CT_PRIo PRIo32
# endif
# ifdef CT_PRIo8
#   warning "duplicate definition"
# else
# 	define CT_PRIo8 PRIo8
# endif
# ifdef CT_PRIo16
#   warning "duplicate definition"
# else
# 	define CT_PRIo16 PRIo16
# endif
# ifdef CT_PRIo32
#   warning "duplicate definition"
# else
# 	define CT_PRIo32 PRIo32
# endif
# ifdef CT_PRIo64
#   warning "duplicate definition"
# else
#	define CT_PRIo64 PRIo64
# endif

# ifdef CT_PRIu
#   warning "duplicate definition"
# else
# 	define CT_PRIu PRIu32
# endif
# ifdef CT_PRIu8
#   warning "duplicate definition"
# else
# 	define CT_PRIu8 PRIu8
# endif
# ifdef CT_PRIu16
#   warning "duplicate definition"
# else
# 	define CT_PRIu16 PRIu16
# endif
# ifdef CT_PRIu32
#   warning "duplicate definition"
# else
# 	define CT_PRIu32 PRIu32
# endif
# ifdef CT_PRIu64
#   warning "duplicate definition"
# else
#	define CT_PRIu64 PRIu64
# endif

# ifdef CT_PRIx
#   warning "duplicate definition"
# else
# 	define CT_PRIx PRIx32
# endif
# ifdef CT_PRIx8
#   warning "duplicate definition"
# else
# 	define CT_PRIx8 PRIx8
# endif
# ifdef CT_PRIx16
#   warning "duplicate definition"
# else
# 	define CT_PRIx16 PRIx16
# endif
# ifdef CT_PRIx32
#   warning "duplicate definition"
# else
# 	define CT_PRIx32 PRIx32
# endif
# ifdef CT_PRIx64
#   warning "duplicate definition"
# else
#	define CT_PRIx64 PRIx64
# endif

# ifdef CT_PRIX
#   warning "duplicate definition"
# else
# 	define CT_PRIX PRIX32
# endif
# ifdef CT_PRIX8
#   warning "duplicate definition"
# else
# 	define CT_PRIX8 PRIX8
# endif
# ifdef CT_PRIX16
#   warning "duplicate definition"
# else
# 	define CT_PRIX16 PRIX16
# endif
# ifdef CT_PRIX32
#   warning "duplicate definition"
# else
# 	define CT_PRIX32 PRIX32
# endif
# ifdef CT_PRIX64
#   warning "duplicate definition"
# else
#	define CT_PRIX64 PRIX64
# endif

# ifdef CT_SCNd
#   warning "duplicate definition"
# else
# 	define CT_SCNd PRIi32
# endif
# ifdef CT_SCNd8
#   warning "duplicate definition"
# else
# 	define CT_SCNd8 PRIi8
# endif
# ifdef CT_SCNd16
#   warning "duplicate definition"
# else
# 	define CT_SCNd16 PRIi16
# endif
# ifdef CT_SCNd32
#   warning "duplicate definition"
# else
# 	define CT_SCNd32 PRIi32
# endif
# ifdef CT_SCNd64
#   warning "duplicate definition"
# else
#	define CT_SCNd64 PRIi64
# endif

# ifdef CT_SCNi
#   warning "duplicate definition"
# else
# 	define CT_SCNi PRIi32
# endif
# ifdef CT_SCNi8
#   warning "duplicate definition"
# else
# 	define CT_SCNi8 PRIi8
# endif
# ifdef CT_SCNi16
#   warning "duplicate definition"
# else
# 	define CT_SCNi16 PRIi16
# endif
# ifdef CT_SCNi32
#   warning "duplicate definition"
# else
# 	define CT_SCNi32 PRIi32
# endif
# ifdef CT_SCNi64
#   warning "duplicate definition"
# else
#	define CT_SCNi64 PRIi64
# endif

# ifdef CT_SCNo
#   warning "duplicate definition"
# else
# 	define CT_SCNo PRIo32
# endif
# ifdef CT_SCNo8
#   warning "duplicate definition"
# else
# 	define CT_SCNo8 PRIo8
# endif
# ifdef CT_SCNo16
#   warning "duplicate definition"
# else
# 	define CT_SCNo16 PRIo16
# endif
# ifdef CT_SCNo32
#   warning "duplicate definition"
# else
# 	define CT_SCNo32 PRIo32
# endif
# ifdef CT_SCNo64
#   warning "duplicate definition"
# else
#	define CT_SCNo64 PRIo64
# endif

# ifdef CT_SCNu
#   warning "duplicate definition"
# else
# 	define CT_SCNu PRIu32
# endif
# ifdef CT_SCNu8
#   warning "duplicate definition"
# else
# 	define CT_SCNu8 PRIu8
# endif
# ifdef CT_SCNu16
#   warning "duplicate definition"
# else
# 	define CT_SCNu16 PRIu16
# endif
# ifdef CT_SCNu32
#   warning "duplicate definition"
# else
# 	define CT_SCNu32 PRIu32
# endif
# ifdef CT_SCNu64
#   warning "duplicate definition"
# else
#	define CT_SCNu64 PRIu64
# endif

# ifdef CT_SCNx
#   warning "duplicate definition"
# else
# 	define CT_SCNx PRIx32
# endif
# ifdef CT_SCNx8
#   warning "duplicate definition"
# else
# 	define CT_SCNx8 PRIx8
# endif
# ifdef CT_SCNx16
#   warning "duplicate definition"
# else
# 	define CT_SCNx16 PRIx16
# endif
# ifdef CT_SCNx32
#   warning "duplicate definition"
# else
# 	define CT_SCNx32 PRIx32
# endif
# ifdef CT_SCNx64
#   warning "duplicate definition"
# else
#	define CT_SCNx64 PRIx64
# endif

# ifdef CT_SCNX
#   warning "duplicate definition"
# else
# 	define CT_SCNX PRIX32
# endif
# ifdef CT_SCNX8
#   warning "duplicate definition"
# else
# 	define CT_SCNX8 PRIX8
# endif
# ifdef CT_SCNX16
#   warning "duplicate definition"
# else
# 	define CT_SCNX16 PRIX16
# endif
# ifdef CT_SCNX32
#   warning "duplicate definition"
# else
# 	define CT_SCNX32 PRIX32
# endif
# ifdef CT_SCNX64
#   warning "duplicate definition"
# else
#	define CT_SCNX64 PRIX64
# endif

// comparison results: greater than, less than, and equal to
enum ct_compare_result {
    CTCompare_ResultLess    = -1,
    CTCompare_ResultEqual   = 0,
    CTCompare_ResultGreater = 1,
};

# ifdef STR_COMPARE_RESULT
#   warning "duplicate definition"
# else
#   define STR_COMPARE_RESULT(_result)            \
    ((_result) == CTCompare_ResultLess    ? "<" : \
     (_result) == CTCompare_ResultEqual   ? "=" : \
     (_result) == CTCompare_ResultGreater ? ">" : \
                                            "?")
# endif

/// compare flags: equal, less, less/equal, greater, greater/equal, unequal
typedef uint8_t ct_compare_flag_t;
# ifdef CTCompare_Equal
#   warning "duplicate definition"
# else
#   define CTCompare_Equal        0x01
# endif
# ifdef CTCompare_Less
#   warning "duplicate definition"
# else
#   define CTCompare_Less         0x02
# endif
# ifdef CTCompare_LessEqual
#   warning "duplicate definition"
# else
#   define CTCompare_LessEqual    0x03
# endif
# ifdef CTCompare_Greater
#   warning "duplicate definition"
# else
#   define CTCompare_Greater      0x04
# endif
# ifdef CTCompare_GreaterEqual
#   warning "duplicate definition"
# else
#   define CTCompare_GreaterEqual 0x05
# endif
# ifdef CTCompare_Unequal
#   warning "duplicate definition"
# else
#   define CTCompare_Unequal      0x06
# endif

// clang-format on
#ifdef __cplusplus
}
#endif
#endif  // _CT_TYPES_H
