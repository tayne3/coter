#[=======================================================================[.rst:
Setup System Libraries
----------------------

Checks for and links against required system libraries based on feature
detection rather than hardcoded OS checks where possible.
#]=======================================================================]

coter_check_header("inttypes.h")
coter_check_header("stdbool.h")
coter_check_header("stdint.h")
coter_check_header("stdatomic.h")
coter_check_header("sys/types.h")
coter_check_header("sys/stat.h")
coter_check_header("sys/time.h")
coter_check_header("fcntl.h")
coter_check_header("pthread.h")
coter_check_header("endian.h")
coter_check_header("sys/endian.h")

coter_check_function("gettid" "unistd.h")
coter_check_function("strlcpy" "string.h")
coter_check_function("strlcat" "string.h")
coter_check_function("memrchr" "string.h")
coter_check_function("clock_gettime" "time.h")
coter_check_function("gettimeofday" "sys/time.h")
coter_check_function("sem_timedwait" "semaphore.h")
coter_check_function("pipe" "unistd.h")
coter_check_function("socketpair" "sys/socket.h")
coter_check_function("eventfd" "sys/eventfd.h")
coter_check_function("setproctitle" "unistd.h")
coter_check_function("pthread_spin_lock" "pthread.h")
coter_check_function("pthread_mutex_timedlock" "pthread.h")
coter_check_function("pthread_rwlock_init" "pthread.h")

coter_check_struct("struct timespec" "tv_sec" "time.h")
coter_check_struct("struct timezone" "tz_minuteswest" "sys/time.h")
if(WIN32)
  coter_check_struct("struct timeval" "tv_sec" "winsock2.h")
else()
  coter_check_struct("struct timeval" "tv_sec" "sys/time.h")
endif()
if(UNIX AND NOT APPLE)
  coter_check_library_function("rt" "clock_gettime" "")
endif()

if(WIN32)
  target_link_libraries(coter_internal_options INTERFACE
    secur32 crypt32 winmm iphlpapi ws2_32 dbghelp
  )
elseif(UNIX)
  check_library_exists(m pow "" HAVE_LIBM)
  if(HAVE_LIBM)
    target_link_libraries(coter_internal_options INTERFACE m)
  endif()
  if(CMAKE_DL_LIBS)
    target_link_libraries(coter_internal_options INTERFACE ${CMAKE_DL_LIBS})
  endif()
  if(HAVE_CLOCK_GETTIME_IN_RT)
    target_link_libraries(coter_internal_options INTERFACE rt)
  endif()
endif()
