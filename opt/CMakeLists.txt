file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")
add_library(coter_opt OBJECT ${SRCS_G})
add_library(coter::opt ALIAS coter_opt)
target_include_directories(coter_opt
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(coter_opt
  PRIVATE
    coter_internal_options
  PUBLIC
    coter::core
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(coter_opt
    PRIVATE
      CT_LIB_EXPORT
    INTERFACE
      CT_SHARED
  )
endif()
target_link_libraries(coter PUBLIC coter::opt)

if(COTER_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(COTER_BUILD_TESTS)
  add_subdirectory(test)
endif()
