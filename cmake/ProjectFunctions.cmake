
# 所有项目宏/函数都应以 "project_" 开头

# 获取目录名称
# ResultVar: 输出变量
macro(project_get_dir_name ResultVar)
    get_filename_component(${ResultVar} "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
endmacro()

# 收集源文件
# ResultVar: 输出变量
macro(project_collect_source_files ResultVar)
    file(GLOB_RECURSE files CONFIGURE_DEPENDS *.c *.cpp *.cc *.h *.hpp)
    foreach(file_path ${files})
        # 转为相对路径
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" file_name ${file_path})
        list(APPEND ${ResultVar} ${file_name})
    endforeach()
endmacro()

# 创建库目标
# Type: 库的类型, [STATIC, SHARED, OBJECT]
# Target: 编译目标名
function(project_add_library Target Type)
    # 如果Type未指定或为空, 则使用OBJECT
    if(NOT Type OR Type STREQUAL "")
        set(Type OBJECT)
    endif()
    # 如果Target未指定或为空, 则使用当前目录名称
    if(NOT ${Target} OR ${Target} STREQUAL "")
        project_get_dir_name(${Target})
    endif()

    # 收集当前目录及其子目录中的源文件
    project_collect_source_files(sources_files)
    # 创建库目标
    add_library(${${Target}} ${Type} ${sources_files})
    set(${Target} ${${Target}} PARENT_SCOPE)
endfunction(project_add_library)

# 创建可执行文件目标
# Target: 编译目标名
function(project_add_executable Target)
    # 如果Target未指定或为空, 则使用当前目录名称
    if(NOT ${Target} OR ${Target} STREQUAL "")
        project_get_dir_name(${Target})
    endif()
    # 如果Target未指定或为空, 则使用当前目录名称
    if(NOT ${Target} OR ${Target} STREQUAL "")
        project_get_dir_name(${Target})
    endif()

    # 收集当前目录及其子目录中的源文件
    project_collect_source_files(sources_files)
    # 创建可执行文件目标
    add_executable(${${Target}} ${sources_files})
    set(${Target} ${${Target}} PARENT_SCOPE)
endfunction(project_add_executable)

# 安装目标
function(project_install_target Target)
    install(TARGETS ${Target}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endfunction(project_install_target)
