add_executable(ctex main.c app.c)
target_link_libraries(ctex coter::coter coter_compile_dependency)
