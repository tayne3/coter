file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")
add_library(coter_encoding OBJECT ${SRCS_G})
add_library(coter::encoding ALIAS coter_encoding)
target_include_directories(coter_encoding
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(coter_encoding 
  PRIVATE
    coter_compile_dependency
  PUBLIC
    coter::core
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(coter_encoding
    PRIVATE
      CT_LIB_EXPORT 
    INTERFACE
      CT_SHARED
  )
endif()
target_link_libraries(coter PUBLIC coter::encoding)

if(COTER_BUILD_TEST)
  add_subdirectory(test)
endif()
