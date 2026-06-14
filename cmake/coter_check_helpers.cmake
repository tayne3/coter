#[=======================================================================[.rst:
Project Functions
--------------

Provides utility functions prefixed with ``coter_``.
#]=======================================================================]

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckLibraryExists)

#[=======================================================================[.rst:
.. command:: coter_check_header

  Checks if the specified header file exists.

  .. code-block:: cmake

    coter_check_header(<header>)

  Sets ``HAVE_<HEADER_NAME>`` to ``1`` if found, ``0`` otherwise.
#]=======================================================================]
function(coter_check_header header)
  string(TOUPPER "${header}" _temp_header)
  string(REGEX REPLACE "[^A-Z0-9_]" "_" _temp_header "${_temp_header}")

  set(_result_var "HAVE_${_temp_header}")
  check_include_files("${header}" ${_result_var})
  if (${_result_var})
    set(${_result_var} 1 PARENT_SCOPE)
  else()
    set(${_result_var} 0 PARENT_SCOPE)
  endif()
endfunction()

#[=======================================================================[.rst:
.. command:: coter_check_function

  Checks if the specified function exists in a given header.

  .. code-block:: cmake

    coter_check_function(<function> <header>)

  Sets ``HAVE_<FUNCTION_NAME>`` to ``1`` if found, ``0`` otherwise.
#]=======================================================================]
function(coter_check_function function header)
  string(TOUPPER "${function}" _temp_function)
  string(REGEX REPLACE "[^A-Z0-9_]" "_" _temp_function "${_temp_function}")

  set(_result_var "HAVE_${_temp_function}")
  check_symbol_exists("${function}" "${header}" ${_result_var})
  if (${_result_var})
    set(${_result_var} 1 PARENT_SCOPE)
  else()
    set(${_result_var} 0 PARENT_SCOPE)
  endif()
endfunction()

#[=======================================================================[.rst:
.. command:: coter_check_struct

  Checks if the specified struct exists and contains the given member.

  .. code-block:: cmake

    coter_check_struct(<struct> <member> <header>)

  Sets ``HAVE_<STRUCT_NAME>`` to ``1`` if found, ``0`` otherwise.
#]=======================================================================]
function(coter_check_struct struct member header)
  string(TOUPPER "${struct}" _temp_struct)
  string(REGEX REPLACE "[^A-Z0-9_]" "_" _temp_struct "${_temp_struct}")

  set(_result_var "HAVE_${_temp_struct}")
  check_struct_has_member("${struct}" "${member}" "${header}" ${_result_var})
  if (${_result_var})
    set(${_result_var} 1 PARENT_SCOPE)
  else()
    set(${_result_var} 0 PARENT_SCOPE)
  endif()
endfunction()

#[=======================================================================[.rst:
.. command:: coter_check_library_function

  Checks if the specified library exists and contains the given function.

  .. code-block:: cmake

    coter_check_library_function(<library> <function> <local>)

  Sets ``HAVE_<FUNCTION>_IN_<LIBRARY>`` to ``1`` if found, ``0`` otherwise.
#]=======================================================================]
function(coter_check_library_function library function local)
  string(TOUPPER "${library}" _temp_lib)
  string(REGEX REPLACE "[^A-Z0-9_]" "_" _temp_lib "${_temp_lib}")

  string(TOUPPER "${function}" _temp_func)
  string(REGEX REPLACE "[^A-Z0-9_]" "_" _temp_func "${_temp_func}")

  set(_result_var "HAVE_${_temp_func}_IN_${_temp_lib}")
  check_library_exists("${library}" "${function}" "${local}" ${_result_var})
  if (${_result_var})
    set(${_result_var} 1 PARENT_SCOPE)
  else()
    set(${_result_var} 0 PARENT_SCOPE)
  endif()
endfunction()
