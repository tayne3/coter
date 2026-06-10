file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
foreach(SRC ${SRCS_G})
  get_filename_component(TEST_NAME ${SRC} NAME_WE)
  add_executable(coter_log_${TEST_NAME} ${SRC})
  add_test(NAME coter_log_${TEST_NAME} COMMAND coter_log_${TEST_NAME})
  target_link_libraries(coter_log_${TEST_NAME} PRIVATE
    coter_compile_dependency
    coter::testing_main
    coter::coter
  )
endforeach()
