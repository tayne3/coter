# ==============================================================================
# 查找 Pthreads 库
#
# 外部需要配置变量 PTHREADS_ROOT 为指向 Pthreads-w32 安装的根目录。
#
# 此模块定义了以下变量：
#
# PTHREADS_FOUND       - 如果找到 Pthreads 库，则为 True
# PTHREADS_LIBRARY     - Pthreads 库的位置
# PTHREADS_INCLUDE_DIR - Pthreads 库的包含目录
# PTHREADS_DEFINITIONS - 预处理器定义（HAVE_PTHREAD_H 是一个相当常见的定义）
#
# ==============================================================================

include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CMakePushCheckState)

if(NOT WIN32)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)

    if(CMAKE_USE_PTHREADS_INIT)
        add_library(pthread INTERFACE)
        target_link_libraries(pthread INTERFACE ${CMAKE_THREAD_LIBS_INIT})
        add_library(pthread::pthread ALIAS pthread)
        message(STATUS "CMAKE_USE_PTHREADS_INIT: ${CMAKE_USE_PTHREADS_INIT}")
    else()
        message(FATAL_ERROR "Could not find pthread library. See README.Win32 for more information.")
    endif()
    return()
endif()

# 是否原生支持pthread
include(CheckLibraryExists)
check_library_exists(pthread pthread_rwlock_init "" HAVE_PTHREAD)
if(HAVE_PTHREAD)
    message(STATUS "HAVE_PTHREAD: ${HAVE_PTHREAD}")
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

# Find .dll file
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

# message(STATUS "PTHREADS_FOUND: ${PTHREADS_FOUND}")
if(PTHREADS_FOUND)
    set(PTHREADS_INCLUDE_DIRS ${PTHREADS_INCLUDE_DIR})
    set(PTHREADS_LIBRARIES ${PTHREADS_LIBRARY})
    set(PTHREADS_DEFINITIONS -DHAVE_PTHREADS_H)

    # Avoid redefinition of struct timespec
    if(MSVC)
        include(CheckStructHasMember)
        check_struct_has_member("struct timespec" tv_sec time.h HAVE_STRUCT_TIMESPEC LANGUAGE C)

        if(HAVE_STRUCT_TIMESPEC)
            set(PTHREADS_DEFINITIONS "${PTHREADS_DEFINITIONS} -DHAVE_STRUCT_TIMESPEC")
        endif()
    endif()

    add_library(pthreadsw32 UNKNOWN IMPORTED)
    add_library(pthread::pthread ALIAS pthreadsw32)
    set_target_properties(pthreadsw32 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PTHREADS_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${PTHREADS_LIBRARIES}"
        INTERFACE_COMPILE_DEFINITIONS "${PTHREADS_DEFINITIONS}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    )

    if(PTHREADS_DLL)
        file(COPY ${PTHREADS_LIBRARIES} ${PTHREADS_DLL} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        message(STATUS "Copied pthreads library files: ${PTHREADS_LIBRARIES} and ${PTHREADS_DLL}")
    else()
        file(COPY ${PTHREADS_LIBRARIES} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        message(WARNING "Could not find pthreads DLL file. Only .lib file will be copied.")
    endif()

    # 标记为高级变量
    mark_as_advanced(PTHREADS_INCLUDE_DIR PTHREADS_LIBRARY)
else()
    message(FATAL_ERROR "Could not find PTHREADS LIBRARY. See README.Win32 for more information.")
endif()
