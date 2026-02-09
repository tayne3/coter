file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")
add_library(coter_thread OBJECT ${SRCS_G})
add_library(coter::thread ALIAS coter_thread)
target_include_directories(coter_thread
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(coter_thread 
  PRIVATE
    coter_compile_dependency
  PUBLIC
    coter::core
    coter::container
    coter::event
    coter::sync
)
target_link_libraries(coter PUBLIC coter::thread)

if(COTER_BUILD_TEST)
  add_subdirectory(test)
endif()
