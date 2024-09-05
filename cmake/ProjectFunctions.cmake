# 所有项目宏/函数都应以 "project_" 开头

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckLibraryExists)

#[[
# 获取当前目录名称
#
# 该宏获取当前源目录的名称并将其存储在指定变量中。
#
# 参数:
#   Target - 用于存储目录名称的变量名
#]]
macro(project_get_dir_name Target)
    get_filename_component(${Target} "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
endmacro()

#[[
# 查找源文件
#
# 该宏递归查找当前目录下的所有源文件，并将结果添加到指定变量中。
#
# 参数:
#   Target - 用于存储源文件列表的变量名
#]]
macro(project_list_sources Target)
    unset(tmp)
    file(GLOB_RECURSE tmp *.c *.cpp *.cc *.h *.hpp)
    list(APPEND ${Target} ${tmp})
endmacro()

#[[
# 查找源文件（带依赖）
#
# 该宏递归查找当前目录下的所有源文件，并将结果添加到指定变量中。
# 使用 CONFIGURE_DEPENDS 标志以在文件变化时触发重新配置。
#
# 参数:
#   Target - 用于存储源文件列表的变量名
#]]
macro(project_list_sources_with_depends Target)
    unset(tmp)
    file(GLOB_RECURSE tmp CONFIGURE_DEPENDS *.c *.cpp *.cc *.h *.hpp)
    list(APPEND ${Target} ${tmp})
endmacro()

#[[
# 安装目标
#
# 该宏设置目标的安装规则。
#
# 参数:
#   Target - 要安装的目标名称
#]]
macro(project_install_target Target)
    install(TARGETS ${Target}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endmacro()

#[[
# 检查头文件是否存在
#
# 该宏检查指定的头文件是否存在，并设置相应的变量。
#
# 参数:
#   header - 要检查的头文件名
#
# 结果:
#   HAVE_<HEADER>_H - 如果头文件存在则为1，否则为0
#]]
macro(project_check_header header)
    string(TOUPPER ${header} str1)
    string(REGEX REPLACE "[/.]" "_" str2 ${str1})
    set(str3 HAVE_${str2})
    check_include_files("${header}" ${str3})
    if (${str3})
        set(${str3} 1)
    else()
        set(${str3} 0)
    endif()
endmacro()

#[[
# 检查函数是否存在
#
# 该宏检查指定的函数是否存在于给定的头文件中，并设置相应的变量。
#
# 参数:
#   function - 要检查的函数名
#   header - 包含该函数的头文件
#
# 结果:
#   HAVE_<FUNCTION> - 如果函数存在则为1，否则为0
#]]
macro(project_check_function function header)
    string(TOUPPER ${function} str1)
    set(str2 HAVE_${str1})
    check_symbol_exists("${function}" "${header}" ${str2})
    if (${str2})
        set(${str2} 1)
    else()
        set(${str2} 0)
    endif()
endmacro()

#[[
# 检查结构体是否存在并包含指定成员
#
# 该宏检查指定的结构体是否存在于给定的头文件中，并且是否包含指定的成员。
#
# 参数:
#   struct - 要检查的结构体名
#   member - 要检查的结构体成员名
#   header - 包含该结构体的头文件
#
# 结果:
#   HAVE_<STRUCT> - 如果结构体存在且包含指定成员则为1，否则为0
#]]
macro(project_check_struct struct member header)
    string(TOUPPER ${struct} str1)
    string(REGEX REPLACE "[ ]" "_" str2 ${str1})
    set(str3 HAVE_${str2})
    check_struct_has_member("${struct}" "${member}" "${header}" ${str3})
    if (${str3})
        set(${str3} 1)
    else()
        set(${str3} 0)
    endif()
endmacro()

#[[
# 检查库是否存在并包含指定函数
#
# 该宏检查指定的库是否存在，并且是否包含指定的函数。
#
# 参数:
#   library - 要检查的库名
#   function - 要检查的函数名
#   local - 搜索库的路径
#
# 结果:
#   HAVE_<FUNCTION>_IN_<LIBRARY> - 如果库存在且包含指定函数则为1，否则为0
#]]
macro(project_check_library_function library function local)
    string(TOUPPER ${library} str1)
    string(TOUPPER ${function} str2)
    set(str3 HAVE_${str2}_IN_${str1})
    check_library_exists("${library}" "${function}" "${local}" ${str3})
    if (${str3})
        set(${str3} 1)
    else()
        set(${str3} 0)
    endif()
endmacro()
