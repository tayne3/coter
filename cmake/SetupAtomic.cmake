# ===========================================
# Setup Atomic Operations Detection
#
# Detects the best available atomic implementation and sets exactly one
# CT_ATOMIC_USE_* variable to 1, all others to 0.
#
# Priority: GCC → Windows → Mutex
# ===========================================

include(CheckCSourceCompiles)

set(CT_ATOMIC_USE_GCC 0)
set(CT_ATOMIC_USE_WIN 0)
set(CT_ATOMIC_USE_MUTEX 0)

set(_SAVED_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
set(_SAVED_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")

# ===========================================
# GCC __atomic_* builtins (C11-style)
# ===========================================
if(NOT CT_ATOMIC_IMPL_SELECTED)
  check_c_source_compiles("
    #include <stdint.h>
    int main(void) {
      int x = 0;
      int64_t y = 0;
      __atomic_fetch_add(&x, 1, __ATOMIC_SEQ_CST);
      __atomic_load_n(&x, __ATOMIC_ACQUIRE);
      __atomic_store_n(&x, 1, __ATOMIC_RELEASE);
      __atomic_fetch_add(&y, 1, __ATOMIC_SEQ_CST);
      return x + (int)y;
    }
  " CT_HAVE_GCC_ATOMIC)

  if(NOT CT_HAVE_GCC_ATOMIC)
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};atomic")
    check_c_source_compiles("
      #include <stdint.h>
      int main(void) {
        int x = 0;
        int64_t y = 0;
        __atomic_fetch_add(&x, 1, __ATOMIC_SEQ_CST);
        __atomic_fetch_add(&y, 1, __ATOMIC_SEQ_CST);
        return x + (int)y;
      }
    " CT_HAVE_GCC_ATOMIC_WITH_LIBATOMIC)
    if(CT_HAVE_GCC_ATOMIC_WITH_LIBATOMIC)
      set(CT_HAVE_GCC_ATOMIC TRUE)
      target_link_libraries(coter_compile_dependency INTERFACE atomic)
    endif()
  endif()

  if(CT_HAVE_GCC_ATOMIC)
    set(CT_ATOMIC_USE_GCC 1)
    set(CT_ATOMIC_GCC_TYPE "C11")
    set(CT_ATOMIC_IMPL_SELECTED TRUE)
  endif()
endif()

# ===========================================
# GCC __sync_* builtins (Legacy)
# ===========================================
if(NOT CT_ATOMIC_IMPL_SELECTED)
  check_c_source_compiles("
    #include <stdint.h>
    int main(void) {
      int x = 0;
      int64_t y = 0;
      __sync_fetch_and_add(&x, 1);
      __sync_fetch_and_add(&y, 1);
      return x + (int)y;
    }
  " CT_HAVE_GCC_SYNC)
  
  if(NOT CT_HAVE_GCC_SYNC)
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};atomic")
    check_c_source_compiles("
      #include <stdint.h>
      int main(void) {
        int x = 0;
        int64_t y = 0;
        __sync_fetch_and_add(&x, 1);
        __sync_fetch_and_add(&y, 1);
        return x + (int)y;
      }
    " CT_HAVE_GCC_SYNC_WITH_LIBATOMIC)
    if(CT_HAVE_GCC_SYNC_WITH_LIBATOMIC)
      set(CT_HAVE_GCC_SYNC TRUE)
      target_link_libraries(coter_compile_dependency INTERFACE atomic)
    endif()
  endif()
  
  if(CT_HAVE_GCC_SYNC)
    set(CT_ATOMIC_USE_GCC 1)
    set(CT_ATOMIC_GCC_TYPE "Legacy")
    set(CT_ATOMIC_IMPL_SELECTED TRUE)
    target_compile_definitions(coter_compile_dependency INTERFACE CT_ATOMIC_USE_GCC_SYNC)
  endif()
endif()

# ===========================================
# Windows Interlocked API
# ===========================================
if(NOT CT_ATOMIC_IMPL_SELECTED AND WIN32)
  set(CMAKE_REQUIRED_FLAGS "${_SAVED_CMAKE_REQUIRED_FLAGS}")
  set(CMAKE_REQUIRED_LIBRARIES "${_SAVED_CMAKE_REQUIRED_LIBRARIES}")
  check_c_source_compiles("
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
  " CT_HAVE_WIN_INTERLOCKED)

  if(CT_HAVE_WIN_INTERLOCKED)
    set(CT_ATOMIC_USE_WIN 1)
    set(CT_ATOMIC_IMPL_SELECTED TRUE)
  endif()
endif()

set(CMAKE_REQUIRED_FLAGS "${_SAVED_CMAKE_REQUIRED_FLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${_SAVED_CMAKE_REQUIRED_LIBRARIES}")

if(CT_ATOMIC_USE_GCC)
  message(STATUS "Atomic implementation: GCC ${CT_ATOMIC_GCC_TYPE} builtins")
elseif(CT_ATOMIC_USE_WIN)
  message(STATUS "Atomic implementation: Windows Interlocked")
else()
  set(CT_ATOMIC_USE_MUTEX 1)
  message(STATUS "Atomic implementation: Mutex fallback")
endif()
