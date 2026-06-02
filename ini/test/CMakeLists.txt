file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
add_executable(coter_ini_test ${SRCS_G})
add_test(NAME coter_ini_test COMMAND coter_ini_test)
target_link_libraries(coter_ini_test PRIVATE
  coter_compile_dependency
  coter::testing_main
  coter::coter
)

set(TEST_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
set(TEST_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(coter_ini_test PRIVATE 
  TEST_RESOURCE_DIR="${TEST_RESOURCE_DIR}"
  TEST_TEMP_DIR="${TEST_TEMP_DIR}"
)
