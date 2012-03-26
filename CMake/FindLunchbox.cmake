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
# - Find LunchBox
# This module searches for the LunchBox library
#    See http://www.liblunchBox.net
#
#==================================
#
# The following environment variables are respected for finding LunchBox.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    LUNCHBOX_ROOT
#    EQ_ROOT
#
# This module defines the following output variables:
#
#    LUNCHBOX_FOUND - Was LunchBox and all of the specified components found?
#
#    LUNCHBOX_VERSION - The version of LunchBox which was found
#
#    LUNCHBOX_VERSION_ABI - The DSO version of LunchBox which was found
#
#    LUNCHBOX_INCLUDE_DIRS - Where to find the headers
#
#    LUNCHBOX_LIBRARIES - The LunchBox libraries
#
#==================================
# Example Usage:
#
#  find_package(LunchBox 0.3.0 REQUIRED)
#  include_directories(${LUNCHBOX_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${LUNCHBOX_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _lunchBox_foo
#  Input variables of the form LunchBox_FOO
#  Output variables of the form LUNCHBOX_FOO
#

#
# find and parse lunchbox/version.h
find_path(_lunchBox_INCLUDE_DIR lunchbox/version.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{LUNCHBOX_ROOT} ${LUNCHBOX_ROOT}
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt/local /opt
  )

if(LunchBox_FIND_REQUIRED)
  set(_lunchBox_version_output_type FATAL_ERROR)
else()
  set(_lunchBox_version_output_type STATUS)
endif()

# Try to ascertain the version...
if(_lunchBox_INCLUDE_DIR)
  set(_lunchBox_Version_file "${_lunchBox_INCLUDE_DIR}/lunchbox/version.h")
  if("${_lunchBox_INCLUDE_DIR}" MATCHES "\\.framework$" AND
      NOT EXISTS "${_lunchBox_Version_file}")
    set(_lunchBox_Version_file "${_lunchBox_INCLUDE_DIR}/Headers/version.h")
  endif()

  if(EXISTS "${_lunchBox_Version_file}")
    file(READ "${_lunchBox_Version_file}" _lunchBox_Version_contents)
  else()
    set(_lunchBox_Version_contents "unknown")
  endif()

  if(_lunchBox_Version_contents MATCHES ".*define LUNCHBOX_VERSION_MAJOR[ \t]+([0-9]+).*")
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_MAJOR[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_MAJOR ${_lunchBox_Version_contents})
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_MINOR[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_MINOR ${_lunchBox_Version_contents})
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_PATCH[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_PATCH ${_lunchBox_Version_contents})
    string(REGEX REPLACE ".*define LUNCHBOX_VERSION_ABI[ \t]+([0-9]+).*"
      "\\1" LUNCHBOX_VERSION_ABI ${_lunchBox_Version_contents})
    set(LUNCHBOX_VERSION "${LUNCHBOX_VERSION_MAJOR}.${LUNCHBOX_VERSION_MINOR}.${LUNCHBOX_VERSION_PATCH}"
      CACHE INTERNAL "The version of LunchBox which was detected")
  endif()
else()
  set(_lunchBox_EPIC_FAIL TRUE)
  message(${_lunchBox_version_output_type}
    "Can't find LunchBox header file version.h.")
endif()

#
# Version checking
#
if(LunchBox_FIND_VERSION AND LUNCHBOX_VERSION)
  if(LunchBox_FIND_VERSION_EXACT)
    if(NOT LUNCHBOX_VERSION VERSION_EQUAL ${LunchBox_FIND_VERSION})
      set(_lunchBox_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT LUNCHBOX_VERSION VERSION_EQUAL ${LunchBox_FIND_VERSION} AND 
        NOT LUNCHBOX_VERSION VERSION_GREATER ${LunchBox_FIND_VERSION})
      set(_lunchBox_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(_lunchBox_LIBRARY lunchbox
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{LUNCHBOX_ROOT} ${LUNCHBOX_ROOT}
  PATH_SUFFIXES lib
  PATHS /usr /usr/local /opt/local /opt
)
        
# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_lunchBox_version_not_high_enough)
  set(_lunchBox_EPIC_FAIL TRUE)
  message(${_lunchBox_version_output_type}
    "Version ${LunchBox_FIND_VERSION} or higher of LunchBox is required. "
    "Version ${LUNCHBOX_VERSION} was found in ${_lunchBox_INCLUDE_DIR}.")
elseif(_lunchBox_version_not_exact)
  set(_lunchBox_EPIC_FAIL TRUE)
  message(${_lunchBox_version_output_type}
    "Version ${LunchBox_FIND_VERSION} of LunchBox is required exactly. "
    "Version ${LUNCHBOX_VERSION} was found.")
else()
  if(LunchBox_FIND_REQUIRED)
    if(_lunchBox_LIBRARY MATCHES "_lunchBox_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the LunchBox library.\n"
        "Consider using CMAKE_PREFIX_PATH or the LUNCHBOX_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LunchBox DEFAULT_MSG
                                    _lunchBox_LIBRARY _lunchBox_INCLUDE_DIR)
endif()

if(_lunchBox_EPIC_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(LUNCHBOX_FOUND FALSE)
    set(_lunchBox_LIBRARY)
    set(_lunchBox_INCLUDE_DIR)
endif()

set(LUNCHBOX_INCLUDE_DIRS ${_lunchBox_INCLUDE_DIR})
set(LUNCHBOX_LIBRARIES ${_lunchBox_LIBRARY})
get_filename_component(LUNCHBOX_LIBRARY_DIR ${_lunchBox_LIBRARY} PATH)

if(LUNCHBOX_FOUND)
  message(STATUS "Found LunchBox ${LUNCHBOX_VERSION}/${LUNCHBOX_VERSION_ABI} in "
    "${LUNCHBOX_INCLUDE_DIRS};${LUNCHBOX_LIBRARIES}")
endif()

