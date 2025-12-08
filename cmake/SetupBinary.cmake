
include(CheckCSourceCompiles)

set(CT_BINARY_USE_BUILTIN 0)
set(CT_BINARY_USE_SIMD 0)
set(CT_BINARY_USE_SCALAR 0)

set(_SAVED_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
set(_SAVED_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")

# Builtin: GCC/Clang __builtin_bswap*
if(NOT CT_BINARY_IMPL_SELECTED)
  check_c_source_compiles("
    #include <stdint.h>
    int main(void) {
      uint16_t x = __builtin_bswap16(0x1234);
      uint32_t y = __builtin_bswap32(0x12345678UL);
      uint64_t z = __builtin_bswap64(0x123456789ABCDEF0ULL);
      return (int)(x + y + z);
    }
  " CT_HAVE_BUILTIN_BSWAP)
  
  if(CT_HAVE_BUILTIN_BSWAP)
    set(CT_BINARY_USE_BUILTIN 1)
    set(CT_BINARY_IMPL_SELECTED TRUE)
  endif()
endif()

# Builtin: MSVC _byteswap_*
if(NOT CT_BINARY_IMPL_SELECTED AND MSVC)
  check_c_source_compiles("
    #include <stdlib.h>
    #include <stdint.h>
    int main(void) {
      uint16_t x = _byteswap_ushort(0x1234);
      uint32_t y = _byteswap_ulong(0x12345678UL);
      uint64_t z = _byteswap_uint64(0x123456789ABCDEF0ULL);
      return (int)(x + y + z);
    }
  " CT_HAVE_MSVC_BYTESWAP)
  
  if(CT_HAVE_MSVC_BYTESWAP)
    set(CT_BINARY_USE_BUILTIN 1)
    set(CT_BINARY_IMPL_SELECTED TRUE)
  endif()
endif()

# SIMD: SSSE3 (x86/x64)
if(NOT CT_BINARY_IMPL_SELECTED)
  set(CMAKE_REQUIRED_FLAGS "${_SAVED_CMAKE_REQUIRED_FLAGS} -mssse3")
  
  check_c_source_compiles("
    #include <tmmintrin.h>
    int main(void) {
      __m128i v = _mm_set1_epi32(0x12345678);
      __m128i shuffle = _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11,
        4, 5, 6, 7, 0, 1, 2, 3
      );
      v = _mm_shuffle_epi8(v, shuffle);
      return _mm_cvtsi128_si32(v);
    }
  " CT_HAVE_SSSE3)
  
  if(CT_HAVE_SSSE3)
    set(CT_BINARY_USE_SIMD 1)
    set(CT_BINARY_IMPL_SELECTED TRUE)
    set(CT_BINARY_SIMD_TYPE "SSSE3")
    target_compile_options(coter_compile_dependency INTERFACE -mssse3)
  endif()
  
  set(CMAKE_REQUIRED_FLAGS "${_SAVED_CMAKE_REQUIRED_FLAGS}")
endif()

# SIMD: NEON (ARM)
if(NOT CT_BINARY_IMPL_SELECTED)
  check_c_source_compiles("
    #include <arm_neon.h>
    int main(void) {
      uint32x4_t v = vdupq_n_u32(0x12345678);
      uint8x16_t v8 = vreinterpretq_u8_u32(v);
      v8 = vrev32q_u8(v8);
      v = vreinterpretq_u32_u8(v8);
      return vgetq_lane_u32(v, 0);
    }
  " CT_HAVE_NEON)
  
  if(CT_HAVE_NEON)
    set(CT_BINARY_USE_SIMD 1)
    set(CT_BINARY_IMPL_SELECTED TRUE)
    set(CT_BINARY_SIMD_TYPE "NEON")
  endif()
endif()

set(CMAKE_REQUIRED_FLAGS "${_SAVED_CMAKE_REQUIRED_FLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${_SAVED_CMAKE_REQUIRED_LIBRARIES}")

if(CT_BINARY_USE_BUILTIN)
  message(STATUS "Binary implementation: Builtin (compiler intrinsics)")
elseif(CT_BINARY_USE_SIMD)
  message(STATUS "Binary implementation: SIMD (${CT_BINARY_SIMD_TYPE})")
else()
  set(CT_BINARY_USE_SCALAR 1)
  message(STATUS "Binary implementation: Scalar")
endif()
