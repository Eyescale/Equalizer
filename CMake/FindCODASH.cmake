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
# - Find CODASH
# This module searches for the CODASH library
#    See https://github.com/BlueBrain/codash
#
#==================================
#
# The following environment variables are respected for finding CODASH.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    CODASH_ROOT
#
# This module defines the following output variables:
#
#    CODASH_FOUND - Was CODASH and all of the specified components found?
#
#    CODASH_VERSION - The version of CODASH which was found
#
#    CODASH_VERSION_ABI - The DSO version of CODASH which was found
#
#    CODASH_INCLUDE_DIRS - Where to find the headers
#
#    CODASH_LIBRARIES - The CODASH libraries
#
#==================================
# Example Usage:
#
#  find_package(CODASH 0.1.0 REQUIRED)
#  include_directories(${CODASH_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${CODASH_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _codash_foo
#  Input variables of the form Dash_FOO
#  Output variables of the form CODASH_FOO
#

#
# find and parse codash/version.h
find_path(_codash_INCLUDE_DIR codash/version.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{CODASH_ROOT} ${CODASH_ROOT} }
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt/local /opt
  )

if(CoDash_FIND_REQUIRED)
  set(_codash_version_output_type FATAL_ERROR)
else()
  set(_codash_version_output_type STATUS)
endif()

# Try to ascertain the version...
if(_codash_INCLUDE_DIR)
  set(_codash_Version_file "${_codash_INCLUDE_DIR}/codash/version.h")

  if(EXISTS "${_codash_Version_file}")
    file(READ "${_codash_Version_file}" _codash_Version_contents)
  else()
    set(_codash_Version_contents "unknown")
  endif()

  if(_codash_Version_contents MATCHES ".*define CODASH_VERSION_MAJOR[ \t]+([0-9]+).*")
    string(REGEX REPLACE ".*define CODASH_VERSION_MAJOR[ \t]+([0-9]+).*"
      "\\1" CODASH_VERSION_MAJOR ${_codash_Version_contents})
    string(REGEX REPLACE ".*define CODASH_VERSION_MINOR[ \t]+([0-9]+).*"
      "\\1" CODASH_VERSION_MINOR ${_codash_Version_contents})
    string(REGEX REPLACE ".*define CODASH_VERSION_PATCH[ \t]+([0-9]+).*"
      "\\1" CODASH_VERSION_PATCH ${_codash_Version_contents})
    set(CODASH_VERSION "${CODASH_VERSION_MAJOR}.${CODASH_VERSION_MINOR}.${CODASH_VERSION_PATCH}"
      CACHE INTERNAL "The version of CODASH which was detected")
  else()
    set(CODASH_VERSION "0.1.0"
      CACHE INTERNAL "The version of CODASH which was detected")
  endif()
else()
  set(_codash_EPIC_FAIL TRUE)
  message(${_codash_version_output_type}
    "Can't find CODASH header file version.h.")
endif()

#
# Version checking
#
if(CoDash_FIND_VERSION AND CODASH_VERSION)
  if(CoDash_FIND_VERSION_EXACT)
    if(NOT CODASH_VERSION VERSION_EQUAL ${CoDash_FIND_VERSION})
      set(_codash_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT CODASH_VERSION VERSION_EQUAL ${CoDash_FIND_VERSION} AND
        NOT CODASH_VERSION VERSION_GREATER ${CoDash_FIND_VERSION})
      set(_codash_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(_codash_LIBRARY codash
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{CODASH_ROOT} ${CODASH_ROOT} }
  PATH_SUFFIXES lib
  PATHS /usr /usr/local /opt/local /opt
)
        
# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_codash_version_not_high_enough)
  set(_codash_EPIC_FAIL TRUE)
  message(${_codash_version_output_type}
    "Version ${CoDash_FIND_VERSION} or higher of CODASH is required. "
    "Version ${CODASH_VERSION} was found in ${_codash_INCLUDE_DIR}.")
elseif(_codash_version_not_exact)
  set(_codash_EPIC_FAIL TRUE)
  message(${_codash_version_output_type}
    "Version ${CoDash_FIND_VERSION} of CODASH is required exactly. "
    "Version ${CODASH_VERSION} was found.")
else()
  if(CoDash_FIND_REQUIRED)
    if(_codash_LIBRARY MATCHES "_codash_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the CODASH library.\n"
        "Consider using CMAKE_PREFIX_PATH or the CODASH_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(CoDash DEFAULT_MSG
                                    _codash_LIBRARY _codash_INCLUDE_DIR)
endif()

if(_codash_EPIC_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(CODASH_FOUND FALSE)
    set(_codash_LIBRARY)
    set(_codash_INCLUDE_DIR)
endif()

set(CODASH_INCLUDE_DIRS ${_codash_INCLUDE_DIR})
set(CODASH_LIBRARIES ${_codash_LIBRARY})
get_filename_component(CODASH_LIBRARY_DIR ${_codash_LIBRARY} PATH)

if(CODASH_FOUND)
  message(STATUS "Found CODASH ${CODASH_VERSION} in "
    "${CODASH_INCLUDE_DIRS};${CODASH_LIBRARIES}")
endif()
