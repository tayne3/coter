# ===========================================
# Setup Pthreads Library
#
# Configuration:
#   PTHREADS_EXCEPTION_SCHEME: Controls pthread-win32 exception model (MSVC only).
#     "C"  - Console, `SetConsoleCtrlHandler` (Default)
#     "CE" - Console Extended
#     "SE" - GUI, Structured Exception (MSVC only)
# ===========================================

include(CheckLibraryExists)

# ------------------------------------------------------------------------------
# Try to find system-provided pthreads
# ------------------------------------------------------------------------------

# Prefer pthreads if available (e.g., Linux, macOS, MinGW)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads QUIET)
if(CMAKE_USE_PTHREADS_INIT)
  set(PTHREADS_LIBS ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(coter_public_dependency INTERFACE ${PTHREADS_LIBS})
  return()
endif()

# ------------------------------------------------------------------------------
# Windows/MSVC Fallback
# ------------------------------------------------------------------------------
# If we reach here, the system does not provide pthreads (common on MSVC).
# We need to either find an external pthreads library or build one.

# Check for existing pthreads libraries in the library path
check_library_exists(pthreads pthread_create "" CMAKE_HAVE_PTHREADS_CREATE)
if(CMAKE_HAVE_PTHREADS_CREATE)
  set(PTHREADS_LIBS "-lpthreads")
  target_link_libraries(coter_public_dependency INTERFACE ${PTHREADS_LIBS})
  return()
endif()

check_library_exists(pthread pthread_create "" CMAKE_HAVE_PTHREAD_CREATE)
if(CMAKE_HAVE_PTHREAD_CREATE)
  set(PTHREADS_LIBS "-lpthread")
  target_link_libraries(coter_public_dependency INTERFACE ${PTHREADS_LIBS})
  return()
endif()

# ------------------------------------------------------------------------------
# Build pthread-win32 from source
# ------------------------------------------------------------------------------
# No pthreads found. Build pthread-win32 as a dependency.

CPMAddPackage(
  NAME pthread-win32
  GIT_REPOSITORY "https://github.com/GerHobbelt/pthread-win32.git"
  GIT_TAG "06e7608bfe926d2bd7176c0b02be0c98f40cced4"
  GIT_SHALLOW TRUE
  OPTIONS "BUILD_SHARED_LIBS OFF" 
          "BUILD_TESTING OFF"
)

if(NOT DEFINED PTHREADS_EXCEPTION_SCHEME)
  set(PTHREADS_EXCEPTION_SCHEME "C")
endif()
if(NOT PTHREADS_EXCEPTION_SCHEME MATCHES "^(C|CE|SE)$")
  message(FATAL_ERROR "Invalid PTHREADS_EXCEPTION_SCHEME: ${PTHREADS_EXCEPTION_SCHEME}. Allowed: C, CE, SE.")
elseif(PTHREADS_EXCEPTION_SCHEME STREQUAL "SE" AND NOT MSVC)
  message(FATAL_ERROR "PTHREADS_EXCEPTION_SCHEME='SE' is only supported with MSVC.")
endif()

# Select the correct library variant based on the scheme
# VC3 = C, VCE3 = CE, VSE3 = SE
set(PTHREADS_LIBS pthreadV${PTHREADS_EXCEPTION_SCHEME}3)
target_link_libraries(coter_public_dependency INTERFACE ${PTHREADS_LIBS})
message(STATUS "Using pthread-win32 exception scheme: ${PTHREADS_EXCEPTION_SCHEME}")
