add_library(coter_compile_dependency INTERFACE)
target_compile_features(coter_compile_dependency INTERFACE c_std_99)

if(NOT DEFINED ENV{CPM_SOURCE_CACHE})
	set(ENV{CPM_SOURCE_CACHE} ${CMAKE_SOURCE_DIR}/cmake_modules)
endif()
string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME)
if(COTER_BUILD_DEBUG)
	set(COTER_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin/debug-${SYSTEM_NAME})
	target_compile_definitions(coter_compile_dependency INTERFACE -DDEBUG)
else()
	set(COTER_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin/release-${SYSTEM_NAME})
	target_compile_definitions(coter_compile_dependency INTERFACE -DNDEBUG)
endif()

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${COTER_OUTPUT_DIR})					
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${COTER_OUTPUT_DIR})					
endif()
# if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
# 	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${COTER_OUTPUT_DIR})					
# endif()

include(GNUInstallDirs)
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@loader_path;@loader_path/${CMAKE_INSTALL_LIBDIR}")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/${CMAKE_INSTALL_LIBDIR}")
endif()

if(PROJECT_IS_TOP_LEVEL)
	find_program(CCACHE_PROGRAM ccache)
	if(CCACHE_PROGRAM)
		set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
		set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	endif()
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 为MSVC编译器设置特定的C语言选项
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(coter_compile_dependency INTERFACE 
		"$<$<COMPILE_LANGUAGE:C>:/source-charset:utf-8>"
	)
endif()

# 为非MSVC和非ClangCL（MSVC前端）的编译器设置C语言警告选项
# 这些选项有助于捕捉潜在的代码问题
if(NOT (CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")))
    target_compile_options(coter_compile_dependency INTERFACE
        "$<$<COMPILE_LANGUAGE:C>:-Wall>"
        "$<$<COMPILE_LANGUAGE:C>:-Wextra>"
        "$<$<COMPILE_LANGUAGE:C>:-Werror=return-type>"
    )
endif()

# 处理旧版本GCC（低于6.0）的特定警告抑制选项
# -Wno-missing-field-initializers 用于抑制因结构体未完全初始化而产生的警告
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 6.0)
    target_compile_options(coter_compile_dependency INTERFACE 
		"$<$<COMPILE_LANGUAGE:C>:-Wno-missing-field-initializers>"
	)
endif()
