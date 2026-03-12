
add_executable(platform_test platform_test.cpp)
add_test(NAME platform_test COMMAND platform_test)
target_link_libraries(platform_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)
