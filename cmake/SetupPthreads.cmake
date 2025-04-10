# ==============================================================================
# Setup Pthreads Library
#
# This module will define the following variables:
#   - PTHREADS_LIBS - The libraries needed for threading.
# ==============================================================================
# 配置 Pthreads 库
#
# 本模块将定义以下变量：
#   - PTHREADS_LIBS - 线程库的链接选项。
# ==============================================================================

include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CMakePushCheckState)
include(CheckLibraryExists)

# Use pthread on non-Windows platforms
if(NOT WIN32)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)

    if(CMAKE_USE_PTHREADS_INIT)
        set(PTHREADS_LIBS ${CMAKE_THREAD_LIBS_INIT})
        message(STATUS "Found pthread library: ${CMAKE_THREAD_LIBS_INIT}")
    else()
        message(FATAL_ERROR "Could not find pthread library. See README.Win32 for more information.")
    endif()
    return()
endif()

# Check if we have -lpthreads library
check_library_exists(pthreads pthread_create "" CMAKE_HAVE_PTHREADS_CREATE)
if(CMAKE_HAVE_PTHREADS_CREATE)
    message(STATUS "Found pthreads library: ${CMAKE_THREAD_LIBS_INIT}")
    set(PTHREADS_LIBS "-lpthreads")
    return()
endif()

# Check if we have -lpthread library
check_library_exists(pthread pthread_create "" CMAKE_HAVE_PTHREAD_CREATE)
if(CMAKE_HAVE_PTHREAD_CREATE)
    message(STATUS "Found pthread library: ${CMAKE_THREAD_LIBS_INIT}")
    set(PTHREADS_LIBS "-lpthread")
    return()
endif()

# fetch and build pthread-win32
message(STATUS "No system pthread library found, using pthread-win32 from CPM")
CPMAddPackage(
    NAME pthread-win32
    GIT_REPOSITORY "https://github.com/GerHobbelt/pthread-win32.git"
    GIT_TAG "3309f4d6e7538f349ae450347b02132ecb0606a7"
    OPTIONS "BUILD_SHARED_LIBS OFF" "BUILD_TESTING OFF"
)

# Configure exception handling scheme for pthread library
#
# Available exception handling schemes
# C (Console Mode): Standard console mode, suitable for text output and input.
# CE (Console Extended Mode): Extended console mode, providing richer console features.
# SE (GUI Mode): Structured Exception Handling, suitable for GUI applications.
#
if(NOT DEFINED PTHREADS_EXCEPTION_SCHEME)
    # Assign default if needed 
    set(PTHREADS_EXCEPTION_SCHEME "C")
    message(STATUS "Using default pthread exception scheme: Console Mode (C)")
else()
    # Validate
    if(NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "C" AND
        NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "CE" AND
        NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "SE")
        message(FATAL_ERROR "See documentation for FindPthreads.cmake, only C, CE, and SE modes are allowed")
    elseif(NOT MSVC AND PTHREADS_EXCEPTION_SCHEME STREQUAL "SE")
        message(FATAL_ERROR "Structured Exception Handling is only allowed for MSVC")
    endif()
    message(STATUS "Using pthread exception scheme: ${PTHREADS_EXCEPTION_SCHEME}")
endif()

# Set the final pthread library name
set(PTHREADS_LIBS pthreadV${PTHREADS_EXCEPTION_SCHEME}3)
message(STATUS "Build pthread library: ${PTHREADS_LIBS}")
