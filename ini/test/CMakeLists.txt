set(TEST_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
set(TEST_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}")

function(ct_ini_add_test name)
  add_executable(${name} ${ARGN})
  add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${name} PRIVATE
    coter_compile_dependency
    coter::ini 
  )
  target_compile_definitions(${name} PRIVATE 
    TEST_RESOURCE_DIR="${TEST_RESOURCE_DIR}"
    TEST_TEMP_DIR="${TEST_TEMP_DIR}"
  )
endfunction()

ct_ini_add_test(test_lifecycle test_lifecycle.cpp)
ct_ini_add_test(test_section_api test_section_api.cpp)
ct_ini_add_test(test_key_api test_key_api.cpp)
ct_ini_add_test(test_value_getter test_value_getter.cpp)
ct_ini_add_test(test_parser test_parser.cpp)
ct_ini_add_test(test_serializer test_serializer.cpp)
ct_ini_add_test(test_edge_cases test_edge_cases.cpp)
ct_ini_add_test(test_error_handling test_error_handling.cpp)
ct_ini_add_test(test_iterator test_iterator.cpp)
ct_ini_add_test(test_cpp test_cpp.cpp)
