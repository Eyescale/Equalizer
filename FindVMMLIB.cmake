# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

if(VMMLIB_FIND_REQUIRED)
  set(_vmmlib_version_output_type FATAL_ERROR)
  set(_vmmlib_output 1)
else()
  set(_vmmlib_version_output_type STATUS)
  if(NOT VMMLIB_FIND_QUIETLY)
    set(_vmmlib_output 1)
  endif()
endif()

find_path(VMMLIB_INCLUDE_DIR vmmlib/vmmlib.hpp
  /usr/include /usr/local/include /opt/local/include ${VMMLIB_ROOT}/include)

if(_vmmlib_output)
  message(STATUS "Found vmmlib in ${VMMLIB_INCLUDE_DIR}")
endif()
