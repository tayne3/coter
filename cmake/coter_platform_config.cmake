#[=======================================================================[.rst:
Setup System Libraries
----------------------

Checks for and links against required system libraries based on feature
detection rather than hardcoded OS checks where possible.
#]=======================================================================]

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckLibraryExists)

check_include_files("inttypes.h" HAVE_INTTYPES_H)
check_include_files("stdbool.h" HAVE_STDBOOL_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)

check_symbol_exists("memrchr" "string.h" HAVE_MEMRCHR)
check_symbol_exists("clock_gettime" "time.h" HAVE_CLOCK_GETTIME)
check_symbol_exists("sem_timedwait" "semaphore.h" HAVE_SEM_TIMEDWAIT)
check_symbol_exists("pthread_rwlock_init" "pthread.h" HAVE_PTHREAD_RWLOCK)

if(UNIX AND NOT APPLE)
  if(NOT HAVE_CLOCK_GETTIME)
    check_library_exists("rt" "clock_gettime" "" HAVE_CLOCK_GETTIME_IN_RT)
    if(HAVE_CLOCK_GETTIME_IN_RT)
      set(HAVE_CLOCK_GETTIME TRUE)
      target_link_libraries(coter_internal_options INTERFACE rt)
    endif()
  endif()
endif()
