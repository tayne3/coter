add_executable(coter_log_example_basic basic.c)
target_link_libraries(coter_log_example_basic PRIVATE
  coter_compile_dependency
  coter::coter
)

add_executable(coter_log_example_file_rotate file_rotate.c)
target_link_libraries(coter_log_example_file_rotate PRIVATE
  coter_compile_dependency
  coter::coter
)

add_executable(coter_log_example_diy_handler diy_handler.c)
target_link_libraries(coter_log_example_diy_handler PRIVATE
  coter_compile_dependency
  coter::coter
)

add_executable(coter_log_example_multi_type multi_type.c)
target_link_libraries(coter_log_example_multi_type PRIVATE
  coter_compile_dependency
  coter::coter
)

add_executable(coter_log_example_hex_dump hex_dump.c)
target_link_libraries(coter_log_example_hex_dump PRIVATE
  coter_compile_dependency
  coter::coter
)
