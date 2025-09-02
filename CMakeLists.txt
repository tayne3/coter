cmake_minimum_required(VERSION 3.14)

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/GitVersion.cmake)
git_version_info(VERSION PROJECT_VERSION DEFAULT_VERSION 0.7.0)
project(coter VERSION ${PROJECT_VERSION})

include(cmake/ProjectIsTopLevel.cmake)
include(cmake/OptionVariables.cmake)
include(cmake/ProjectConfig.cmake)
include(cmake/CPM.cmake)
include(cmake/ProjectFunctions.cmake)
include(cmake/SetupPthreads.cmake)
include(cmake/SetupLibrarys.cmake)

configure_file(
    "include/coter/base/config.h.in"
    "include/coter/base/config.h"
    @ONLY
)

file(GLOB_RECURSE project_source_files CONFIGURE_DEPENDS src/*.c)
add_library(${PROJECT_NAME} ${PROJECT_LIB_TYPE} ${project_source_files})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_compile_dependency)
target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_LIB_NAME} ALIAS ${PROJECT_NAME})
message(STATUS "${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_LIB_NAME} library")

if(COTER_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(COTER_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
