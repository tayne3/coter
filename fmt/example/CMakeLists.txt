add_executable(coter_fmt_sample sample.cpp)
target_link_libraries(coter_fmt_sample PRIVATE
  coter_compile_dependency
  coter::fmt
)
