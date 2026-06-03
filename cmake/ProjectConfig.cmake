include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(COTER_BUILD_TEST "build test program" OFF)
  option(COTER_BUILD_EXAMPLE "build examples" OFF)

  get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(NOT isMultiConfig
     AND NOT CMAKE_BUILD_TYPE
     AND NOT CMAKE_CONFIGURATION_TYPES
  )
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
  endif()

  # Enable Link-Time Optimization (LTO/IPO) for Release configurations if supported
  if(NOT (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0))
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _lto_supported LANGUAGES C CXX)
    if(_lto_supported)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    endif()
  endif()
    
  if(NOT DEFINED CMAKE_C_VISIBILITY_PRESET)
    set(CMAKE_C_VISIBILITY_PRESET hidden CACHE STRING "Preset for the export of private symbols")
    set_property(CACHE CMAKE_C_VISIBILITY_PRESET PROPERTY STRINGS hidden default)
  endif()

  if(NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden CACHE STRING "Preset for the export of private symbols")
    set_property(CACHE CMAKE_CXX_VISIBILITY_PRESET PROPERTY STRINGS hidden default)
  endif()

  if(NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON CACHE BOOL "Whether to add a compile flag to hide symbols of inline functions")
  endif()
  
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  
  include(GNUInstallDirs)
  set(CMAKE_SKIP_BUILD_RPATH OFF)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)
  if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
  endif()
endif()

add_library(coter_compile_dependency INTERFACE)
target_compile_features(coter_compile_dependency INTERFACE c_std_99 cxx_std_11)
add_library(coter_public_dependency INTERFACE)
target_compile_features(coter_public_dependency INTERFACE c_std_99 cxx_std_11)

# Enable GNU extensions on Linux/Unix systems (covers glibc, musl, etc.)
if((UNIX AND NOT APPLE) OR CMAKE_SYSTEM_NAME MATCHES "Linux|GNU")
  target_compile_definitions(coter_compile_dependency INTERFACE
    _GNU_SOURCE=1
  )
endif()

if(WIN32)
  target_compile_definitions(coter_compile_dependency INTERFACE
    _CRT_SECURE_NO_WARNINGS          # Suppress secure CRT warnings
    _CRT_NONSTDC_NO_DEPRECATE        # Suppress POSIX deprecation warnings
    _WINSOCK_DEPRECATED_NO_WARNINGS  # Suppress Winsock deprecation warnings
    NOMINMAX                         # Prevent min/max macro definitions
  )
  if(MINGW)
    target_compile_definitions(coter_compile_dependency INTERFACE
      __USE_MINGW_ANSI_STDIO=1
    )
  endif()
endif()

if(MSVC)
  target_compile_options(coter_compile_dependency INTERFACE 
		"$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>"
    "$<$<COMPILE_LANGUAGE:C>:/utf-8>"   # UTF-8 source and execution character set
    "$<$<COMPILE_LANGUAGE:CXX>:/utf-8>"
    "$<$<COMPILE_LANGUAGE:C>:/wd4018>"  # signed/unsigned comparison
    "$<$<COMPILE_LANGUAGE:CXX>:/wd4018>" 
    "$<$<COMPILE_LANGUAGE:C>:/wd4100>"  # unused parameter
    "$<$<COMPILE_LANGUAGE:CXX>:/wd4100>"
    "$<$<COMPILE_LANGUAGE:C>:/wd4102>"  # unreferenced label
    "$<$<COMPILE_LANGUAGE:CXX>:/wd4102>"
    "$<$<COMPILE_LANGUAGE:C>:/wd4244>"  # conversion with possible loss of data
    "$<$<COMPILE_LANGUAGE:CXX>:/wd4244>"
    "$<$<COMPILE_LANGUAGE:C>:/wd4267>"  # 64-bit to 32-bit conversion
    "$<$<COMPILE_LANGUAGE:CXX>:/wd4267>" 
    "$<$<COMPILE_LANGUAGE:C>:/wd4819>"  # Unicode character issues
    "$<$<COMPILE_LANGUAGE:CXX>:/wd4819>"
    "$<$<COMPILE_LANGUAGE:C>:/wd4996>"  # deprecated function warnings
    "$<$<COMPILE_LANGUAGE:CXX>:/wd4996>"
	)
else()
  target_compile_options(coter_compile_dependency INTERFACE
    "$<$<COMPILE_LANGUAGE:C>:-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=/>"
    "$<$<COMPILE_LANGUAGE:CXX>:-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=/>"
  )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set(_is_clangcl TRUE)
else()
  set(_is_clangcl FALSE)
endif()

if(NOT MSVC AND NOT _is_clangcl)
  target_compile_options(coter_compile_dependency INTERFACE
    "$<$<COMPILE_LANGUAGE:C>:-Wall>"
    "$<$<COMPILE_LANGUAGE:C>:-Wextra>"
    "$<$<COMPILE_LANGUAGE:CXX>:-Wall>"
    "$<$<COMPILE_LANGUAGE:CXX>:-Wextra>"
  )

  # Probe once via the C driver: on any GCC/Clang toolchain the C and CXX
  # drivers share the same -W flag set, so a single check covers both.
  include(CheckCCompilerFlag)
  check_c_compiler_flag("-Werror=return-type" HAVE_WERROR_RETURN_TYPE)
  if(HAVE_WERROR_RETURN_TYPE)
    target_compile_options(coter_compile_dependency INTERFACE
      "$<$<COMPILE_LANGUAGE:C>:-Werror=return-type>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Werror=return-type>"
    )
  endif()

  # GCC < 6.0 incorrectly fires -Wmissing-field-initializers on valid C99
  # designated / partial initializers. The false positive was fixed in GCC 6.
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0")
    target_compile_options(coter_compile_dependency INTERFACE
      "$<$<COMPILE_LANGUAGE:C>:-Wno-missing-field-initializers>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wno-missing-field-initializers>"
    )
  endif()
endif()
