# Copyright (c) 2013 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# - Try to find the SAGE library
# Once done this will define
#
#  SAGE_ROOT - Set this variable to the root installation of CAIRO
#
# Read-Only variables:
#  SAGE_FOUND - system has the SAGE library
#  SAGE_INCLUDE_DIR - the SAGE include directory
#  SAGE_LIBRARIES - The libraries needed to use SAGE
#  SAGE_VERSION - This is set to $major.$minor.$revision (eg. 0.9.8)

include(FindPackageHandleStandardArgs)

if(SAGE_FIND_REQUIRED)
  set(_SAGE_required REQUIRED)
  set(_SAGE_version_output_type FATAL_ERROR)
  set(_SAGE_output 1)
else()
  set(_SAGE__version_output_type STATUS)
  if(NOT SAGE_FIND_QUIETLY)
    set(_SAGE_output 1)
  endif()
endif()
if(SAGE_FIND_QUIETLY)
  set(_SAGE_quiet QUIET)
endif()

find_path(_SAGE_INCLUDE_DIR sage.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{SAGE_ROOT} ${SAGE_ROOT}
  PATH_SUFFIXES include sage/include
  PATHS /usr/local/sage /usr /usr/local /opt /opt/local)

if(_SAGE_INCLUDE_DIR AND EXISTS "${_SAGE_INCLUDE_DIR}/sage.h")
  set(_SAGE_Version_file "${_SAGE_INCLUDE_DIR}/sage.h")
  file(READ ${_SAGE_Version_file} _SAGE_header_contents)
  string(REGEX REPLACE ".*define SAGE_VERSION[ \t]+\"([0-9].+)\".*"
    "\\1" _SAGE_VERSION ${_SAGE_header_contents})

  set(SAGE_VERSION ${_SAGE_VERSION}
    CACHE INTERNAL "The version of SAGE which was detected")
else()
  set(_SAGE_EPIC_FAIL TRUE)
  if(_SAGE_output)
    message(${_SAGE_version_output_type} "Can't find SAGE header file sage.h.")
  endif()
endif()

# Version checking
if(SAGE_FIND_VERSION AND SAGE_VERSION)
  if(SAGE_FIND_VERSION_EXACT)
    if(NOT SAGE_VERSION VERSION_EQUAL ${SAGE_FIND_VERSION})
      set(_SAGE_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT SAGE_VERSION VERSION_EQUAL ${SAGE_FIND_VERSION} AND
        NOT SAGE_VERSION VERSION_GREATER ${SAGE_FIND_VERSION})
      set(_SAGE_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(SAGE_SAIL_LIBRARY sail
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{SAGE_ROOT} ${SAGE_ROOT}
  PATH_SUFFIXES lib lib64 sage/lib sage/lib64
  PATHS /usr/local/sage /usr /usr/local /opt /opt/local)

find_library(SAGE_QUANTA_LIBRARY quanta
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{SAGE_ROOT} ${SAGE_ROOT}
  PATH_SUFFIXES lib lib64 sage/lib sage/lib64
  PATHS /usr/local/sage /usr /usr/local /opt /opt/local)

# Inform the users with an error message based on what version they
# have vs. what version was required.
if(NOT SAGE_VERSION)
  set(_SAGE_EPIC_FAIL TRUE)
  find_package_handle_standard_args(SAGE DEFAULT_MSG
                                    SAGE_SAIL_LIBRARY _SAGE_INCLUDE_DIR)
elseif(_SAGE_version_not_high_enough)
  set(_SAGE_EPIC_FAIL TRUE)
  message(${_SAGE_version_output_type}
    "Version ${SAGE_FIND_VERSION} or higher of SAGE is required. "
    "Version ${SAGE_VERSION} was found in ${_SAGE_Version_file}.")
elseif(_SAGE_version_not_exact)
  set(_SAGE_EPIC_FAIL TRUE)
  message(${_SAGE_version_output_type}
    "Version ${SAGE_FIND_VERSION} of SAGE is required exactly. "
    "Version ${SAGE_VERSION} was found.")
else()
  if(SAGE_FIND_REQUIRED)
    if(SAGE_SAIL_LIBRARY MATCHES "SAGE_SAIL_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the SAGE sail library.\n"
        "Consider using CMAKE_PREFIX_PATH or the SAGE_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
    if(SAGE_QUANTA_LIBRARY MATCHES "SAGE_QUANTA_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the SAGE quanta library.\n"
        "Consider using CMAKE_PREFIX_PATH or the SAGE_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  find_package_handle_standard_args(SAGE DEFAULT_MSG
                                    SAGE_SAIL_LIBRARY _SAGE_INCLUDE_DIR)
endif()

if(_SAGE_EPIC_FAIL)
  # Zero out everything, we didn't meet version requirements
  set(SAGE_FOUND FALSE)
  set(SAGE_SAIL_LIBRARY)
  set(SAGE_QUANTA_LIBRARY)
  set(_SAGE_INCLUDE_DIR)
  set(SAGE_INCLUDE_DIRS)
  set(SAGE_LIBRARIES)
else()
  set(SAGE_INCLUDE_DIRS ${_SAGE_INCLUDE_DIR})
  set(SAGE_LIBRARIES ${SAGE_SAIL_LIBRARY} ${SAGE_QUANTA_LIBRARY})
endif()
