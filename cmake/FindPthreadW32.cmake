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

#
# Find the header file
#
find_path(PTHREADS_INCLUDE_DIR
    NAMES pthread.h
    HINTS ${PTHREADS_ROOT}/include
)

if(PTHREADS_INCLUDE_DIR)
    message(STATUS "Found pthread.h: ${PTHREADS_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "Could not find pthread.h. See README.Win32 for more information.")
endif()

#
# Find the library
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

if(MSVC)
    set(PTHREADS_NAMES
        pthreadV${PTHREADS_EXCEPTION_SCHEME}2
        libpthread
    )
elseif(MINGW)
    set(PTHREADS_NAMES
        pthreadG${PTHREADS_EXCEPTION_SCHEME}2
        pthread
    )
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PTHREADS_SUBDIR "/x86")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PTHREADS_SUBDIR "/x64")
endif()

find_library(PTHREADS_LIBRARY
    NAMES ${PTHREADS_NAMES}
    DOC "The Portable Threads Library"
    PATH_SUFFIXES lib/${PTHREADS_SUBDIR}
    HINTS
    ${PTHREADS_ROOT}
    ${CMAKE_SOURCE_DIR}/lib
    $ENV{PTHREADS_LIBRARY_PATH}
    C:/MinGW/lib/
)

if(PTHREADS_LIBRARY)
    message(STATUS "Found PTHREADS library: ${PTHREADS_LIBRARY} (PTHREADS Exception Scheme: ${PTHREADS_EXCEPTION_SCHEME})")
else()
    message(FATAL_ERROR "Could not find PTHREADS LIBRARY. See README.Win32 for more information.")
endif()

get_filename_component(PTHREADS_LIB_NAME ${PTHREADS_LIBRARY} NAME_WE)

#
# Find .dll file
#
find_file(PTHREADS_DLL
    NAMES "${PTHREADS_LIB_NAME}.dll"
    PATH_SUFFIXES dll/${PTHREADS_SUBDIR}
    HINTS
    ${PTHREADS_ROOT}
    ${CMAKE_SOURCE_DIR}/lib
    $ENV{PTHREADS_LIBRARY_PATH}
    C:/MinGW/lib/
)

# 查找包处理标准参数
include(FindPackageHandleStandardArgs) 
find_package_handle_standard_args(PTHREADS DEFAULT_MSG PTHREADS_LIBRARY PTHREADS_INCLUDE_DIR)

if(PTHREADS_FOUND)
    set(PTHREADS_INCLUDE_DIRS ${PTHREADS_INCLUDE_DIR})
    set(PTHREADS_LIBRARIES ${PTHREADS_LIBRARY})

    add_library(pthreads-w32 UNKNOWN IMPORTED)
    set_target_properties(pthreads-w32 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PTHREADS_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${PTHREADS_LIBRARIES}"
        INTERFACE_COMPILE_DEFINITIONS "${PTHREADS_DEFINITIONS}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    )
    set(PTHREADS_LIBS pthreads-w32)

    if(PTHREADS_DLL)
        file(COPY ${PTHREADS_LIBRARIES} ${PTHREADS_DLL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        message(STATUS "Copied pthreads library files: ${PTHREADS_LIBRARIES} and ${PTHREADS_DLL}")
    else()
        file(COPY ${PTHREADS_LIBRARIES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        message(WARNING "Could not find pthreads DLL file. Only .lib file will be copied.")
    endif()

    mark_as_advanced(PTHREADS_INCLUDE_DIR PTHREADS_LIBRARY)
else()
    message(FATAL_ERROR "Could not find PTHREADS LIBRARY. See README.Win32 for more information.")
endif()
