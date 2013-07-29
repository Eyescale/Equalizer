# hooks to gather targets in global properties ALL_DEP_TARGETS and
# ALL_LIB_TARGETS for future processing

include(CMakeParseArguments)
set(ALL_DEP_TARGETS "")
set(ALL_LIB_TARGETS "")
macro(add_executable _target)
  _add_executable(${_target} ${ARGN})
  set_property(GLOBAL APPEND PROPERTY ALL_DEP_TARGETS ${_target})
endmacro()
macro(add_library _target)
  _add_library(${_target} ${ARGN})

  # ignore IMPORTED add_library from finders (e.g. Qt)
  cmake_parse_arguments(_arg "IMPORTED" "" "" ${ARGN})

  # ignore user-specified targets, e.g. language bindings
  list(FIND IGNORE_LIB_TARGETS ${_target} _ignore_target)

  if(NOT _arg_IMPORTED AND _ignore_target EQUAL -1)
    # add defines TARGET_DSO_NAME and TARGET_SHARED for dlopen() usage
    get_target_property(THIS_DEFINITIONS ${_target} COMPILE_DEFINITIONS)
    if(NOT THIS_DEFINITIONS)
      set(THIS_DEFINITIONS) # clear THIS_DEFINITIONS-NOTFOUND
    endif()
    string(TOUPPER ${_target} _TARGET)

    if(MSVC OR XCODE_VERSION)
      set(_libraryname ${CMAKE_SHARED_LIBRARY_PREFIX}${_target}${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
      if(APPLE)
        set(_libraryname ${CMAKE_SHARED_LIBRARY_PREFIX}${_target}.${VERSION_ABI}${CMAKE_SHARED_LIBRARY_SUFFIX})
      else()
        set(_libraryname ${CMAKE_SHARED_LIBRARY_PREFIX}${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}.${VERSION_ABI})
      endif()
    endif()

    list(APPEND THIS_DEFINITIONS
      ${_TARGET}_SHARED ${_TARGET}_DSO_NAME=\"${_libraryname}\")

    set_target_properties(${_target} PROPERTIES
      COMPILE_DEFINITIONS "${THIS_DEFINITIONS}")

    set_property(GLOBAL APPEND PROPERTY ALL_DEP_TARGETS ${_target})
    set_property(GLOBAL APPEND PROPERTY ALL_LIB_TARGETS ${_target})
  endif()
endmacro()
