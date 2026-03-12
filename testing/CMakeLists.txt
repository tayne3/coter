add_library(coter_testing INTERFACE)
add_library(coter::testing ALIAS coter_testing)
target_include_directories(coter_testing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

file(WRITE 
  ${CMAKE_CURRENT_BINARY_DIR}/main.cpp 
  "#define CATCH_CONFIG_MAIN\n#include \"catch.hpp\""
)
add_library(coter_testing_main OBJECT ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)
target_include_directories(coter_testing_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(coter::testing_main ALIAS coter_testing_main) 
