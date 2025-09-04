# =======================================================================================================================
# Project Build Options and Variables
#
# Overview:
#   This module defines standard CMake build options and project-specific variables that control
#   the compilation process, such as build type, library type (shared/static), and whether to
#   build examples or tests.
#
# |---------------------------|---------------------|-----------------------------------------------|---------------------------------------------------------------|
# | Variable / Option         | Availability        | Default                                       | Description                                                   |
# |===========================|=====================|===============================================|===============================================================|
# | CMAKE_BUILD_TYPE          | Always              | "Debug" (if not set)                          | Standard CMake: Debug, Release, MinSizeRel, RelWithDebInfo.   |
# | COTER_BUILD_SHARED        | Always (Option)     | OFF                                           | Build shared libraries if ON, static if OFF.                  |
# | COTER_BUILD_EXAMPLE       | Top-Level (Option)  | OFF                                           | Build example programs.                                       |
# | COTER_BUILD_TEST          | Top-Level (Option)  | OFF                                           | Build test programs.                                          |
# |---------------------------|---------------------|-----------------------------------------------|---------------------------------------------------------------|

if(NOT CMAKE_CONFIGURATION_TYPES)
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
	endif()
	if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") # For debugging purposes, includes symbol table and unoptimized code (-O0 -g)
		OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")) # Includes optimized code and symbol table for debugging when errors occur (-O2 -g -DNDEBUG)
		set(PROJECT_BUILD_DEBUG ON)
	elseif(("${CMAKE_BUILD_TYPE}" STREQUAL "Release") # For release purposes, includes optimized code and typically does not include symbol table (-O3 -DNDEBUG)
		OR ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")) # Minimizes binary size, enables optimizations, and removes unnecessary symbols (-Os -DNDEBUG)
		set(PROJECT_BUILD_DEBUG OFF)
	else()
		message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}.")
	endif()
endif()

option(COTER_BUILD_SHARED "build shared library" OFF)
mark_as_advanced(COTER_BUILD_SHARED)

if(PROJECT_IS_TOP_LEVEL)
    option(COTER_BUILD_EXAMPLE "build example program" OFF)
    option(COTER_BUILD_TEST "build test program" OFF)
endif()
