
set(PTHREADS_ROOT ${SUB_DIR}/pthreads-w32)
include(FindPthreadW32)

if(COTER_BUILD_TEST)
    add_subdirectory(${SUB_DIR}/ctunit)
endif()
