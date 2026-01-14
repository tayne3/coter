
add_executable(md5_test md5_test.c)
add_test(NAME md5_test COMMAND md5_test)
target_link_libraries(md5_test PRIVATE coter::coter coter_private_dependency cunit::cunit)

add_executable(sha1_test sha1_test.c)
add_test(NAME sha1_test COMMAND sha1_test)
target_link_libraries(sha1_test PRIVATE coter::coter coter_private_dependency cunit::cunit)
