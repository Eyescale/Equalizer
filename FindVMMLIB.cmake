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

find_path(VMMLIB_INCLUDE_DIRS vmmlib/vmmlib.hpp
  $ENV{VMMLIB_ROOT}/include /usr/include /usr/local/include /opt/local/include)

if(VMMLIB_INCLUDE_DIRS)
  set(VMMLIB_FOUND 1)
  set(VMMLIB_DEB_BUILD_DEPENDENCIES "vmmlib1-dev")
  if(_vmmlib_output)
    message(STATUS "Found vmmlib in ${VMMLIB_INCLUDE_DIRS}")
  endif()
elseif(_vmmlib_output)
  message(${_vmmlib_version_output_type} "Could not find VMMLib")
endif()
