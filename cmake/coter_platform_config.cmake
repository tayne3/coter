#[=======================================================================[.rst:
Setup System Libraries
----------------------

Checks for and links against required system libraries based on feature
detection rather than hardcoded OS checks where possible.
#]=======================================================================]

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckLibraryExists)

find_package(Threads REQUIRED)

get_target_property(_COTER_INTERFACE_DEFS coter_internal_options INTERFACE_COMPILE_DEFINITIONS)
set(_COTER_SAVED_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
if(_COTER_INTERFACE_DEFS)
  foreach(_def IN LISTS _COTER_INTERFACE_DEFS)
    if(_def MATCHES "^-D")
      list(APPEND CMAKE_REQUIRED_DEFINITIONS "${_def}")
    else()
      list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D${_def}")
    endif()
  endforeach()
endif()

set(_COTER_SAVED_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
set(CMAKE_REQUIRED_LIBRARIES Threads::Threads)

check_include_files("inttypes.h" HAVE_INTTYPES_H)
check_include_files("stdbool.h" HAVE_STDBOOL_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)

check_symbol_exists("memrchr" "string.h" HAVE_MEMRCHR)
check_symbol_exists("clock_gettime" "time.h" HAVE_CLOCK_GETTIME)
check_symbol_exists("sem_timedwait" "semaphore.h" HAVE_SEM_TIMEDWAIT)
check_symbol_exists("pthread_rwlock_init" "pthread.h" HAVE_PTHREAD_RWLOCK)
check_symbol_exists("pthread_cond_clockwait" "pthread.h" HAVE_PTHREAD_COND_CLOCKWAIT)

if(UNIX AND NOT APPLE)
  if(NOT HAVE_CLOCK_GETTIME)
    check_library_exists("rt" "clock_gettime" "" HAVE_CLOCK_GETTIME_IN_RT)
    if(HAVE_CLOCK_GETTIME_IN_RT)
      set(HAVE_CLOCK_GETTIME TRUE)
      target_link_libraries(coter_internal_options INTERFACE rt)
    endif()
  endif()
endif()

set(CMAKE_REQUIRED_LIBRARIES "${_COTER_SAVED_REQUIRED_LIBRARIES}")
set(CMAKE_REQUIRED_DEFINITIONS "${_COTER_SAVED_REQUIRED_DEFINITIONS}")

target_link_libraries(coter_internal_options INTERFACE Threads::Threads)
