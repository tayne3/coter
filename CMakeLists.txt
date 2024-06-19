
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(coter VERSION 0.1.2.3 LANGUAGES C)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ProjectConfig)
include(ProjectFunctions)
include(ToolchainLinux)
include(EnablingCCache)

add_subdirectory(src)
add_subdirectory(sub)

if(COTER_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(COTER_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
