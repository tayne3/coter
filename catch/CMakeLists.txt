add_library(coter_catch2 INTERFACE)
add_library(coter::catch2 ALIAS coter_catch2)
target_include_directories(coter_catch2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(coter_catch2_with_main OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_include_directories(coter_catch2_with_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(coter::catch2_with_main ALIAS coter_catch2_with_main) 
