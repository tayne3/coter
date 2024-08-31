
string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME)					# 系统名称
set(SUB_DIR ${CMAKE_SOURCE_DIR}/sub)								# 子项目路径
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)								# 二进制路径
set(OUTPUT_DIR_DEBUG ${BIN_DIR}/debug-${SYSTEM_NAME})				# 调试版本输出路径
set(OUTPUT_DIR_RELEASE ${BIN_DIR}/release-${SYSTEM_NAME})			# 发布版本输出路径

# 默认编译类型
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# 根据编译类型不同
if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") # 用于调试目的，包含符号表和未优化的代码 (-O0 -g)
	OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")) # 包含优化的代码和符号表，用于在发生错误时进行调试 (-O2 -g -DNDEBUG)
	set(OUTPUT_DIR ${OUTPUT_DIR_DEBUG})
    add_definitions(-DDEBUG)
elseif(("${CMAKE_BUILD_TYPE}" STREQUAL "Release") # 用于发布目的，包含优化的代码，并且通常不包含符号表 (-O3 -DNDEBUG)
	OR ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")) # 最小化二进制文件大小，启用优化并删除不必要的符号 (-Os -DNDEBUG)
	set(OUTPUT_DIR ${OUTPUT_DIR_RELEASE})
    add_definitions(-DNDEBUG)
else()
	message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}.")
endif()

# 设置可执行文件输出路径
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endif()

# 设置共享库输出路径
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endif()

# 设置静态库输出路径
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endif()

set(CMAKE_C_STANDARD 99) 											# 指定C语言标准
set(CMAKE_C_STANDARD_REQUIRED ON) 									# 是否要求使用特定的C语言标准
set(CMAKE_C_EXTENSIONS ON)											# 是否启用C语言编译器扩展功能
set(CMAKE_CXX_STANDARD 11) 											# 指定C++标准版本
set(CMAKE_CXX_STANDARD_REQUIRED ON) 								# 是否要求使用特定的C++标准
set(CMAKE_CXX_EXTENSIONS ON) 										# 是否启用C++编译器扩展功能
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 								# 构建时生成 compile_commands.json
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) 							# 设置导出所有符号 (Windows)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)								# 生成位置无关代码

include(GNUInstallDirs)
set(CMAKE_SKIP_BUILD_RPATH OFF)  									# 不跳过设置构建路径的 RPATH
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON) 								# 使用安装路径作为 RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF) 							# 不将链接路径作为 RPATH
set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/${CMAKE_INSTALL_LIBDIR}") 	# 设置 RPATH
