
add_executable(rand_test rand_test.c)
add_test(NAME rand_test COMMAND rand_test)
target_link_libraries(rand_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)
