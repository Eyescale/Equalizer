#
# Copyright 2012 Stefan Eilemann <eile@eyescale.ch>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#  - Neither the name of Eyescale Software GmbH nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#
#==================================
#
# - Find Lunchbox
# This module searches for the Lunchbox library
#    See https://github.com/Eyescale/Lunchbox
#
#==================================
#
# The following environment variables are respected for finding Lunchbox.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    LUNCHBOX_ROOT
#    EQ_ROOT
#
# This module defines the following output variables:
#
#    LUNCHBOX_FOUND - Was Lunchbox and all of the specified components found?
#
#    LUNCHBOX_VERSION - The version of Lunchbox which was found
#
#    LUNCHBOX_VERSION_ABI - The DSO version of Lunchbox which was found
#
#    LUNCHBOX_INCLUDE_DIRS - Where to find the headers
#
#    LUNCHBOX_LIBRARIES - The Lunchbox libraries
#
#==================================
# Example Usage:
#
#  find_package(Lunchbox 0.3.0 REQUIRED)
#  include_directories(${LUNCHBOX_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${LUNCHBOX_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _lunchbox_foo
#  Input variables of the form Lunchbox_FOO
#  Output variables of the form LUNCHBOX_FOO
#

#
# find and parse lunchbox/version.h
find_path(_lunchbox_INCLUDE_DIR lunchbox/version.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{LUNCHBOX_ROOT} ${LUNCHBOX_ROOT}
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt/local /opt
  )

if(Lunchbox_FIND_REQUIRED)
  set(_lunchbox_version_output_type FATAL_ERROR)
  set(_lunchbox_output 1)
else()
  set(_lunchbox_version_output_type STATUS)
  if(NOT Lunchbox_FIND_QUIETLY)
    set(_lunchbox_output 1)
  endif()
endif()

# Try to ascertain the version...
if(_lunchbox_INCLUDE_DIR)
  set(_lunchbox_Version_file "${_lunchbox_INCLUDE_DIR}/lunchbox/version.h")
  if("${_lunchbox_INCLUDE_DIR}" MATCHES "\\.framework$" AND
      NOT EXISTS "${_lunchbox_Version_file}")
    set(_lunchbox_Version_file "${_lunchbox_INCLUDE_DIR}/Headers/version.h")
  endif()

  if(EXISTS "${_lunchbox_Version_file}")
    file(READ "${_lunchbox_Version_file}" _lunchbox_Version_contents)
  else()
    set(_lunchbox_Version_contents "unknown")
  endif()

  if(_lunchbox_Version_contents MATCHES ".*define LUNCHBOX_VERSION_MAJOR[ \t]+([0-9]+).*")
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_MAJOR[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_MAJOR ${_lunchbox_Version_contents})
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_MINOR[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_MINOR ${_lunchbox_Version_contents})
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_PATCH[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_PATCH ${_lunchbox_Version_contents})
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_ABI[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_ABI ${_lunchbox_Version_contents})
    set(LUNCHBOX_VERSION "${LUNCHBOX_VERSION_MAJOR}.${LUNCHBOX_VERSION_MINOR}.${LUNCHBOX_VERSION_PATCH}"
      CACHE INTERNAL "The version of Lunchbox which was detected")
  endif()
else()
  set(_lunchbox_EPIC_FAIL TRUE)
  if(_lunchbox_output)
    message(${_lunchbox_version_output_type}
      "Can't find Lunchbox header file version.h.")
  endif()
endif()

#
# Version checking
#
if(Lunchbox_FIND_VERSION AND LUNCHBOX_VERSION)
  if(Lunchbox_FIND_VERSION_EXACT)
    if(NOT LUNCHBOX_VERSION VERSION_EQUAL ${Lunchbox_FIND_VERSION})
      set(_lunchbox_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT LUNCHBOX_VERSION VERSION_EQUAL ${Lunchbox_FIND_VERSION} AND 
        NOT LUNCHBOX_VERSION VERSION_GREATER ${Lunchbox_FIND_VERSION})
      set(_lunchbox_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(_lunchbox_LIBRARY lunchbox
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{LUNCHBOX_ROOT} ${LUNCHBOX_ROOT}
  PATH_SUFFIXES lib
  PATHS /usr /usr/local /opt/local /opt
)
        
# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_lunchbox_version_not_high_enough)
  set(_lunchbox_EPIC_FAIL TRUE)
  if(_lunchbox_output)
    message(${_lunchbox_version_output_type}
      "Version ${Lunchbox_FIND_VERSION} or higher of Lunchbox is required. "
      "Version ${LUNCHBOX_VERSION} was found in ${_lunchbox_INCLUDE_DIR}.")
  endif()
elseif(_lunchbox_version_not_exact)
  set(_lunchbox_EPIC_FAIL TRUE)
  if(_lunchbox_output)
    message(${_lunchbox_version_output_type}
      "Version ${Lunchbox_FIND_VERSION} of Lunchbox is required exactly. "
      "Version ${LUNCHBOX_VERSION} was found.")
  endif()
else()
  if(Lunchbox_FIND_REQUIRED)
    if(_lunchbox_LIBRARY MATCHES "_lunchbox_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the Lunchbox library.\n"
        "Consider using CMAKE_PREFIX_PATH or the LUNCHBOX_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lunchbox DEFAULT_MSG
                                    _lunchbox_LIBRARY _lunchbox_INCLUDE_DIR)
endif()

if(_lunchbox_EPIC_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(LUNCHBOX_FOUND FALSE)
    set(_lunchbox_LIBRARY)
    set(_lunchbox_INCLUDE_DIR)
endif()

set(LUNCHBOX_INCLUDE_DIRS ${_lunchbox_INCLUDE_DIR})
set(LUNCHBOX_LIBRARIES ${_lunchbox_LIBRARY})
get_filename_component(LUNCHBOX_LIBRARY_DIR ${_lunchbox_LIBRARY} PATH)

if(LUNCHBOX_FOUND)
  if(_lunchbox_output)
    message(STATUS "Found Lunchbox ${LUNCHBOX_VERSION}/${LUNCHBOX_VERSION_ABI}"
      " in ${LUNCHBOX_INCLUDE_DIRS};${LUNCHBOX_LIBRARIES}")
  endif()
endif()

