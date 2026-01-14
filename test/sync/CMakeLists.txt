
add_executable(atomic_stress_test atomic_stress_test.c)
add_test(NAME atomic_stress_test COMMAND atomic_stress_test)
target_link_libraries(atomic_stress_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(atomic_test atomic_test.c)
add_test(NAME atomic_test COMMAND atomic_test)
target_link_libraries(atomic_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)

add_executable(waitgroup_test waitgroup_test.c)
add_test(NAME waitgroup_test COMMAND waitgroup_test)
target_link_libraries(waitgroup_test PRIVATE coter::coter coter_compile_dependency cunit::cunit)
