
add_executable(hash_test hash_test.c)
add_test(NAME hash_test COMMAND hash_test)
target_link_libraries(hash_test PRIVATE coter::coter cunit::cunit)

add_executable(heap_test heap_test.c)
add_test(NAME heap_test COMMAND heap_test)
target_link_libraries(heap_test PRIVATE coter::coter cunit::cunit)

add_executable(list_test list_test.c)
add_test(NAME list_test COMMAND list_test)
target_link_libraries(list_test PRIVATE coter::coter cunit::cunit)

add_executable(packet_test packet_test.c)
add_test(NAME packet_test COMMAND packet_test)
target_link_libraries(packet_test PRIVATE coter::coter cunit::cunit)

add_executable(queue_test queue_test.c)
add_test(NAME queue_test COMMAND queue_test)
target_link_libraries(queue_test PRIVATE coter::coter cunit::cunit)

add_executable(rbuf_test rbuf_test.c)
add_test(NAME rbuf_test COMMAND rbuf_test)
target_link_libraries(rbuf_test PRIVATE coter::coter cunit::cunit)

add_executable(stack_test stack_test.c)
add_test(NAME stack_test COMMAND stack_test)
target_link_libraries(stack_test PRIVATE coter::coter cunit::cunit)

add_executable(vector_define_test vector_define_test.c)
add_test(NAME vector_define_test COMMAND vector_define_test)
target_link_libraries(vector_define_test PRIVATE coter::coter cunit::cunit)

add_executable(vector_test vector_test.c)
add_test(NAME vector_test COMMAND vector_test)
target_link_libraries(vector_test PRIVATE coter::coter cunit::cunit)

