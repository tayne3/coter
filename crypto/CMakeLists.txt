file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")
add_library(coter_crypto OBJECT ${SRCS_G})
add_library(coter::crypto ALIAS coter_crypto)
target_include_directories(coter_crypto
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(coter_crypto 
  PRIVATE
    coter_compile_dependency
  PUBLIC
    coter::core
    coter::encoding
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(coter_crypto
    PRIVATE
      COTER_LIB_EXPORT 
    INTERFACE
      COTER_SHARED
  )
endif()
target_link_libraries(coter PUBLIC coter::crypto)

if(COTER_BUILD_TEST)
  add_subdirectory(test)
endif()
