
# 设置cmake最低版本号
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# 导入配置
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ProjectBasic)

# 设置项目信息
project(${PROJECT_NAME}
    VERSION ${BUILD_VERSION}
    LANGUAGES C
)
include(ProjectConfig)
include(ProjectFunctions)

# 导入子文件夹
add_subdirectory(src)
add_subdirectory(sub)

if(COTER_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(COTER_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

