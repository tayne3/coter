
# 设置程序名
set(TARGET_NAME coter_obj)
# 创建编译目标
project_add_library(TARGET_NAME OBJECT)
# 添加别名
add_library(coter::obj ALIAS coter_obj)
# 设置头文件查找路径
target_include_directories(coter_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# 链接库文件
target_link_libraries(coter_obj PUBLIC pthread rt)

# 根据编译类型不同
if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") # 用于调试目的，包含符号表和未优化的代码 (-O0 -g)
	OR DEFINED("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")) # 包含优化的代码和符号表，用于在发生错误时进行调试 (-O2 -g -DNDEBUG)
    set(BUILD_VERSION_DEBUG 1)
elseif(("${CMAKE_BUILD_TYPE}" STREQUAL "Release") # 用于发布目的，包含优化的代码，并且通常不包含符号表 (-O3 -DNDEBUG)
	OR ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")) # 最小化二进制文件大小，启用优化并删除不必要的符号 (-Os -DNDEBUG)
    set(BUILD_VERSION_DEBUG 0)
else()
	message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}.")
endif()
# 生成调试标志头文件
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/base/ct_debug.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/base/ct_debug.h" @ONLY)

# 编译动态库
if(COTER_BUILD_SHARED)
    add_library(coter_shared SHARED $<TARGET_OBJECTS:coter_obj>)
    add_library(coter::shared ALIAS coter_shared)
    target_include_directories(coter_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(coter_shared PROPERTIES OUTPUT_NAME coter)
    project_install_target(coter_shared)
    message(STATUS "Coter build shared library: ON")
else()
    message(STATUS "Coter build shared library: OFF")
endif()

# 编译静态库
if(COTER_BUILD_STATIC)
    add_library(coter_static STATIC $<TARGET_OBJECTS:coter_obj>)
    add_library(coter::static ALIAS coter_static)
    target_include_directories(coter_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(coter_static PROPERTIES OUTPUT_NAME coter)
    project_install_target(coter_static)
    message(STATUS "Coter build static library: ON")
else()
    message(STATUS "Coter build static library: OFF")
endif()
