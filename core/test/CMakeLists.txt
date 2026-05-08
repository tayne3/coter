add_executable(coter_core_macro_test macro_test.cpp)
add_test(NAME coter_core_macro_test COMMAND coter_core_macro_test)
target_link_libraries(coter_core_macro_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(coter_core_fs_test fs_test.cpp)
add_test(NAME coter_core_fs_test COMMAND coter_core_fs_test)
target_link_libraries(coter_core_fs_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(coter_core_time_test time_test.cpp)
add_test(NAME coter_core_time_test COMMAND coter_core_time_test)
target_link_libraries(coter_core_time_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)
