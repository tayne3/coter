#[=======================================================================[.rst:
Setup System Libraries
----------------------

Checks for and links against required system libraries based on feature
detection rather than hardcoded OS checks where possible.
#]=======================================================================]

coter_check_header("inttypes.h")
coter_check_header("stdbool.h")
coter_check_header("sys/types.h")
coter_check_header("sys/stat.h")

coter_check_function("memrchr" "string.h")
coter_check_function("clock_gettime" "time.h")
coter_check_function("sem_timedwait" "semaphore.h")
coter_check_function("pthread_rwlock_init" "pthread.h")

if(UNIX AND NOT APPLE)
  coter_check_library_function("rt" "clock_gettime" "")
  if(HAVE_CLOCK_GETTIME_IN_RT)
    target_link_libraries(coter_internal_options INTERFACE rt)
  endif()
endif()
