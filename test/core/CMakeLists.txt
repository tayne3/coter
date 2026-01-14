
add_executable(platform_test platform_test.c)
add_test(NAME platform_test COMMAND platform_test)
target_link_libraries(platform_test PRIVATE coter::coter coter_private_dependency cunit::cunit)
