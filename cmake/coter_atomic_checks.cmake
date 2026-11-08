#[=======================================================================[.rst:
Setup Atomic Operations Detection
---------------------------------

Detects the best available atomic implementation and sets exactly one
``CT_ATOMIC_USE_*`` variable to `TRUE` (others to `FALSE`).

Priority fallback:
1. GCC ``__atomic_*`` builtins (C11-style)
2. GCC ``__sync_*`` builtins (Legacy)
3. Windows Interlocked API
4. Mutex fallback (if all above fail)
#]=======================================================================]

include_guard(GLOBAL)

include(CheckCSourceCompiles)
include(CMakePushCheckState)

set(CT_ATOMIC_IMPL_FLAG FALSE)
set(CT_ATOMIC_IMPL_STR "")
set(CT_ATOMIC_LIBRARIES "")

set(CT_ATOMIC_USE_GCC_ATOMIC FALSE)
set(CT_ATOMIC_USE_GCC_SYNC FALSE)
set(CT_ATOMIC_USE_WIN FALSE)
set(CT_ATOMIC_USE_MUTEX FALSE)

# GCC __atomic_* builtins (C11-style)
if(NOT CT_ATOMIC_IMPL_FLAG)
  set(CT_ATOMIC_SRC_C11 "
    #include <stdint.h>
    int main(void) {
      int x = 0;
      int64_t y = 0;
      __atomic_fetch_add(&x, 1, __ATOMIC_SEQ_CST);
      __atomic_load_n(&x, __ATOMIC_ACQUIRE);
      __atomic_store_n(&x, 1, __ATOMIC_RELEASE);
      __atomic_fetch_add(&y, 1, __ATOMIC_SEQ_CST);
      __atomic_load_n(&y, __ATOMIC_ACQUIRE);
      __atomic_store_n(&y, 1, __ATOMIC_RELEASE);
      return x + (int)y;
    }
  ")

  cmake_push_check_state()
  check_c_source_compiles("${CT_ATOMIC_SRC_C11}" CT_HAVE_GCC_ATOMIC)

  if(NOT CT_HAVE_GCC_ATOMIC)
    list(APPEND CMAKE_REQUIRED_LIBRARIES atomic)
    check_c_source_compiles("${CT_ATOMIC_SRC_C11}" CT_HAVE_GCC_ATOMIC_WITH_LIBATOMIC)
    if(CT_HAVE_GCC_ATOMIC_WITH_LIBATOMIC)
      set(CT_ATOMIC_LIBRARIES atomic)
    endif()
  endif()
  cmake_pop_check_state()

  if(CT_HAVE_GCC_ATOMIC OR CT_HAVE_GCC_ATOMIC_WITH_LIBATOMIC)
    set(CT_ATOMIC_USE_GCC_ATOMIC TRUE)
    set(CT_ATOMIC_IMPL_FLAG TRUE)
    set(CT_ATOMIC_IMPL_STR "GCC __atomic builtins")
  endif()

  unset(CT_ATOMIC_SRC_C11)
endif()

# GCC __sync_* builtins (Legacy)
if(NOT CT_ATOMIC_IMPL_FLAG)
  set(CT_ATOMIC_SRC_SYNC "
    #include <stdint.h>
    int main(void) {
      int x = 0;
      int64_t y = 0;
      __sync_fetch_and_add(&x, 1);
      __sync_val_compare_and_swap(&x, 0, 0);
      (void)__sync_lock_test_and_set(&x, 1);
      __sync_fetch_and_add(&y, 1);
      __sync_val_compare_and_swap(&y, 0, 0);
      (void)__sync_lock_test_and_set(&y, 1);
      return x + (int)y;
    }
  ")

  cmake_push_check_state()
  check_c_source_compiles("${CT_ATOMIC_SRC_SYNC}" CT_HAVE_GCC_SYNC)

  if(NOT CT_HAVE_GCC_SYNC)
    list(APPEND CMAKE_REQUIRED_LIBRARIES atomic)
    check_c_source_compiles("${CT_ATOMIC_SRC_SYNC}" CT_HAVE_GCC_SYNC_WITH_LIBATOMIC)
    if(CT_HAVE_GCC_SYNC_WITH_LIBATOMIC)
      set(CT_ATOMIC_LIBRARIES atomic)
    endif()
  endif()
  cmake_pop_check_state()

  if(CT_HAVE_GCC_SYNC OR CT_HAVE_GCC_SYNC_WITH_LIBATOMIC)
    set(CT_ATOMIC_USE_GCC_SYNC TRUE)
    set(CT_ATOMIC_IMPL_FLAG TRUE)
    set(CT_ATOMIC_IMPL_STR "GCC __sync builtins")
  endif()

  unset(CT_ATOMIC_SRC_SYNC)
endif()

# Windows Interlocked API
if(NOT CT_ATOMIC_IMPL_FLAG AND WIN32)
  set(CT_ATOMIC_SRC_WIN "
    #include <intrin.h>
    int main(void) {
      volatile long x = 0;
      volatile long long y = 0;
      _InterlockedExchange(&x, 1);
      _InterlockedCompareExchange(&x, 2, 1);
      _InterlockedExchangeAdd(&x, 1);
      _InterlockedExchange64(&y, 1LL);
      _InterlockedCompareExchange64(&y, 2LL, 1LL);
      _InterlockedExchangeAdd64(&y, 1LL);
      return (int)(x + (int)y);
    }
  ")

  cmake_push_check_state()
  check_c_source_compiles("${CT_ATOMIC_SRC_WIN}" CT_HAVE_WIN_INTERLOCKED)
  cmake_pop_check_state()

  if(CT_HAVE_WIN_INTERLOCKED)
    set(CT_ATOMIC_USE_WIN TRUE)
    set(CT_ATOMIC_IMPL_FLAG TRUE)
    set(CT_ATOMIC_IMPL_STR "Windows Interlocked")
  endif()

  unset(CT_ATOMIC_SRC_WIN)
endif()

# Mutex Fallback
if(NOT CT_ATOMIC_IMPL_FLAG)
  set(CT_ATOMIC_USE_MUTEX TRUE)
  set(CT_ATOMIC_IMPL_STR "Mutex fallback")
endif()

if(CT_ATOMIC_LIBRARIES)
  target_link_libraries(coter_internal_options INTERFACE ${CT_ATOMIC_LIBRARIES})
endif()

message(STATUS "Atomic implementation selected: ${CT_ATOMIC_IMPL_STR}")
