
add_executable(base64_test base64_test.c)
add_test(NAME base64_test COMMAND base64_test)
target_link_libraries(base64_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(binary_test binary_test.c)
add_test(NAME binary_test COMMAND binary_test)
target_link_libraries(binary_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(hex_test hex_test.c)
add_test(NAME hex_test COMMAND hex_test)
target_link_libraries(hex_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(pack_test pack_test.c)
add_test(NAME pack_test COMMAND pack_test)
target_link_libraries(pack_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)
