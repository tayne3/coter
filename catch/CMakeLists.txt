add_library(Catch2 INTERFACE)
add_library(Catch2::Catch2 ALIAS Catch2)
add_library(coter::catch2 ALIAS Catch2)
target_include_directories(Catch2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Catch2WithMain OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_include_directories(Catch2WithMain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(coter::catch2_with_main ALIAS Catch2WithMain) 
