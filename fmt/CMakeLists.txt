add_library(coter_fmt INTERFACE)
add_library(coter::fmt ALIAS coter_fmt)
target_include_directories(coter_fmt INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(coter_fmt INTERFACE
  coter::core
)
target_link_libraries(coter PUBLIC coter::fmt)

if(COTER_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(COTER_BUILD_TESTS)
  add_subdirectory(test)
endif()
