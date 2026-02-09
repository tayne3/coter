configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/coter/core/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/coter/core/config.h"
  @ONLY
)

file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")
add_library(coter_core OBJECT ${SRCS_G})
add_library(coter::core ALIAS coter_core)
target_include_directories(coter_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(coter_core
  PRIVATE
    coter_compile_dependency
  PUBLIC
    coter_public_dependency
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(coter_core
    PRIVATE
      COTER_LIB_EXPORT 
    INTERFACE
      COTER_SHARED
  )
endif()
target_link_libraries(coter PUBLIC coter::core)

if(COTER_BUILD_TEST)
  add_subdirectory(test)
endif()
