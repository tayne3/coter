
add_executable(strings_test strings_test.c)
add_test(NAME strings_test COMMAND strings_test)
target_link_libraries(strings_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)
