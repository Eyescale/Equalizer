# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  return()
endif()

set (DOXYGEN_DEP_TARGETS "")
macro(add_executable _target)
  _add_executable(${_target} ${ARGN})
  set_property(GLOBAL APPEND PROPERTY DOXYGEN_DEP_TARGETS ${_target})
endmacro()
macro(add_library _target)
  _add_library(${_target} ${ARGN})
  set_property(GLOBAL APPEND PROPERTY DOXYGEN_DEP_TARGETS ${_target})
endmacro()
