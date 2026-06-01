string(TIMESTAMP COTER_BUILD_DATE "%Y-%m-%d")

add_executable(ini_sample sample.c)
target_link_libraries(ini_sample PRIVATE
  coter_compile_dependency
  coter::ini
)

add_executable(ini_sample_cpp sample_cpp.cpp)
target_link_libraries(ini_sample_cpp PRIVATE
  coter_compile_dependency
  coter::ini
)

add_executable(ini_inix inix.c)
target_link_libraries(ini_inix PRIVATE
  coter_compile_dependency
  coter::ini
)
target_compile_definitions(ini_inix PRIVATE
  -DCOTER_COMPILER_ID="${CMAKE_C_COMPILER_ID}"
  -DCOTER_COMPILER_VERSION="${CMAKE_C_COMPILER_VERSION}"
  -DCOTER_VERSION="${PROJECT_VERSION}"
  -DCOTER_BUILD_DATE="${COTER_BUILD_DATE}"
)
