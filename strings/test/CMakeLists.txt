
add_executable(strings_test strings_test.cpp)
add_test(NAME strings_test COMMAND strings_test)
target_link_libraries(strings_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)
