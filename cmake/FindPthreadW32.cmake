# ==============================================================================
# Find Pthreads Library
#
# External variables required:
#   - PTHREADS_ROOT - Root directory of the Pthreads-w32 installation.
#
# This module will define the following variables:
#   - PTHREADS_LIBS - The libraries needed for threading.
# ==============================================================================
# 查找 Pthreads 库
#
# 外部需要配置变量:
#   - PTHREADS_ROOT - 指向 Pthreads-w32 安装的根目录。
#
# 此模块将输出以下变量：
#   - PTHREADS_LIBS - 用于线程的库。
# ==============================================================================

include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CMakePushCheckState)
include(CheckLibraryExists) # for check_library_exists

# Use pthread
if(NOT WIN32)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)

    if(CMAKE_USE_PTHREADS_INIT)
        set(PTHREADS_LIBS ${CMAKE_THREAD_LIBS_INIT})
        message(STATUS "CMAKE_USE_PTHREADS_INIT: ${CMAKE_USE_PTHREADS_INIT}")
    else()
        message(FATAL_ERROR "Could not find pthread library. See README.Win32 for more information.")
    endif()
    return()
endif()

# Do we have -lpthreads
check_library_exists(pthreads pthread_create "" CMAKE_HAVE_PTHREADS_CREATE)
if(CMAKE_HAVE_PTHREADS_CREATE)
    message(STATUS "CMAKE_HAVE_PTHREADS_CREATE: ${CMAKE_HAVE_PTHREADS_CREATE}")
    set(PTHREADS_LIBS "-lpthreads")
    return()
endif()

# Ok, how about -lpthread
check_library_exists(pthread pthread_create "" CMAKE_HAVE_PTHREAD_CREATE)
if(CMAKE_HAVE_PTHREAD_CREATE)
    message(STATUS "CMAKE_HAVE_PTHREAD_CREATE: ${CMAKE_HAVE_PTHREAD_CREATE}")
    set(PTHREADS_LIBS "-lpthread")
    return()
endif()

CPMAddPackage(
    NAME pthread-win32
    GIT_REPOSITORY "https://github.com/GerHobbelt/pthread-win32.git"
    GIT_TAG "3309f4d6e7538f349ae450347b02132ecb0606a7"
    OPTIONS "BUILD_SHARED_LIBS OFF" "BUILD_TESTING OFF"
)

#
# Find the library
#
# C (Console Mode): Standard console mode, suitable for text output and input.
# CE (Console Extended Mode): Extended console mode, providing richer console features.
# SE (GUI Mode): Graphical user interface mode, suitable for applications requiring a graphical interface.
#
if(NOT DEFINED PTHREADS_EXCEPTION_SCHEME)
    # Assign default if needed
    set(PTHREADS_EXCEPTION_SCHEME "C")
else()
    # Validate
    if(NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "C" AND
        NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "CE" AND
        NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "SE")
        message(FATAL_ERROR "See documentation for FindPthreads.cmake, only C, CE, and SE modes are allowed")
    elseif(NOT MSVC AND PTHREADS_EXCEPTION_SCHEME STREQUAL "SE")
        message(FATAL_ERROR "Structured Exception Handling is only allowed for MSVC")
    endif()
endif()
set(PTHREADS_LIBS pthreadV${PTHREADS_EXCEPTION_SCHEME}3)
message(STATUS "PTHREADS_LIBS: ${PTHREADS_LIBS}")
