add_executable(coter_opt_basic basic.c)
target_link_libraries(coter_opt_basic PRIVATE
  coter_compile_dependency
  coter::coter
)

add_executable(coter_opt_usage usage.c)
target_link_libraries(coter_opt_usage PRIVATE
  coter_compile_dependency
  coter::coter
)

add_executable(coter_opt_subcommands subcommands.c)
target_link_libraries(coter_opt_subcommands PRIVATE
  coter_compile_dependency
  coter::coter
)
