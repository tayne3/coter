
set(PTHREADS_ROOT ${THIRD_DIR}/pthreads-w32)
include(FindPthreadW32)
set(LIBS ${PTHREADS_LIBS})

# see configure
# Checks for header files
project_check_header("stdbool.h")
project_check_header("stdint.h")
project_check_header("stdatomic.h")
project_check_header("sys/types.h")
project_check_header("sys/stat.h")
project_check_header("sys/time.h")
project_check_header("fcntl.h")
project_check_header("pthread.h")
project_check_header("endian.h")
project_check_header("sys/endian.h")

project_check_function("gettid" "unistd.h")
project_check_function("strlcpy" "string.h")
project_check_function("strlcat" "string.h")
project_check_function("memrchr" "string.h")
project_check_function("clock_gettime" "time.h")
project_check_function("gettimeofday" "sys/time.h")
project_check_function("sem_timedwait" "semaphore.h")
project_check_function("pipe" "unistd.h")
project_check_function("socketpair" "sys/socket.h")
project_check_function("eventfd" "sys/eventfd.h")
project_check_function("setproctitle" "unistd.h")

# project_check_function("pthread_spin_lock" "pthread.h")
# project_check_function("pthread_mutex_timedlock" "pthread.h")

project_check_struct("struct timespec" "tv_sec" "time.h")
project_check_library_function("rt" "clock_gettime" "")

# 生成配置信息头文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/base/prefix/ct_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/base/prefix/ct_config.h"
    @ONLY
)

if(WIN32)
    set(LIBS ${LIBS} secur32 crypt32 winmm iphlpapi ws2_32)
    set(LIBS ${LIBS} dbghelp)
endif()

if(UNIX)
    set(LIBS ${LIBS} pthread m dl)
    if(CMAKE_COMPILER_IS_GNUCC)
        set(LIBS ${LIBS} rt)
    endif()
endif()

# 打印版本号
message(STATUS "Coter version: ${PROJECT_VERSION}")

# 收集当前目录及其子目录中的源文件
project_list_sources(files)
# 创建库目标
add_library(coter_obj OBJECT ${files})
# 添加别名
add_library(coter::obj ALIAS coter_obj)
# 设置头文件查找路径
target_include_directories(coter_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
# 添加宏
target_compile_definitions(coter_obj PRIVATE DLL_EXPORT)
# 链接库
target_link_libraries(coter_obj PUBLIC ${LIBS})

# 编译动态库
if(COTER_BUILD_SHARED)
    add_library(coter_shared SHARED $<TARGET_OBJECTS:coter_obj>)
    add_library(coter::shared ALIAS coter_shared)
    target_include_directories(coter_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(coter_shared PROPERTIES OUTPUT_NAME coter)
    target_compile_definitions(coter_shared PRIVATE DLL_EXPORT)
    target_link_libraries(coter_shared PUBLIC ${LIBS})
    if(NOT TARGET coter::coter)
        add_library(coter::coter ALIAS coter_shared)
    endif()
    message(STATUS "Coter build shared library: ON")
else()
    message(STATUS "Coter build shared library: OFF")
endif()

# 编译静态库
if(COTER_BUILD_STATIC)
    add_library(coter_static STATIC $<TARGET_OBJECTS:coter_obj>)
    add_library(coter::static ALIAS coter_static)
    target_include_directories(coter_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(coter_static PROPERTIES OUTPUT_NAME coter)
    target_compile_definitions(coter_static PRIVATE DLL_EXPORT)
    target_link_libraries(coter_static PUBLIC ${LIBS})
    if(NOT TARGET coter::coter)
        add_library(coter::coter ALIAS coter_static)
    endif()
    message(STATUS "Coter build static library: ON")
else()
    message(STATUS "Coter build static library: OFF")
endif()
