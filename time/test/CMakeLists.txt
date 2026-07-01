file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
foreach(SRC ${SRCS_G})
  get_filename_component(TEST_NAME ${SRC} NAME_WE)
  add_executable(coter_time_${TEST_NAME} ${SRC})
  add_test(NAME coter_time_${TEST_NAME} COMMAND coter_time_${TEST_NAME})
  target_link_libraries(coter_time_${TEST_NAME} PRIVATE
    coter_internal_options
    coter::doctest_main
    coter::coter
  )
  target_include_directories(coter_time_${TEST_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
  )
endforeach()
