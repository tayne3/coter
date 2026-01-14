add_executable(ctex main.c app.c)
target_link_libraries(ctex coter::coter)
target_compile_options(ctex PRIVATE 
  "$<$<COMPILE_LANGUAGE:C>:/utf-8>"
)
