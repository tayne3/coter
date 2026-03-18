add_executable(cron_expr_test cron_expr_test.cpp)
add_test(NAME cron_expr_test COMMAND cron_expr_test)
target_link_libraries(cron_expr_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(cron_test cron_test.cpp)
add_test(NAME cron_test COMMAND cron_test)
target_link_libraries(cron_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)

add_executable(timer_test timer_test.cpp)
add_test(NAME timer_test COMMAND timer_test)
target_link_libraries(timer_test PRIVATE coter_compile_dependency coter::coter coter::testing_main)
target_include_directories(timer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
