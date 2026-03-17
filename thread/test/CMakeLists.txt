
add_executable(jobpool_test jobpool_test.cpp)
add_test(NAME jobpool_test COMMAND jobpool_test)
target_link_libraries(jobpool_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(thpool_test thpool_test.cpp)
add_test(NAME thpool_test COMMAND thpool_test)
target_link_libraries(thpool_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(thread_test thread_test.cpp)
add_test(NAME thread_test COMMAND thread_test)
target_link_libraries(thread_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)
