# ===========================================
# Setup System Libraries
#
# Overview:
#   Checks for and links against required system libraries based on feature detection
#   rather than hardcoded OS checks where possible.
# ===========================================

project_check_header("stdbool.h")
project_check_header("stdint.h")
project_check_header("stdatomic.h")
project_check_header("sys/types.h")
project_check_header("sys/stat.h")
project_check_header("sys/time.h")
project_check_header("fcntl.h")
project_check_header("pthread.h")
project_check_header("endian.h")
project_check_header("sys/endian.h")

project_check_function("gettid" "unistd.h")
project_check_function("strlcpy" "string.h")
project_check_function("strlcat" "string.h")
project_check_function("memrchr" "string.h")
project_check_function("clock_gettime" "time.h")
project_check_function("gettimeofday" "sys/time.h")
project_check_function("sem_timedwait" "semaphore.h")
project_check_function("pipe" "unistd.h")
project_check_function("socketpair" "sys/socket.h")
project_check_function("eventfd" "sys/eventfd.h")
project_check_function("setproctitle" "unistd.h")
project_check_function("pthread_spin_lock" "pthread.h")
project_check_function("pthread_mutex_timedlock" "pthread.h")
project_check_function("pthread_rwlock_init" "pthread.h")

project_check_struct("struct timespec" "tv_sec" "time.h")
project_check_struct("struct timezone" "tz_minuteswest" "sys/time.h")
if(WIN32)
  project_check_struct("struct timeval" "tv_sec" "winsock2.h")
else()
  project_check_struct("struct timeval" "tv_sec" "sys/time.h")
endif()
project_check_library_function("rt" "clock_gettime" "")

if(WIN32)
  target_link_libraries(coter_compile_dependency INTERFACE 
    secur32 crypt32 winmm iphlpapi ws2_32 dbghelp
  )
elseif(UNIX)
  check_library_exists(m pow "" HAVE_LIBM)
  if(HAVE_LIBM)
    target_link_libraries(coter_compile_dependency INTERFACE m)
  endif()
  if(CMAKE_DL_LIBS)
    target_link_libraries(coter_compile_dependency INTERFACE ${CMAKE_DL_LIBS})
  endif()
  if(HAVE_CLOCK_GETTIME_IN_RT)
    target_link_libraries(coter_compile_dependency INTERFACE rt)
  endif()
endif()
