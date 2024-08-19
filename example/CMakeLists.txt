
add_executable(coter_example main.c app.c dump_win.c dump_unix.c)
target_link_libraries(coter_example coter::coter)
