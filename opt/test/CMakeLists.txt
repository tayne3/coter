
add_executable(opt_basic_test basic_test.cpp)
add_test(NAME opt_basic_test COMMAND opt_basic_test)
target_link_libraries(opt_basic_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(opt_help_test help_test.cpp)
add_test(NAME opt_help_test COMMAND opt_help_test)
target_link_libraries(opt_help_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(opt_stress_test stress_test.cpp)
add_test(NAME opt_stress_test COMMAND opt_stress_test)
target_link_libraries(opt_stress_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)
