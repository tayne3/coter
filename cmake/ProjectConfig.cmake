
# 是否编译示例与测试程序
option(COTER_BUILD_EXAMPLE "Build example program" OFF)
option(COTER_BUILD_TEST "Build test program" OFF)
# 是否编译动态库和静态库
option(COTER_BUILD_SHARED "build shared library" ON)
option(COTER_BUILD_STATIC "build static library" OFF)

# 默认编译类型
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# 根据编译类型不同
if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") # 用于调试目的，包含符号表和未优化的代码 (-O0 -g)
	OR DEFINED("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")) # 包含优化的代码和符号表，用于在发生错误时进行调试 (-O2 -g -DNDEBUG)
	set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/debug)
elseif(("${CMAKE_BUILD_TYPE}" STREQUAL "Release") # 用于发布目的，包含优化的代码，并且通常不包含符号表 (-O3 -DNDEBUG)
	OR("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")) # 最小化二进制文件大小，启用优化并删除不必要的符号 (-Os -DNDEBUG)
	set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/release)
else()
	message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}.")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	string(APPEND BIN_DIR "-win32")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	string(APPEND BIN_DIR "-linux")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}) 	# 设置可执行文件的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}) 	# 设置动态库的输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR}) 	# 设置静态库的输出路径
set(CMAKE_INSTALL_PREFIX ${BIN_DIR}) 			# 设置安装路径

set(CMAKE_C_STANDARD 99) 					# 指定C语言标准
set(CMAKE_C_STANDARD_REQUIRED ON) 			# 是否要求使用特定的C语言标准
set(CMAKE_C_EXTENSIONS ON)					# 是否启用C语言编译器扩展功能
set(CMAKE_CXX_STANDARD 11) 					# 指定C++标准版本
set(CMAKE_CXX_STANDARD_REQUIRED ON) 		# 是否要求使用特定的C++标准
set(CMAKE_CXX_EXTENSIONS ON) 				# 是否启用C++编译器扩展功能
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 		# 构建时生成 compile_commands.json
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) 	# 设置导出所有符号 (Windows)

# 导入工具链
include(ToolchainLinux)
# 启用CCache
# include(EnablingCCache)
