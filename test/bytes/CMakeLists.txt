
add_executable(builder_test builder_test.c)
add_test(NAME builder_test COMMAND builder_test)
target_link_libraries(builder_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(bytes_test bytes_test.c)
add_test(NAME bytes_test COMMAND bytes_test)
target_link_libraries(bytes_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(bytes_concurrent_test bytes_concurrent_test.c)
add_test(NAME bytes_concurrent_test COMMAND bytes_concurrent_test)
target_link_libraries(bytes_concurrent_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(pool_test pool_test.c)
add_test(NAME pool_test COMMAND pool_test)
target_link_libraries(pool_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(seg_test seg_test.cpp ${MAIN_CPP})
add_test(NAME seg_test COMMAND seg_test)
target_link_libraries(seg_test PRIVATE Catch2 coter::coter)

add_executable(seg_hpp_test seg_test.cpp ${MAIN_CPP})
add_test(NAME seg_hpp_test COMMAND seg_hpp_test)
target_link_libraries(seg_hpp_test PRIVATE Catch2 coter::coter)
