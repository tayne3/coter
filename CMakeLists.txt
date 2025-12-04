cmake_minimum_required(VERSION 3.14)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/PreventInSourceBuilds.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/GitVersion.cmake)
git_version_info(VERSION PROJECT_VERSION DEFAULT_VERSION 0.7.0)
project(coter VERSION ${PROJECT_VERSION})

include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectIsTopLevel.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/OptionVariables.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectConfig.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/CPM.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectFunctions.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupPthreads.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupLibrarys.cmake)

if(COTER_BUILD_SHARED)
  set(COTER coter_shared)
  add_library(coter_shared SHARED)
  message(STATUS "coter v${PROJECT_VERSION} shared library")
else()
  set(COTER coter_static)
  add_library(coter_static STATIC)
  message(STATUS "coter v${PROJECT_VERSION} static library")
endif()
add_library(coter::coter ALIAS ${COTER})

configure_file(
  "include/coter/base/config.h.in"
  "include/coter/base/config.h"
  @ONLY
)

file(GLOB_RECURSE project_source_files CONFIGURE_DEPENDS src/*.c)
target_sources(${COTER} PUBLIC ${project_source_files})
target_link_libraries(${COTER} PRIVATE coter_compile_dependency)
target_include_directories(${COTER} PUBLIC 
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
