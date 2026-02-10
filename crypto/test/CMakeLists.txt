
add_executable(hashalgo_test hash/hashalgo_test.cpp)
add_test(NAME hashalgo_test COMMAND hashalgo_test)
target_link_libraries(hashalgo_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)

add_executable(md5_test md5_test.cpp)
add_test(NAME md5_test COMMAND md5_test)
target_link_libraries(md5_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)

add_executable(sha1_test sha1_test.cpp)
add_test(NAME sha1_test COMMAND sha1_test)
target_link_libraries(sha1_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)
