
add_executable(builder_test builder_test.cpp)
add_test(NAME builder_test COMMAND builder_test)
target_link_libraries(builder_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(bytes_test bytes_test.cpp)
add_test(NAME bytes_test COMMAND bytes_test)
target_link_libraries(bytes_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(bytes_concurrent_test bytes_concurrent_test.cpp)
add_test(NAME bytes_concurrent_test COMMAND bytes_concurrent_test)
target_link_libraries(bytes_concurrent_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(pool_test pool_test.cpp)
add_test(NAME pool_test COMMAND pool_test)
target_link_libraries(pool_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(seg_test seg_test.cpp)
add_test(NAME seg_test COMMAND seg_test)
target_link_libraries(seg_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(seg_hpp_test seg_hpp_test.cpp)
add_test(NAME seg_hpp_test COMMAND seg_hpp_test)
target_link_libraries(seg_hpp_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(segment_test segment_test.cpp)
add_test(NAME segment_test COMMAND segment_test)
target_link_libraries(segment_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)
