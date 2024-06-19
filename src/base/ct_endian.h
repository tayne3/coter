/**
 * @file ct_endian.h
 * @author tayne3@dingtalk.com
 * @date 2024.6.4
 */
#ifndef _CT_ENDIAN_H
#define _CT_ENDIAN_H
#ifdef __cplusplus
extern "C" {
#endif
// clang-format off

#include <stdbool.h>

// byte endian
typedef bool ct_endian_t;

# ifdef CTEndian_Little
#   warning "duplicate definition"
# else
#   define CTEndian_Little  false
# endif

# ifdef CTEndian_Big
#   warning "duplicate definition"
# else
#   define CTEndian_Big     true
# endif

# ifdef CTEndian_Network
#   warning "duplicate definition"
# else
#   define CTEndian_Network CTEndian_Big
# endif

# ifdef CTEndian_System
#   warning "duplicate definition"
# else
#   if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN__) ||     \
       (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || defined(__ARMEL__) || defined(__THUMBEL__) || \
        defined(__AARCH64EL__) || defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#       define CTEndian_System CTEndian_Little
#   elif (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__BIG_ENDIAN__) ||         \
         (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || defined(__ARMEB__) || defined(__THUMBEB__) ||  \
          defined(__AARCH64EB__) || defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#       define CTEndian_System CTEndian_Big
#   else
#       define CTEndian_System ((*(unsigned char *)&(unsigned int){1}) == 0)
#   endif
# endif

#ifdef __cplusplus
}
#endif
#endif  // _CT_ENDIAN_H
