# Benchmark test: do not add ctest, run manually.
file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
foreach(SRC ${SRCS_G})
  get_filename_component(BENCH_NAME ${SRC} NAME_WE)
  add_executable(coter_log_bench_${BENCH_NAME} ${SRC})
  target_link_libraries(coter_log_bench_${BENCH_NAME} PRIVATE
    coter_internal_options
    coter::coter
    coter::nanobench
  )
endforeach()
