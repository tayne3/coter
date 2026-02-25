
add_executable(cron_next_test cron_next_test.cpp)
add_test(NAME cron_next_test COMMAND cron_next_test)
target_link_libraries(cron_next_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(cron_test cron_test.cpp)
add_test(NAME cron_test COMMAND cron_test)
target_link_libraries(cron_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)

add_executable(timer_test timer_test.cpp)
add_test(NAME timer_test COMMAND timer_test)
target_link_libraries(timer_test PRIVATE coter_compile_dependency coter::coter coter::catch2_with_main)
