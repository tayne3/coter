# ===========================================
# Setup Pthreads Library
#
# Overview:
#   This module configures and sets up the Pthreads library for threading support
#   in various platforms. It handles different detection strategies and fallback
#   mechanisms.
#
# |---------------------------------|---------------------|---------|-----------------------------------------------|
# | Option                          | Availability        | Default | Description                                   |
# |=================================|=====================|=========|===============================================|
# | PTHREADS_EXCEPTION_SCHEME       | Before Include      | "C"     | Controls pthread-win32 exception model:       |
# |                                 |                     |         |   "C"  - Console, `SetConsoleCtrlHandler`     |
# |                                 |                     |         |   "CE" - Console Extended                     |
# |                                 |                     |         |   "SE" - GUI, Structured Exception (MSVC only)|
# |---------------------------------|---------------------|---------|-----------------------------------------------|
#
# ===========================================
# 配置 Pthreads 库
#
# 概述：
#   本模块配置并设置 Pthreads 库以支持各种平台的线程功能。
#   它处理不同的检测策略和回退机制。
#
# |---------------------------------|---------------------|---------|-----------------------------------------------|
# | Option                          | Availability        | Default | Description                                   |
# |=================================|=====================|=========|===============================================|
# | PTHREADS_EXCEPTION_SCHEME       | 包含此模块前        | "C"     | 控制 pthread-win32 的异常处理模型:            |
# |                                 |                     |         |   "C"  - 控制台模式, `SetConsoleCtrlHandler`  |
# |                                 |                     |         |   "CE" - 扩展控制台模式                       |
# |                                 |                     |         |   "SE" - 图形界面模式, 结构化异常 (仅限MSVC)  |
# |---------------------------------|---------------------|---------|-----------------------------------------------|
#
# ===========================================

include(CheckLibraryExists)

# ------------------------------------------------------------------------------
# Platform-specific pthread detection
# ------------------------------------------------------------------------------

# Use standard pthread on non-Windows platforms
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

# ------------------------------------------------------------------------------
# Windows-specific pthread detection
# ------------------------------------------------------------------------------

# Check for system installed pthread libraries first
check_library_exists(pthreads pthread_create "" CMAKE_HAVE_PTHREADS_CREATE)
if(CMAKE_HAVE_PTHREADS_CREATE)
  message(STATUS "Found pthreads library")
  set(PTHREADS_LIBS "-lpthreads")
  return()
endif()

check_library_exists(pthread pthread_create "" CMAKE_HAVE_PTHREAD_CREATE)
if(CMAKE_HAVE_PTHREAD_CREATE)
  message(STATUS "Found pthread library")
  set(PTHREADS_LIBS "-lpthread")
  return()
endif()

# ------------------------------------------------------------------------------
# Fallback: Build pthread-win32 from source
# ------------------------------------------------------------------------------

message(STATUS "No system pthread library found, building from source")

# Fetch and build pthread-win32
CPMAddPackage(
  NAME pthread-win32
  GIT_REPOSITORY "https://github.com/GerHobbelt/pthread-win32.git"
  GIT_TAG "3309f4d6e7538f349ae450347b02132ecb0606a7"
  GIT_SHALLOW TRUE
  OPTIONS "BUILD_SHARED_LIBS OFF" 
          "BUILD_TESTING OFF"
)

# ------------------------------------------------------------------------------
# Configure exception handling scheme
# ------------------------------------------------------------------------------

# Apply and validate exception handling scheme configuration
if(NOT DEFINED PTHREADS_EXCEPTION_SCHEME)
  # Use default scheme if not specified
  set(PTHREADS_EXCEPTION_SCHEME "C")
  message(STATUS "Using default exception scheme: Console Mode (C)")
else()
  # Validate the provided scheme
  if(NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "C" AND
      NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "CE" AND
      NOT PTHREADS_EXCEPTION_SCHEME STREQUAL "SE")
    message(FATAL_ERROR "Invalid exception scheme. Only C, CE, and SE modes are allowed. See documentation for details.")
  elseif(NOT MSVC AND PTHREADS_EXCEPTION_SCHEME STREQUAL "SE")
    message(FATAL_ERROR "Structured Exception Handling (SE) is only allowed with MSVC compiler.")
  endif()
  message(STATUS "Using exception scheme: ${PTHREADS_EXCEPTION_SCHEME}")
endif()

# Set the final pthread library name based on the exception scheme
set(PTHREADS_LIBS pthreadV${PTHREADS_EXCEPTION_SCHEME}3)
message(STATUS "Building pthread library: ${PTHREADS_LIBS}")
