
# 遍历所有源文件
project_list_sources(sources_files)

# 遍历
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
    set_tests_properties(${test_name} PROPERTIES FAIL_REGULAR_EXPRESSION "failed|error")
endforeach(test_src)
