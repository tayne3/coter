
add_executable(hash_test hash_test.cpp)
add_test(NAME hash_test COMMAND hash_test)
target_link_libraries(hash_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(heap_test heap_test.cpp)
add_test(NAME heap_test COMMAND heap_test)
target_link_libraries(heap_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(list_test list_test.cpp)
add_test(NAME list_test COMMAND list_test)
target_link_libraries(list_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(packet_test packet_test.cpp)
add_test(NAME packet_test COMMAND packet_test)
target_link_libraries(packet_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(queue_test queue_test.cpp)
add_test(NAME queue_test COMMAND queue_test)
target_link_libraries(queue_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(rbuf_test rbuf_test.cpp)
add_test(NAME rbuf_test COMMAND rbuf_test)
target_link_libraries(rbuf_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(stack_test stack_test.cpp)
add_test(NAME stack_test COMMAND stack_test)
target_link_libraries(stack_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(array_test array_test.cpp)
add_test(NAME array_test COMMAND array_test)
target_link_libraries(array_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(vector_test vector_test.cpp)
add_test(NAME vector_test COMMAND vector_test)
target_link_libraries(vector_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)
