add_executable(opt_basic basic.c)
target_link_libraries(opt_basic PRIVATE coter_compile_dependency coter::coter)

add_executable(opt_usage usage.c)
target_link_libraries(opt_usage PRIVATE coter_compile_dependency coter::coter)

add_executable(opt_subcommands subcommands.c)
target_link_libraries(opt_subcommands PRIVATE coter_compile_dependency coter::coter)
