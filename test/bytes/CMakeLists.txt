
add_executable(builder_test builder_test.c)
add_test(NAME builder_test COMMAND builder_test)
target_link_libraries(builder_test PRIVATE coter::coter cunit::cunit)

add_executable(bytes_test bytes_test.c)
add_test(NAME bytes_test COMMAND bytes_test)
target_link_libraries(bytes_test PRIVATE coter::coter cunit::cunit)

add_executable(bytes_concurrent_test bytes_concurrent_test.c)
add_test(NAME bytes_concurrent_test COMMAND bytes_concurrent_test)
target_link_libraries(bytes_concurrent_test PRIVATE coter::coter cunit::cunit)

add_executable(pool_test pool_test.c)
add_test(NAME pool_test COMMAND pool_test)
target_link_libraries(pool_test PRIVATE coter::coter cunit::cunit)

add_executable(span_test span_test.c)
add_test(NAME span_test COMMAND span_test)
target_link_libraries(span_test PRIVATE coter::coter cunit::cunit)
