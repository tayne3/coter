
if(COTER_BUILD_DEBUG)
    add_definitions(-DDEBUG)
else()
	add_definitions(-DNDEBUG)
endif()

# 设置编译选项
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_options(
        -Wall                               # 启用常见警告
        -Wextra                             # 启用额外警告
        -Wstrict-aliasing                   # 增强类型别名检查
        -Wstrict-prototypes                 # 检查函数原型
        -Wshadow                            # 检查变量遮蔽 
        -Wundef                             # 检查未定义宏 
        -Wunused                            # 检查未使用的变量、函数等
        -Wunused-parameter                  # 检查未使用的函数参数
        -Wmissing-declarations              # 检查缺失声明
        -Wmissing-prototypes                # 检查缺少函数原型声明
        -Wformat                            # 检查格式化字符串的正确性
        -Wpointer-arith                     # 检查指针运算的类型
        -Wuninitialized                     # 检查未初始化的变量使用
        -fPIC                               # 生成位置无关代码 (编译动态库)
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC)
        add_compile_options(
            /source-charset:utf-8               # 设置源代码文件的字符集编码为UTF-8
        )
    else()
        add_compile_options(
            -Wall                               # 启用常见警告
            -Wextra                             # 启用额外警告
            -Wstrict-aliasing                   # 增强类型别名检查
            -Wstrict-prototypes                 # 检查函数原型
            -Wshadow                            # 检查变量遮蔽 
            -Wundef                             # 检查未定义宏 
            -Wunused                            # 检查未使用的变量、函数等
            -Wunused-parameter                  # 检查未使用的函数参数
            -Wmissing-declarations              # 检查缺失声明
            -Wmissing-prototypes                # 检查缺少函数原型声明
            -Wformat                            # 检查格式化字符串的正确性
            -Wpointer-arith                     # 检查指针运算的类型
            -Wuninitialized                     # 检查未初始化的变量使用
            -fPIC                               # 生成位置无关代码 (编译动态库)
        )
    endif()
endif()
