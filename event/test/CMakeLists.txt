
add_executable(hub_test hub_test.cpp)
add_test(NAME hub_test COMMAND hub_test)
target_link_libraries(hub_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)

add_executable(msgqueue_test msgqueue_test.cpp)
add_test(NAME msgqueue_test COMMAND msgqueue_test)
target_link_libraries(msgqueue_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)
