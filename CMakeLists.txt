cmake_minimum_required(VERSION 3.14)

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/GitVersion.cmake)
gitversion_extract(VERSION PROJECT_VERSION DEFAULT_VERSION 0.6.6)
project(coter VERSION ${PROJECT_VERSION})

include(cmake/ProjectIsTopLevel.cmake)
include(cmake/OptionVariables.cmake)
include(cmake/ProjectConfig.cmake)
include(cmake/CPM.cmake)
include(cmake/CompileOptions.cmake)
include(cmake/ProjectFunctions.cmake)
include(cmake/SetupPthreads.cmake)
include(cmake/SetupLibrarys.cmake)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/base/prefix/ct_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/base/prefix/ct_config.h"
    @ONLY
)

file(GLOB_RECURSE coter_source_files CONFIGURE_DEPENDS src/*.c src/*.cpp src/*.cc src/*.h src/*.hpp)
add_library(coter ${COTER_LIB_TYPE} ${coter_source_files})
target_include_directories(coter PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(coter PUBLIC ${COTER_LIBS})
target_compile_definitions(coter PRIVATE ${COTER_COMPILE_DEFINITIONS})
target_compile_features(coter PRIVATE c_std_99 cxx_std_11)

add_library(coter::coter ALIAS coter)
if(COTER_BUILD_SHARED)
    add_library(coter::shared ALIAS coter)
    message(STATUS "Coter ${PROJECT_VERSION} shared library")
else()
    add_library(coter::static ALIAS coter)
    message(STATUS "Coter ${PROJECT_VERSION} static library")
endif()

if(COTER_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(COTER_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
