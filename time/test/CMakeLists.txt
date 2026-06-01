file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
add_executable(coter_time_test ${SRCS_G})
add_test(NAME coter_time_test COMMAND coter_time_test)
target_link_libraries(coter_time_test PRIVATE
  coter_compile_dependency
  coter::testing_main
  coter::coter
)
target_include_directories(coter_time_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
