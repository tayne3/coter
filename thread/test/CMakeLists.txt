file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
foreach(SRC ${SRCS_G})
  get_filename_component(TEST_NAME ${SRC} NAME_WE)
  add_executable(coter_thread_${TEST_NAME} ${SRC})
  add_test(NAME coter_thread_${TEST_NAME} COMMAND coter_thread_${TEST_NAME})
  target_link_libraries(coter_thread_${TEST_NAME} PRIVATE
    coter_internal_options
    coter::doctest_main
    coter::coter
  )
endforeach()
