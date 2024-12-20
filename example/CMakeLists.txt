# 导入编译选项
include(CompileOptions)

add_executable(ctex main.c app.c)
target_link_libraries(ctex coter::coter)
