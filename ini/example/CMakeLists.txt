string(TIMESTAMP COTER_BUILD_DATE "%Y-%m-%d")

add_executable(coter_ini_sample sample.c)
target_link_libraries(coter_ini_sample PRIVATE
  coter_internal_options
  coter::ini
)

add_executable(coter_ini_sample_cpp sample_cpp.cpp)
target_link_libraries(coter_ini_sample_cpp PRIVATE
  coter_internal_options
  coter::ini
)

add_executable(coter_ini_inix inix.c)
target_link_libraries(coter_ini_inix PRIVATE
  coter_internal_options
  coter::ini
)
target_compile_definitions(coter_ini_inix PRIVATE
  -DCOTER_COMPILER_ID="${CMAKE_C_COMPILER_ID}"
  -DCOTER_COMPILER_VERSION="${CMAKE_C_COMPILER_VERSION}"
  -DCOTER_VERSION="${PROJECT_VERSION}"
  -DCOTER_BUILD_DATE="${COTER_BUILD_DATE}"
)
