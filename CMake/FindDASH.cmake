#
# Copyright 2012 Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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
# - Find DASH
# This module searches for the DASH library
#    See https://github.com/BlueBrain/dash
#
#==================================
#
# The following environment variables are respected for finding DASH.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    DASH_ROOT
#
# This module defines the following output variables:
#
#    DASH_FOUND - Was DASH and all of the specified components found?
#
#    DASH_VERSION - The version of DASH which was found
#
#    DASH_VERSION_ABI - The DSO version of DASH which was found
#
#    DASH_INCLUDE_DIRS - Where to find the headers
#
#    DASH_LIBRARIES - The DASH libraries
#
#==================================
# Example Usage:
#
#  find_package(DASH 0.1.0 REQUIRED)
#  include_directories(${DASH_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${DASH_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _dash_foo
#  Input variables of the form Dash_FOO
#  Output variables of the form DASH_FOO
#

#
# find and parse dash/version.h
find_path(_dash_INCLUDE_DIR dash/Version.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{DASH_ROOT} ${DASH_ROOT} }
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt/local /opt
  )

if(Dash_FIND_REQUIRED)
  set(_dash_version_output_type FATAL_ERROR)
  set(_dash_output 1)
else()
  set(_dash_version_output_type STATUS)
  if(NOT Dash_FIND_QUIETLY)
    set(_dash_output 1)
  endif()
endif()

# Try to ascertain the version...
if(_dash_INCLUDE_DIR)
  set(_dash_Version_file "${_dash_INCLUDE_DIR}/dash/Version.h")

  if(EXISTS "${_dash_Version_file}")
    file(READ "${_dash_Version_file}" _dash_Version_contents)
  else()
    set(_dash_Version_contents "unknown")
  endif()

  if(_dash_Version_contents MATCHES ".*define DASH_VERSION_MAJOR[ \t]+([0-9]+).*")
    string(REGEX REPLACE ".*define DASH_VERSION_MAJOR[ \t]+([0-9]+).*"
      "\\1" DASH_VERSION_MAJOR ${_dash_Version_contents})
    string(REGEX REPLACE ".*define DASH_VERSION_MINOR[ \t]+([0-9]+).*"
      "\\1" DASH_VERSION_MINOR ${_dash_Version_contents})
    string(REGEX REPLACE ".*define DASH_VERSION_PATCH[ \t]+([0-9]+).*"
      "\\1" DASH_VERSION_PATCH ${_dash_Version_contents})
    set(DASH_VERSION "${DASH_VERSION_MAJOR}.${DASH_VERSION_MINOR}.${DASH_VERSION_PATCH}"
      CACHE INTERNAL "The version of DASH which was detected")
  else()
    set(DASH_VERSION "0.1.0"
      CACHE INTERNAL "The version of DASH which was detected")
  endif()
else()
  set(_dash_EPIC_FAIL TRUE)
  if(_dash_output)
    message(${_dash_version_output_type} "Can't find DASH header version.h.")
  endif()
endif()

#
# Version checking
#
if(Dash_FIND_VERSION AND DASH_VERSION)
  if(Dash_FIND_VERSION_EXACT)
    if(NOT DASH_VERSION VERSION_EQUAL ${Dash_FIND_VERSION})
      set(_dash_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT DASH_VERSION VERSION_EQUAL ${Dash_FIND_VERSION} AND 
        NOT DASH_VERSION VERSION_GREATER ${Dash_FIND_VERSION})
      set(_dash_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(_dash_LIBRARY dash
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{DASH_ROOT} ${DASH_ROOT} }
  PATH_SUFFIXES lib
  PATHS /usr /usr/local /opt/local /opt
)
        
# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_dash_version_not_high_enough)
  set(_dash_EPIC_FAIL TRUE)
  if(_dash_output)
    message(${_dash_version_output_type}
      "Version ${Dash_FIND_VERSION} or higher of DASH is required. "
      "Version ${DASH_VERSION} was found in ${_dash_INCLUDE_DIR}.")
  endif()
elseif(_dash_version_not_exact)
  set(_dash_EPIC_FAIL TRUE)
  if(_dash_output)
    message(${_dash_version_output_type}
      "Version ${Dash_FIND_VERSION} of DASH is required exactly. "
      "Version ${DASH_VERSION} was found.")
  endif()
else()
  if(Dash_FIND_REQUIRED)
    if(_dash_LIBRARY MATCHES "_dash_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the DASH library.\n"
        "Consider using CMAKE_PREFIX_PATH or the DASH_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Dash DEFAULT_MSG
                                    _dash_LIBRARY _dash_INCLUDE_DIR)
endif()

if(_dash_EPIC_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(DASH_FOUND FALSE)
    set(_dash_LIBRARY)
    set(_dash_INCLUDE_DIR)
endif()

set(DASH_INCLUDE_DIRS ${_dash_INCLUDE_DIR})
set(DASH_LIBRARIES ${_dash_LIBRARY})
get_filename_component(DASH_LIBRARY_DIR ${_dash_LIBRARY} PATH)

if(DASH_FOUND AND _dash_output)
  message(STATUS "Found DASH ${DASH_VERSION} in "
    "${DASH_INCLUDE_DIRS};${DASH_LIBRARIES}")
endif()

