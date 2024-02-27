
# 遍历所有源文件
file(GLOB_RECURSE FILES CONFIGURE_DEPENDS *.c *.cpp *.cc *.h *.hpp)
foreach(filepath ${FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND sources_files ${filename})
endforeach(filepath)

# 编译对象库
add_library(coter_obj OBJECT ${sources_files})
add_library(coter::obj ALIAS coter_obj)
# 设置头文件查找路径
target_include_directories(coter_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# 链接库文件
target_link_libraries(coter_obj PUBLIC pthread rt)

# 编译动态库
if(COTER_BUILD_SHARED)
    add_library(coter_shared SHARED $<TARGET_OBJECTS:coter_obj>)
    add_library(coter::shared ALIAS coter_shared)
    target_include_directories(coter_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(coter_shared PROPERTIES OUTPUT_NAME coter)
    message(STATUS "Coter build shared library: ON")
else()
    message(STATUS "Coter build shared library: OFF")
endif()

# 编译静态库
if(COTER_BUILD_STATIC)
    add_library(coter_static STATIC $<TARGET_OBJECTS:coter_obj>)
    add_library(coter::static ALIAS coter_static)
    target_include_directories(coter_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(coter_static PROPERTIES OUTPUT_NAME coter)
    message(STATUS "Coter build static library: ON")
else()
    message(STATUS "Coter build static library: OFF")
endif()
