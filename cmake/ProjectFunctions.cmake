
# 所有项目宏/函数都应以 "project_" 开头

# 获取目录名称
macro(project_get_dir_name Target)
    get_filename_component(${Target} "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
endmacro()

# 检查头文件是否存在
include(CheckIncludeFiles)
macro(project_check_header header)
    string(TOUPPER ${header} str1)
    string(REGEX REPLACE "[/.]" "_" str2 ${str1})
    set(str3 HAVE_${str2})
    check_include_files(${header} ${str3})
    if (${str3})
        set(${str3} 1)
    else()
        set(${str3} 0)
    endif()
endmacro()

# 检查函数是否存在
include(CheckSymbolExists)
macro(project_check_function function header)
    string(TOUPPER ${function} str1)
    set(str2 HAVE_${str1})
    check_symbol_exists(${function} ${header} ${str2})
    if (${str2})
        set(${str2} 1)
    else()
        set(${str2} 0)
    endif()
endmacro()

# 检查结构体是否存在
include(CheckStructHasMember)
macro(project_check_struct struct member header)
    string(TOUPPER ${struct} str1)
    string(REGEX REPLACE "[ ]" "_" str2 ${str1})
    set(str3 HAVE_${str2})
    check_struct_has_member(${struct} ${member} ${header} ${str3})
    if (${str3})
        set(${str3} 1)
    else()
        set(${str3} 0)
    endif()
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

# 查找源文件
macro(project_list_sources Target)
    unset(tmp)
    file(GLOB_RECURSE tmp *.c *.cpp *.cc *.h *.hpp)
    list(APPEND ${Target} ${tmp})
endmacro()

# 查找源文件 (带依赖)
macro(project_list_sources_with_depends Target)
    unset(tmp)
    file(GLOB_RECURSE tmp CONFIGURE_DEPENDS *.c *.cpp *.cc *.h *.hpp)
    list(APPEND ${Target} ${tmp})
endmacro()
