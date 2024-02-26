
if(MSVC)
	return()
endif()

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
	message(STATUS "Enabling CCache: ${CCACHE_PROGRAM}")
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
else(CCACHE_PROGRAM)
	message(WARNING "CCache Not Found")
endif(CCACHE_PROGRAM)
