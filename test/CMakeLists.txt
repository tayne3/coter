

# 遍历所有源文件
file(GLOB_RECURSE FILES CONFIGURE_DEPENDS *.c)
foreach(filepath ${FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND sources_files ${filename})
endforeach(filepath)

# 遍历
foreach(test_src ${sources_files})
    # 测试名
    get_filename_component(test_name ${test_src} NAME_WLE)
    # 添加可执行文件
    add_executable(${test_name} ${test_src})
    # 设置头文件查找路径
    target_include_directories(${test_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    # 链接库
    target_link_libraries(${test_name} coter::obj ctunit::obj)
    # 添加测试
    add_test(NAME ${test_name} COMMAND ${test_name})
    # 设置测试通过条件
    set_tests_properties(${test_name} PROPERTIES FAIL_REGULAR_EXPRESSION "failed|error")
endforeach(test_src)
