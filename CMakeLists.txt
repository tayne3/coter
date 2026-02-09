cmake_minimum_required(VERSION 3.14)

# In-source build guard
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
    FATAL_ERROR
    "In-source builds are not supported. "
    "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' before rebuilding this project."
  )
endif()

project(coter VERSION 0.9.0)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectFunctions.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectConfig.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/CPM.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupPthreads.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupLibrarys.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupAtomic.cmake)

if(BUILD_SHARED_LIBS)
  set(COTER_LIB_TYPE SHARED)
else()
  set(COTER_LIB_TYPE STATIC)
endif()
message(STATUS "coter v${PROJECT_VERSION} ${COTER_LIB_TYPE} library")
add_library(coter ${COTER_LIB_TYPE})
add_library(coter::coter ALIAS coter)
target_link_libraries(coter PRIVATE coter_compile_dependency)

if(COTER_BUILD_TEST)
  enable_testing()
  CPMAddPackage(
    NAME cunit
    URL "https://codeload.github.com/tayne3/cunit/tar.gz/refs/tags/v0.2.7"
    URL_HASH "SHA256=5ede3cb48cb7e59cef32b4e31cc258a510751b81906d85756d71401b1772b9fc"
    OPTIONS "BUILD_SHARED_LIBS OFF"
  )
endif()
add_subdirectory(catch)

add_subdirectory(bytes)
add_subdirectory(container)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(encoding)
add_subdirectory(event)
add_subdirectory(log)
add_subdirectory(math)
add_subdirectory(strings)
add_subdirectory(sync)
add_subdirectory(thread)
add_subdirectory(time)
add_subdirectory(types)
