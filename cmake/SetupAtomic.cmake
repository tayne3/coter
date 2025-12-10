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
# GCC __sync_* builtins
# ===========================================
if(NOT CT_ATOMIC_IMPL_SELECTED)
  check_c_source_compiles("
    int main(void) {
      int x = 0;
      long y = 0;
      long long z = 0;
      __sync_fetch_and_add(&x, 1);
      __sync_fetch_and_sub(&x, 1);
      __sync_val_compare_and_swap(&x, 0, 1);
      __sync_lock_test_and_set(&x, 2);
      __sync_fetch_and_add(&y, 1L);
      __sync_fetch_and_sub(&y, 1L);
      __sync_val_compare_and_swap(&y, 0L, 1L);
      __sync_fetch_and_add(&z, 1LL);
      __sync_fetch_and_sub(&z, 1LL);
      __sync_val_compare_and_swap(&z, 0LL, 1LL);
      return x + y + z;
    }
  " CT_HAVE_GCC_SYNC)
  
  # Some architectures (ARM, RISC-V) need -latomic for __sync_* builtins
  if(NOT CT_HAVE_GCC_SYNC)
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};atomic")
    
    check_c_source_compiles("
      int main(void) {
        int x = 0;
        long y = 0;
        long long z = 0;
        __sync_fetch_and_add(&x, 1);
        __sync_fetch_and_sub(&x, 1);
        __sync_val_compare_and_swap(&x, 0, 1);
        __sync_lock_test_and_set(&x, 2);
        __sync_fetch_and_add(&y, 1L);
        __sync_fetch_and_sub(&y, 1L);
        __sync_val_compare_and_swap(&y, 0L, 1L);
        __sync_fetch_and_add(&z, 1LL);
        __sync_fetch_and_sub(&z, 1LL);
        __sync_val_compare_and_swap(&z, 0LL, 1LL);
        return x + y + z;
      }
    " CT_HAVE_GCC_SYNC_WITH_LIBATOMIC)
    
    if(CT_HAVE_GCC_SYNC_WITH_LIBATOMIC)
      set(CT_HAVE_GCC_SYNC TRUE)
      target_link_libraries(coter_compile_dependency INTERFACE atomic)
    endif()
  endif()
  
  if(CT_HAVE_GCC_SYNC)
    set(CT_ATOMIC_USE_GCC 1)
    set(CT_ATOMIC_IMPL_SELECTED TRUE)
  endif()
endif()

# ===========================================
# Windows Interlocked API
# ===========================================
if(NOT CT_ATOMIC_IMPL_SELECTED AND WIN32)
  set(CMAKE_REQUIRED_FLAGS "${_SAVED_CMAKE_REQUIRED_FLAGS}")
  set(CMAKE_REQUIRED_LIBRARIES "${_SAVED_CMAKE_REQUIRED_LIBRARIES}")
  check_c_source_compiles("
    #include <windows.h>
    int main(void) {
      volatile LONG x = 0;
      volatile LONGLONG y = 0;
      InterlockedExchange(&x, 1);
      InterlockedCompareExchange(&x, 2, 1);
      InterlockedExchangeAdd(&x, 1);
      InterlockedExchange64(&y, 1LL);
      InterlockedCompareExchange64(&y, 2LL, 1LL);
      InterlockedExchangeAdd64(&y, 1LL);
      return (int)(x + y);
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
  message(STATUS "Atomic implementation: GCC __sync_* builtins")
elseif(CT_ATOMIC_USE_WIN)
  message(STATUS "Atomic implementation: Windows Interlocked")
else()
  set(CT_ATOMIC_USE_MUTEX 1)
  message(STATUS "Atomic implementation: Mutex fallback")
endif()
