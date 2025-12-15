
add_executable(hashalgo_test hashalgo_test.c)
add_test(NAME hashalgo_test COMMAND hashalgo_test)
target_link_libraries(hashalgo_test PRIVATE coter::coter cunit::cunit)
