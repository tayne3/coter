CPMAddPackage(
  NAME cunit
  GIT_REPOSITORY "http://192.168.1.47:3000/tayne3/cunit.git"
  VERSION 0.2.3
  GIT_SHALLOW TRUE
)

file(GLOB_RECURSE test_source_files CONFIGURE_DEPENDS *.c)
foreach(test_src ${test_source_files})
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${test_name} PRIVATE coter::coter cunit::cunit)
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
