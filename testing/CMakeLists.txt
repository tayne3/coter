add_library(coter_testing INTERFACE)
target_include_directories(coter_testing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(coter::testing ALIAS coter_testing)

# doctest Main
file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.cpp
  "#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN\n#include \"coter/testing/doctest.h\""
)
add_library(coter_doctest_main OBJECT ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.cpp)
set_target_properties(coter_doctest_main PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(coter_doctest_main PUBLIC coter_testing)
add_library(coter::doctest_main ALIAS coter_doctest_main)

# nanobench Implementation
file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/nanobench_impl.cpp
  "#define ANKERL_NANOBENCH_IMPLEMENT\n#include \"coter/testing/nanobench.h\""
)
add_library(coter_nanobench OBJECT ${CMAKE_CURRENT_BINARY_DIR}/nanobench_impl.cpp)
set_target_properties(coter_nanobench PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(coter_nanobench PUBLIC coter_testing)
add_library(coter::nanobench ALIAS coter_nanobench)

# fff Implementation
file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/fff_impl.cpp
  "#include \"coter/testing/fff.h\"\nDEFINE_FFF_GLOBALS;"
)
add_library(coter_fff OBJECT ${CMAKE_CURRENT_BINARY_DIR}/fff_impl.cpp)
set_target_properties(coter_fff PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(coter_fff PUBLIC coter_testing)
add_library(coter::fff ALIAS coter_fff)
