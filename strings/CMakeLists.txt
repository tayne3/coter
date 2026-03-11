file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")
add_library(coter_strings OBJECT ${SRCS_G})
add_library(coter::strings ALIAS coter_strings)
target_include_directories(coter_strings
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(coter_strings 
  PRIVATE
    coter_compile_dependency
  PUBLIC
    coter::core
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(coter_strings
    PRIVATE
      COTER_LIB_EXPORT 
    INTERFACE
      COTER_SHARED
  )
endif()
target_link_libraries(coter PUBLIC coter::strings)

if(MINGW)
  target_compile_definitions(coter_strings PRIVATE __USE_MINGW_ANSI_STDIO=1)
endif()

if(COTER_BUILD_TEST)
  add_subdirectory(test)
endif()
