# 设置 cmake 最低版本
cmake_minimum_required(VERSION 3.5)
# 设置项目属性
project(coter VERSION 0.5.4.3 LANGUAGES C)

# 是否编译示例与测试程序
option(COTER_BUILD_EXAMPLE "build example program" OFF)
option(COTER_BUILD_TEST "build test program" OFF)
# 是否编译动态库和静态库
option(COTER_BUILD_SHARED "build shared library" ON)
option(COTER_BUILD_STATIC "build static library" OFF)

# 导入 cmake 模块
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(ProjectConfig)
include(ProjectFunctions)
include(ToolchainLinux)
include(ToolchainWindows)

# 导入子目录: 源码
add_subdirectory(src)
# 导入子目录: 第三方库
add_subdirectory(3rd)

# 导入子目录: 测试
if(COTER_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

# 导入子目录: 示例
if(COTER_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
