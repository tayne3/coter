
add_executable(atomic_stress_test atomic_stress_test.cpp)
add_test(NAME atomic_stress_test COMMAND atomic_stress_test)
target_link_libraries(atomic_stress_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(atomic_test atomic_test.cpp)
add_test(NAME atomic_test COMMAND atomic_test)
target_link_libraries(atomic_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(waitgroup_test waitgroup_test.cpp)
add_test(NAME waitgroup_test COMMAND waitgroup_test)
target_link_libraries(waitgroup_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(mutex_cond_test mutex_cond_test.cpp)
add_test(NAME mutex_cond_test COMMAND mutex_cond_test)
target_link_libraries(mutex_cond_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(rwlock_test rwlock_test.cpp)
add_test(NAME rwlock_test COMMAND rwlock_test)
target_link_libraries(rwlock_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(event_test event_test.cpp)
add_test(NAME event_test COMMAND event_test)
target_link_libraries(event_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(sem_test sem_test.cpp)
add_test(NAME sem_test COMMAND sem_test)
target_link_libraries(sem_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)
