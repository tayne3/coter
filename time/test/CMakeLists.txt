add_library(coter_time_internal_test_deps STATIC)
target_link_libraries(coter_time_internal_test_deps PUBLIC
  coter_internal_options
  coter::core
  coter::container
  coter::thread
  coter::sync
)

add_executable(coter_time_timer_test
  timer_test.cpp
  time_mock.cpp
  ../src/timer.c
)
target_compile_definitions(coter_time_timer_test PRIVATE
  ct_gettimeofday_ms=mock_ct_gettimeofday_ms
  ct_getuptime_ms=mock_ct_getuptime_ms
)
target_include_directories(coter_time_timer_test PRIVATE
  ../include
  ../src
)
target_link_libraries(coter_time_timer_test PRIVATE
  coter_internal_options
  coter_time_internal_test_deps
  coter::doctest_main
  coter::fff
)
add_test(NAME coter_time_timer_test COMMAND coter_time_timer_test)

add_executable(coter_time_cron_test
  cron_test.cpp
  time_mock.cpp
  ../src/cron.c
  ../src/cron_expr.c
  ../src/timer.c
)
target_compile_definitions(coter_time_cron_test PRIVATE
  ct_gettimeofday_ms=mock_ct_gettimeofday_ms
  ct_getuptime_ms=mock_ct_getuptime_ms
)
target_include_directories(coter_time_cron_test PRIVATE
  ../include
  ../src
)
target_link_libraries(coter_time_cron_test PRIVATE
  coter_internal_options
  coter_time_internal_test_deps
  coter::doctest_main
  coter::fff
)
add_test(NAME coter_time_cron_test COMMAND coter_time_cron_test)

add_executable(coter_time_cron_expr_test
  cron_expr_test.cpp
)
target_link_libraries(coter_time_cron_expr_test PRIVATE
  coter_internal_options
  coter_time_internal_test_deps
  coter::doctest_main
  coter::time
)
add_test(NAME coter_time_cron_expr_test COMMAND coter_time_cron_expr_test)
