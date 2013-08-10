# Copyright (c) 2013 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# - Try to find the DisplayCluster library
# Once done this will define
#
#  DISPLAYCLUSTER_ROOT - Set this variable to the root installation
#
# Read-Only variables:
#  DISPLAYCLUSTER_FOUND - system has the DisplayCluster library
#  DISPLAYCLUSTER_INCLUDE_DIR - the DisplayCluster include directory
#  DISPLAYCLUSTER_LIBRARIES - The libraries needed to use DisplayCluster
#  DISPLAYCLUSTER_VERSION - This is set to $major.$minor.$patch (eg. 0.9.8)

include(FindPackageHandleStandardArgs)

if(DisplayCluster_FIND_REQUIRED)
  set(_DISPLAYCLUSTER_output_type FATAL_ERROR)
else()
  set(_DISPLAYCLUSTER_output_type STATUS)
endif()
if(DisplayCluster_FIND_QUIETLY)
  set(_DISPLAYCLUSTER_output)
else()
  set(_DISPLAYCLUSTER_output 1)
endif()

find_path(_DISPLAYCLUSTER_INCLUDE_DIR dcStream.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{DISPLAYCLUSTER_ROOT} ${DISPLAYCLUSTER_ROOT}
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt /opt/local)

if(_DISPLAYCLUSTER_INCLUDE_DIR AND EXISTS "${_DISPLAYCLUSTER_INCLUDE_DIR}/dcStream.h")
  if(EXISTS "${_DISPLAYCLUSTER_INCLUDE_DIR}/dcVersion.h")
   set(_DISPLAYCLUSTER_Version_file "${_DISPLAYCLUSTER_INCLUDE_DIR}/dcVersion.h")
   file(READ ${_DISPLAYCLUSTER_Version_file} _DISPLAYCLUSTER_header_contents)
   string(REGEX REPLACE ".*#define DISPLAYCLUSTER_VERSION ([0-9].[0-9].[0-9]).*" "\\1"
     _DISPLAYCLUSTER_VERSION "${_DISPLAYCLUSTER_header_contents}")
   set(DISPLAYCLUSTER_VERSION ${_DISPLAYCLUSTER_VERSION}
      CACHE INTERNAL "The version of DisplayCluster which was detected")
  else()
    set(DISPLAYCLUSTER_VERSION 0.1.0
      CACHE INTERNAL "The version of DisplayCluster which was detected")
  endif()
else()
  set(_DISPLAYCLUSTER_EPIC_FAIL TRUE)
  if(_DISPLAYCLUSTER_output)
    message(${_DISPLAYCLUSTER_output_type}
      "Can't find DisplayCluster header file dcStream.h.")
  endif()
endif()

# Version checking
if(DISPLAYCLUSTER_FIND_VERSION AND DISPLAYCLUSTER_VERSION)
  if(DISPLAYCLUSTER_FIND_VERSION_EXACT)
    if(NOT DISPLAYCLUSTER_VERSION VERSION_EQUAL ${DISPLAYCLUSTER_FIND_VERSION})
      set(_DISPLAYCLUSTER_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT DISPLAYCLUSTER_VERSION VERSION_EQUAL ${DISPLAYCLUSTER_FIND_VERSION} AND
        NOT DISPLAYCLUSTER_VERSION VERSION_GREATER ${DISPLAYCLUSTER_FIND_VERSION})
      set(_DISPLAYCLUSTER_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(DISPLAYCLUSTER_LIBRARY DisplayCluster
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{DISPLAYCLUSTER_ROOT} ${DISPLAYCLUSTER_ROOT}
  PATH_SUFFIXES lib lib64
  PATHS /usr /usr/local /opt /opt/local)

# Inform the users with an error message based on what version they
# have vs. what version was required.
if(NOT DISPLAYCLUSTER_VERSION)
  set(_DISPLAYCLUSTER_EPIC_FAIL TRUE)
elseif(_DISPLAYCLUSTER_version_not_high_enough)
  set(_DISPLAYCLUSTER_EPIC_FAIL TRUE)
  if(_DISPLAYCLUSTER_output)
    message(${_DISPLAYCLUSTER_output_type}
      "Version ${DISPLAYCLUSTER_FIND_VERSION} or higher of DisplayCluster is required. "
      "Version ${DISPLAYCLUSTER_VERSION} was found in ${_DISPLAYCLUSTER_Version_file}.")
  endif()
elseif(_DISPLAYCLUSTER_version_not_exact)
  set(_DISPLAYCLUSTER_EPIC_FAIL TRUE)
  if(_DISPLAYCLUSTER_output)
    message(${_DISPLAYCLUSTER_output_type}
      "Version ${DISPLAYCLUSTER_FIND_VERSION} of DisplayCluster is required exactly. "
      "Version ${DISPLAYCLUSTER_VERSION} was found.")
  endif()
else()
  if(DISPLAYCLUSTER_FIND_REQUIRED)
    if(DISPLAYCLUSTER_LIBRARY MATCHES "DISPLAYCLUSTER_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the DisplayCluster library.\n"
        "Consider using CMAKE_PREFIX_PATH or the DISPLAYCLUSTER_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  find_package_handle_standard_args(DISPLAYCLUSTER DEFAULT_MSG
                                    DISPLAYCLUSTER_LIBRARY _DISPLAYCLUSTER_INCLUDE_DIR)
endif()

if(_DISPLAYCLUSTER_EPIC_FAIL)
  # Zero out everything, we didn't meet version requirements
  set(DISPLAYCLUSTER_FOUND FALSE)
  set(DISPLAYCLUSTER_LIBRARY)
  set(_DISPLAYCLUSTER_INCLUDE_DIR)
  set(DISPLAYCLUSTER_INCLUDE_DIRS)
  set(DISPLAYCLUSTER_LIBRARIES)
else()
  set(DISPLAYCLUSTER_INCLUDE_DIRS ${_DISPLAYCLUSTER_INCLUDE_DIR})
  set(DISPLAYCLUSTER_LIBRARIES ${DISPLAYCLUSTER_LIBRARY})
endif()
