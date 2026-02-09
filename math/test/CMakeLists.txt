
add_executable(rand_test rand_test.cpp)
add_test(NAME rand_test COMMAND rand_test)
target_link_libraries(rand_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)

add_executable(bit_cast_test bit_cast_test.cpp)
add_test(NAME bit_cast_test COMMAND bit_cast_test)
target_link_libraries(bit_cast_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)

add_executable(bit_hpp_test bit_test.cpp)
add_test(NAME bit_hpp_test COMMAND bit_hpp_test)
target_link_libraries(bit_hpp_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)
