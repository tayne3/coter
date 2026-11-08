#[=======================================================================[.rst:
Setup System Libraries
----------------------

Checks for and links against required system libraries based on feature
detection rather than hardcoded OS checks where possible.
#]=======================================================================]

include_guard(GLOBAL)

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CMakePushCheckState)

find_package(Threads REQUIRED)
target_link_libraries(coter_internal_options INTERFACE Threads::Threads)

cmake_push_check_state()
list(APPEND CMAKE_REQUIRED_LIBRARIES Threads::Threads)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
endif()

check_include_files("inttypes.h" HAVE_INTTYPES_H)
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("stdbool.h" HAVE_STDBOOL_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/types.h;sys/stat.h" HAVE_SYS_STAT_H)

check_symbol_exists("memrchr" "string.h" HAVE_MEMRCHR)
check_symbol_exists("clock_gettime" "time.h" HAVE_CLOCK_GETTIME)
check_symbol_exists("sem_timedwait" "semaphore.h" HAVE_SEM_TIMEDWAIT)
check_symbol_exists("pthread_rwlock_init" "pthread.h" HAVE_PTHREAD_RWLOCK)
check_symbol_exists("pthread_cond_clockwait" "pthread.h" HAVE_PTHREAD_COND_CLOCKWAIT)

if(UNIX AND NOT APPLE)
  foreach(_sym IN ITEMS clock_gettime sem_timedwait)
    string(TOUPPER "${_sym}" _SYM)
    if(NOT HAVE_${_SYM})
      set(_hdr "time.h")
      if(_sym STREQUAL "sem_timedwait")
        set(_hdr "semaphore.h")
      endif()
      cmake_push_check_state()
      list(APPEND CMAKE_REQUIRED_LIBRARIES rt)
      check_symbol_exists(${_sym} "${_hdr}" HAVE_${_SYM}_IN_RT)
      cmake_pop_check_state()
      if(HAVE_${_SYM}_IN_RT)
        set(HAVE_${_SYM} TRUE CACHE INTERNAL "" FORCE)
      endif()
    endif()
  endforeach()
endif()

if(HAVE_CLOCK_GETTIME_IN_RT OR HAVE_SEM_TIMEDWAIT_IN_RT)
  target_link_libraries(coter_internal_options INTERFACE rt)
endif()

cmake_pop_check_state()
