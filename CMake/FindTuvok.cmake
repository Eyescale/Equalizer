# Copyright (c) 2013 Ahmet Bilgili <ahmet.bilgili@epfl.ch>
# Modified from: FindSAGE.cmake

# - Try to find the TUVOK library
# Once done this will define
#
#  TUVOK_ROOT - Set this variable to the root installation of CAIRO
#
# Read-Only variables:
#  TUVOK_FOUND - system has the TUVOK library
#  TUVOK_INCLUDE_DIR - the TUVOK include directory
#  TUVOK_LIBRARY - The libraries needed to use SAGE
#  (TODO) TUVOK_VERSION - This is set to $major.$minor.$revision (eg. 0.9.8)

include(FindPackageHandleStandardArgs)

if(TUVOK_FIND_REQUIRED)
  set(_TUVOK_required REQUIRED)
  set(_TUVOK_output 1)
else()
  if(NOT TUVOK_FIND_QUIETLY)
    set(_TUVOK_output 1)
  endif()
endif()
if(TUVOK_FIND_QUIETLY)
  set(_TUVOK_quiet QUIET)
endif()

find_path(_TUVOK_INCLUDE_DIR StdTuvokDefines.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{TUVOK_ROOT} ${TUVOK_ROOT}
  PATH_SUFFIXES include/Tuvok
  PATHS /usr/local /usr /opt )

if(_TUVOK_INCLUDE_DIR AND EXISTS "${_TUVOK_INCLUDE_DIR}/StdTuvokDefines.h")
  set(_TUVOK_FAIL FALSE)
else()
  set(_TUVOK_FAIL TRUE)
  if(_TUVOK_output)
    message(STATUS "Can't find Tuvok header file StdTuvokDefines.h.")
  endif()
endif()

find_library(TUVOK_LIBRARY Tuvok
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{TUVOK_ROOT} ${TUVOK_ROOT}
  PATH_SUFFIXES lib lib64
  PATHS /usr/local /usr /usr/local /opt /opt/local)

if(TUVOK_FIND_REQUIRED)
 if(TUVOK_LIBRARY MATCHES "TUVOK_LIBRARY-NOTFOUND")
   message(FATAL_ERROR "Missing the Tuvok library.\n"
     "Consider using CMAKE_PREFIX_PATH or the TUVOK_ROOT environment variable. "
     "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
 endif()
endif()
find_package_handle_standard_args(TUVOK DEFAULT_MSG TUVOK_LIBRARY _TUVOK_INCLUDE_DIR)

if(_TUVOK_FAIL)
  # Zero out everything, we didn't meet version requirements
  set(TUVOK_FOUND FALSE)
  set(TUVOK_LIBRARY)
  set(_TUVOK_INCLUDE_DIR)
  set(TUVOK_INCLUDE_DIR)
  set(TUVOK_LIBRARY)
else()
  set(TUVOK_INCLUDE_DIR ${_TUVOK_INCLUDE_DIR})
endif()
