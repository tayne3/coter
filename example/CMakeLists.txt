
add_executable(ctex main.c app.c log.c dump_win.c dump_unix.c)
target_link_libraries(ctex coter::coter)
