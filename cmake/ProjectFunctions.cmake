
# 所有项目宏/函数都应以 "project_" 开头

# 获取目录名称
macro(project_get_dir_name Target)
    get_filename_component(${Target} "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
endmacro()

# 收集源文件
macro(project_collect_source_files Target)
    file(GLOB_RECURSE files CONFIGURE_DEPENDS *.c *.cpp *.cc *.h *.hpp)
    foreach(file_path ${files})
        # 转为相对路径
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" file_name ${file_path})
        list(APPEND ${Target} ${file_name})
    endforeach()
endmacro()

# 安装目标
macro(project_install_target Target)
    install(TARGETS ${Target}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endmacro()

# 创建库目标
# Target: 编译目标名
# Type: 库的类型, [STATIC, SHARED, OBJECT]
macro(project_add_library Target Type)
    # 收集当前目录及其子目录中的源文件
    project_collect_source_files(sources_files)
    # 创建库目标
    add_library(${Target} ${Type} ${sources_files})
endmacro()

# 创建可执行文件目标
# Target: 编译目标名
macro(project_add_executable Target)
    # 收集当前目录及其子目录中的源文件
    project_collect_source_files(sources_files)
    # 创建可执行文件目标
    add_executable(${Target} ${sources_files})
endmacro()
