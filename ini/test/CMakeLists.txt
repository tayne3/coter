set(TEST_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
set(TEST_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}")

file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
foreach(SRC ${SRCS_G})
  get_filename_component(TEST_NAME ${SRC} NAME_WE)
  add_executable(coter_ini_${TEST_NAME} ${SRC})
  add_test(NAME coter_ini_${TEST_NAME} COMMAND coter_ini_${TEST_NAME})
  target_link_libraries(coter_ini_${TEST_NAME} PRIVATE
    coter_internal_options
    coter::doctest_main
    coter::coter
  )
  target_compile_definitions(coter_ini_${TEST_NAME} PRIVATE
    TEST_RESOURCE_DIR="${TEST_RESOURCE_DIR}"
    TEST_TEMP_DIR="${TEST_TEMP_DIR}"
  )
endforeach()
