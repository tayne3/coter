
add_executable(atomic_stress_test atomic_stress_test.cpp)
add_test(NAME atomic_stress_test COMMAND atomic_stress_test)
target_link_libraries(atomic_stress_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)

add_executable(atomic_test atomic_test.cpp)
add_test(NAME atomic_test COMMAND atomic_test)
target_link_libraries(atomic_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)

add_executable(waitgroup_test waitgroup_test.cpp)
add_test(NAME waitgroup_test COMMAND waitgroup_test)
target_link_libraries(waitgroup_test PRIVATE coter_compile_dependency coter::coter Catch2WithMain)
