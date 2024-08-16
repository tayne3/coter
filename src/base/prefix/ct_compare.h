/**
 * @file ct_compare.h
 * @brief 比较
 * @author tayne3@dingtalk.com
 * @date 2024.6.4
 */
#ifndef _CT_COMPARE_H
#define _CT_COMPARE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ct_macro.h"

// comparison results: greater than, less than, and equal to
enum ct_compare_result {
	CTCompare_ResultLess    = -1,
	CTCompare_ResultEqual   = 0,
	CTCompare_ResultGreater = 1,
};

#define STR_COMPARE_RESULT(_result)               \
	((_result) == CTCompare_ResultLess    ? "<" : \
	 (_result) == CTCompare_ResultEqual   ? "=" : \
	 (_result) == CTCompare_ResultGreater ? ">" : \
											"?")

/// compare flags: equal, less, less/equal, greater, greater/equal, unequal
typedef uint8_t ct_compare_flag_t;

#define CTCompare_Equal        0x01
#define CTCompare_Less         0x02
#define CTCompare_LessEqual    0x03
#define CTCompare_Greater      0x04
#define CTCompare_GreaterEqual 0x05
#define CTCompare_Unequal      0x06

#ifdef __cplusplus
}
#endif
#endif  // _CT_COMPARE_H
