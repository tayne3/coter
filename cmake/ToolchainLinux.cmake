
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    return()
endif()

# 设置编译选项
add_compile_options(
    -Wall                               # 启用常见警告
    -Wextra                             # 启用额外警告
    -Wstrict-aliasing                   # 增强类型别名检查
    -Wundef                             # 检查未定义宏
    -Wshadow                            # 检查变量遮蔽
    -Wstrict-prototypes                 # 检查函数原型
    -Wmissing-declarations              # 检查缺失声明
    -Wmissing-prototypes                # 检查缺少函数原型声明
    -Wstrict-overflow                   # 检查求值溢出
    -Wformat                            # 检查格式化字符串的正确性
    -Wuninitialized                     # 检查未初始化的变量使用
    -Wunused                            # 检查未使用的变量、函数等
    -Wunused-parameter                  # 检查未使用的函数参数
    -Wpointer-arith                     # 检查指针运算的类型
    -pedantic                           # 要求代码严格符合C/C++标准
    -fPIC                               # 生成位置无关代码 (编译动态库)
#    -Wconversion                        # 检查类型转换
#    -Wsign-conversion                   # 检查符号转换
#    -Wcast-align                        # 检查指针对齐
#    -Wcast-qual                         # 检查强制类型转换丢失 const 限定符
#    -Werror                             # 将所有警告视为错误，编译过程中如果有警告产生则终止编译
#    -pedantic-errors                    # 将不符合标准的代码作为错误处理
)
