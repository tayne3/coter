cmake_minimum_required(VERSION 3.14)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported. "
    "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' before rebuilding this project."
  )
endif()

project(coter VERSION 0.13.3)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/coter_compiler_options.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/coter_platform_config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/coter_atomic_checks.cmake)

message(STATUS "coter v${PROJECT_VERSION} (BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS})")
add_library(coter)
add_library(coter::coter ALIAS coter)
target_link_libraries(coter PRIVATE coter_internal_options)

if(COTER_BUILD_TESTS)
  enable_testing()
endif()
add_subdirectory(testing)

add_subdirectory(bytes)
add_subdirectory(container)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(encoding)
add_subdirectory(opt)
add_subdirectory(sync)
add_subdirectory(thread)
