
add_executable(log_callback_test log_callback_test.c)
add_test(NAME log_callback_test COMMAND log_callback_test)
target_link_libraries(log_callback_test PRIVATE coter_compile_dependency coter::coter cunit::cunit)

add_executable(log_hex_test log_hex_test.c)
add_test(NAME log_hex_test COMMAND log_hex_test)
target_link_libraries(log_hex_test PRIVATE coter_compile_dependency coter::coter cunit::cunit)

add_executable(log_print_test log_print_test.c)
add_test(NAME log_print_test COMMAND log_print_test)
target_link_libraries(log_print_test PRIVATE coter_compile_dependency coter::coter cunit::cunit)

add_executable(log_write_test log_write_test.c)
add_test(NAME log_write_test COMMAND log_write_test)
target_link_libraries(log_write_test PRIVATE coter_compile_dependency coter::coter cunit::cunit)
