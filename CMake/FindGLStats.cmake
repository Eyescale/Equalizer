#
# Copyright 2011-2012 Stefan Eilemann <eile@eyescale.ch>
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
# Find GLStats. This module searches for the GLStats library
#    See http://www.equalizergraphics.com/GLStats
#
#
#==================================
#
# The following environment variables are respected for finding GLStats.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    GLSTATS_ROOT
#
# This module defines the following output variables:
#
#    GLSTATS_FOUND - Was GLStats and all of the specified components found?
#
#    GLSTATS_VERSION - The version of GLStats which was found
#
#    GLSTATS_INCLUDE_DIRS - Where to find the headers
#
#    GLSTATS_LIBRARIES - The GLStats libraries
#
#    GLSTATS_COMPONENTS - A list of components found
#
#    GLSTATS_DEB_DEPENDENCIES - A list of dependencies for the CPack deb generator
#
# Components may be: core, cgl, glx, wgl, dns_sd
#   For each component, the following variables are set. In addition, the
#   relevent libraries are added to GLSTATS_LIBRARIES. The core component is
#   implicit and always searched.
#
#   GLSTATS_${COMPONENT}_FOUND - Was the component found?
#   GLSTATS_${COMPONENT}_LIBRARY - The component librarys
#
#==================================
# Example Usage:
#
#  find_package(GLSTATS 1.0.0 dns_sd REQUIRED)
#  include_directories(${GLSTATS_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${GLSTATS_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _glstats_foo
#  Input variables of the form GLSTATS_FOO
#  Output variables of the form GLSTATS_FOO
#

#
# find and parse GLStats/version.h
find_path(_glstats_INCLUDE_DIR GLStats/version.h
  HINTS "${GLSTATS_ROOT}/include" "$ENV{GLSTATS_ROOT}/include"
  PATHS /usr/include /usr/local/include /opt/local/include /opt/include)

if(GLSTATS_FIND_REQUIRED)
  set(_glstats_version_output_type FATAL_ERROR)
  set(_glstats_output 1)
else()
  set(_glstats_version_output_type STATUS)
  if(NOT GLSTATS_FIND_QUIETLY)
    set(_glstats_output 1)
  endif()
endif()

# Try to ascertain the version...
if(_glstats_INCLUDE_DIR)
  set(_glstats_Version_file "${_glstats_INCLUDE_DIR}/GLStats/version.h")
  if("${_glstats_INCLUDE_DIR}" MATCHES "\\.framework$" AND
      NOT EXISTS "${_glstats_Version_file}")
    set(_glstats_Version_file "${_glstats_INCLUDE_DIR}/Headers/version.h")
  endif()

  if(EXISTS "${_glstats_Version_file}")
    file(READ "${_glstats_Version_file}" _glstats_Version_contents)
  else()
    set(_glstats_Version_contents "unknown")
  endif()

  if(_glstats_Version_contents MATCHES ".*define GLSTATS_VERSION_MAJOR[ \t]+([0-9]+).*")
    string(REGEX REPLACE ".*define GLSTATS_VERSION_MAJOR[ \t]+([0-9]+).*"
      "\\1" GLSTATS_VERSION_MAJOR ${_glstats_Version_contents})
    string(REGEX REPLACE ".*define GLSTATS_VERSION_MINOR[ \t]+([0-9]+).*"
      "\\1" GLSTATS_VERSION_MINOR ${_glstats_Version_contents})
    string(REGEX REPLACE ".*define GLSTATS_VERSION_PATCH[ \t]+([0-9]+).*"
      "\\1" GLSTATS_VERSION_PATCH ${_glstats_Version_contents})
    set(GLSTATS_VERSION "${GLSTATS_VERSION_MAJOR}.${GLSTATS_VERSION_MINOR}.${GLSTATS_VERSION_PATCH}"
      CACHE INTERNAL "The version of GLStats which was detected")
  else()
    set(_glstats_EPIC_FAIL TRUE)
    if(_glstats_output)
      message(${_glstats_version_output_type} "Can't parse GLStats/version.h.")
    endif()
  endif()
else()
  set(_glstats_EPIC_FAIL TRUE)
  if(_glstats_output)
    message(${_glstats_version_output_type} "Can't find GLStats/version.h.")
  endif()
endif()

# Version checking
if(GLSTATS_FIND_VERSION AND GLSTATS_VERSION)
  if(GLSTATS_FIND_VERSION_EXACT)
    if(NOT GLSTATS_VERSION VERSION_EQUAL ${GLSTATS_FIND_VERSION})
      set(_glstats_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT GLSTATS_VERSION VERSION_EQUAL ${GLSTATS_FIND_VERSION} AND 
        NOT GLSTATS_VERSION VERSION_GREATER ${GLSTATS_FIND_VERSION})
      set(_glstats_version_not_high_enough TRUE)
    endif()
  endif()
endif()

# include
set(GLSTATS_INCLUDE_DIRS ${_glstats_INCLUDE_DIR})

find_library(GLSTATS_LIBRARY GLStats
  PATHS ${_glstats_INCLUDE_DIR}/.. PATH_SUFFIXES lib NO_DEFAULT_PATH)

set(GLSTATS_LIBRARIES ${GLSTATS_LIBRARY})

# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_glstats_version_not_high_enough)
  set(_glstats_EPIC_FAIL TRUE)
  if(_glstats_output)
    message(${_glstats_version_output_type}
      "Version ${GLSTATS_FIND_VERSION} or higher of GLStats is required. "
      "Version ${GLSTATS_VERSION} was found in ${_glstats_INCLUDE_DIR}.")
  endif()
elseif(_glstats_version_not_exact)
  set(_glstats_EPIC_FAIL TRUE)
  if(_glstats_output)
    message(${_glstats_version_output_type}
      "Version ${GLSTATS_FIND_VERSION} of GLStats is required exactly. "
      "Version ${GLSTATS_VERSION} was found.")
  endif()
else()
  if(GLSTATS_FIND_REQUIRED)
    if(GLSTATS_core_LIBRARY MATCHES "GLSTATS_core_LIBRARY-NOTFOUND")
      set(_glstats_EPIC_FAIL TRUE)
      if(_glstats_output)
        message(${_glstats_version_output_type}
          "ERROR: Missing the GLStats core library.\n"
          "Consider using CMAKE_PREFIX_PATH or the GLSTATS_ROOT variable. "
          "See ${CMAKE_CURRENT_LIST_FILE} for more details.")
      endif()
    endif()
  endif()
endif()

if(_glstats_EPIC_FAIL)
  # Zero out everything, we didn't meet version requirements
  set(GLSTATS_FOUND FALSE)
  set(GLSTATS_LIBRARIES)
  set(GLSTATS_INCLUDE_DIRS)
else()
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLSTATS DEFAULT_MSG
                                    GLSTATS_LIBRARY _glstats_INCLUDE_DIR)
endif()

if(GLSTATS_FOUND AND _glstats_output)
  message(STATUS "Found GLStats ${GLSTATS_VERSION} in ${GLSTATS_INCLUDE_DIRS}:"
    "${GLSTATS_LIBRARIES}")
endif()

