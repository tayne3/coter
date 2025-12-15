cmake_minimum_required(VERSION 3.14)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/PreventInSourceBuilds.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/GitVersion.cmake)
git_version_info(VERSION COTER_VERSION DEFAULT_VERSION 0.8.0)
project(coter VERSION ${COTER_VERSION})

include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectIsTopLevel.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/OptionVariables.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectConfig.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/CPM.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectFunctions.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupPthreads.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupLibrarys.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SetupAtomic.cmake)

if(COTER_BUILD_SHARED)
  add_library(coter_shared SHARED)
  set(COTER coter_shared)
  message(STATUS "coter v${COTER_VERSION} shared library")
else()
  add_library(coter_static STATIC)
  set(COTER coter_static)
  message(STATUS "coter v${COTER_VERSION} static library")
endif()
add_library(coter::coter ALIAS ${COTER})

configure_file(
  "include/coter/core/config.h.in"
  "include/coter/core/config.h"
  @ONLY
)

set(project_source_files
  src/bytes/builder.c
  src/bytes/bytes.c
  src/bytes/span.c
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

  src/encoding/hex.c
  src/encoding/binary.c
  src/encoding/pack.c
  src/encoding/base64.c

  src/event/hub.c
  src/event/msgqueue.c

  src/hash/hash.c

  src/log/logger.c
  src/log/log_storage.c
  src/log/log_config.c
  src/log/log_printer.c
  src/log/log_callback.c

  src/math/rand.c

  src/strings/strings.c

  src/sync/shared_ptr.c
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
target_sources(${COTER} PRIVATE ${project_source_files})
target_link_libraries(${COTER} PUBLIC coter_compile_dependency)
target_include_directories(${COTER} PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/include 
  ${CMAKE_CURRENT_BINARY_DIR}/include   
)

if(COTER_BUILD_TEST)
  CPMAddPackage(
    NAME cunit
    URL "https://codeload.github.com/tayne3/cunit/tar.gz/refs/tags/v0.2.3"
  )
  enable_testing()
  add_subdirectory(test)
endif()

if(COTER_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
