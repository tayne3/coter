
add_executable(cron_next_test cron_next_test.c)
add_test(NAME cron_next_test COMMAND cron_next_test)
target_link_libraries(cron_next_test PRIVATE coter::coter cunit::cunit)

add_executable(cron_test cron_test.c)
add_test(NAME cron_test COMMAND cron_test)
target_link_libraries(cron_test PRIVATE coter::coter cunit::cunit)

add_executable(time_test time_test.c)
add_test(NAME time_test COMMAND time_test)
target_link_libraries(time_test PRIVATE coter::coter cunit::cunit)

add_executable(timer_test timer_test.c)
add_test(NAME timer_test COMMAND timer_test)
target_link_libraries(timer_test PRIVATE coter::coter cunit::cunit)
