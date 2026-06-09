file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
set(DEFAULT_SETUP_TEST "${CMAKE_CURRENT_SOURCE_DIR}/default_setup_test.cpp")
list(REMOVE_ITEM SRCS_G "${DEFAULT_SETUP_TEST}" "default_setup_test.cpp")

add_executable(coter_log_test ${SRCS_G})
add_test(NAME coter_log_test COMMAND coter_log_test)
target_link_libraries(coter_log_test PRIVATE
  coter_compile_dependency
  coter::testing_main
  coter::coter
)

add_executable(coter_log_default_setup_test default_setup_test.cpp)
add_test(NAME coter_log_default_setup_test COMMAND coter_log_default_setup_test)
target_link_libraries(coter_log_default_setup_test PRIVATE
  coter_compile_dependency
  coter::testing_main
  coter::coter
)
