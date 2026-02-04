cmake_minimum_required(VERSION 3.14)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/PreventInSourceBuilds.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/GitVersion.cmake)
git_version_info(VERSION COTER_VERSION DEFAULT_VERSION 0.8.0)
project(coter VERSION ${COTER_VERSION})

include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectFunctions.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectConfig.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/CPM.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupPthreads.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupLibrarys.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupAtomic.cmake)

set(COTER_LIB_TYPE STATIC)
if(BUILD_SHARED_LIBS)
  set(COTER_LIB_TYPE SHARED)
endif()
message(STATUS "coter v${COTER_VERSION} ${COTER_LIB_TYPE} library")

configure_file(
  "include/coter/core/config.h.in"
  "include/coter/core/config.h"
  @ONLY
)

set(project_source_files
  src/bytes/builder.c
  src/bytes/bytes.c
  src/bytes/seg.c
  src/bytes/pool.c

  src/container/list.c
  src/container/heap.c
  src/container/stack.c
  src/container/packet.c
  src/container/vector.c
  src/container/rbuf.c
  src/container/queue.c
  src/container/hash.c

  src/crypto/sha1.c
  src/crypto/md5.c
  src/crypto/hash/hash.c

  src/encoding/hex.c
  src/encoding/binary.c
  src/encoding/pack.c
  src/encoding/base64.c

  src/event/hub.c
  src/event/msgqueue.c

  src/log/logger.c
  src/log/log_storage.c
  src/log/log_config.c
  src/log/log_printer.c
  src/log/log_callback.c

  src/math/rand.c

  src/strings/strings.c

  src/sync/waitgroup.c
  src/sync/atomic/atomic_mutex.c

  src/thread/jobpool.c
  src/thread/cache.c
  src/thread/thpool.c

  src/time/timer.c
  src/time/time.c
  src/time/cron.c
  src/time/datetime.c

  src/types/any.c
)
add_library(coter ${COTER_LIB_TYPE} ${project_source_files})
add_library(coter::coter ALIAS coter)
target_link_libraries(coter PRIVATE coter_compile_dependency)
target_link_libraries(coter PUBLIC coter_public_dependency)
target_include_directories(coter PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(coter PRIVATE COTER_LIB_EXPORT INTERFACE COTER_SHARED)
endif()

if(COTER_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

if(COTER_BUILD_TEST)
  CPMAddPackage(
    NAME cunit
    URL "https://codeload.github.com/tayne3/cunit/tar.gz/refs/tags/v0.2.5"
    URL_HASH "SHA256=73429bf8bc733ce5431b012689e56f9d255bab3d6d1ac745311f81514b322186"
    OPTIONS "BUILD_SHARED_LIBS OFF"
  )
  enable_testing()
  add_subdirectory(test)
endif()
