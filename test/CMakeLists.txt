CPMAddPackage(
    NAME ctunit
    GIT_REPOSITORY "http://192.168.1.47:3000/tayne3/ctunit.git"
    GIT_TAG "dca5dc8c6f72d1b17c47cd8ca2827203c52c2af5"
    OPTIONS "CTUNIT_BUILD_SHARED OFF" "CTUNIT_BUILD_STATIC OFF"
)

# 遍历所有源文件
file(GLOB_RECURSE sources_files CONFIGURE_DEPENDS *.c *.cpp *.cc *.h *.hpp)

foreach(test_src ${sources_files})
    # 测试名
    get_filename_component(test_name ${test_src} NAME_WE)
    # 添加可执行文件
    add_executable(${test_name} ${test_src})
    # 设置头文件查找路径
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    # 链接库
    target_link_libraries(${test_name} coter::coter ctunit::ctunit)
    # 添加测试
    add_test(NAME ${test_name} COMMAND ${test_name} -q)
    # 设置测试通过条件
    set_tests_properties(${test_name} PROPERTIES FAIL_REGULAR_EXPRESSION "failed")
endforeach(test_src)
