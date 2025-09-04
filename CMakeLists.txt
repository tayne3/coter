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
if(COTER_BUILD_SHARED)
    add_library(coter SHARED ${project_source_files})
    add_library(coter::shared ALIAS coter)
    message(STATUS "coter ${PROJECT_VERSION} shared library")
else()
    add_library(coter STATIC ${project_source_files})
    add_library(coter::static ALIAS coter)
    message(STATUS "coter ${PROJECT_VERSION} static library")
endif()
add_library(coter::coter ALIAS coter)
target_link_libraries(coter PRIVATE coter_compile_dependency)
target_include_directories(coter PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

if(COTER_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(COTER_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
