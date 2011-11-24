#
# Copyright 2011 Stefan Eilemann <eile@eyescale.ch>
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
# Find GPU-SD. This module searches for the GPU-SD library
#    See http://www.equalizergraphics.com/gpu-sd
#
#
#==================================
#
# The following environment variables are respected for finding GPU-SD.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    GPUSD_ROOT
#
# This module defines the following output variables:
#
#    GPUSD_FOUND - Was GPU-SD and all of the specified components found?
#
#    GPUSD_VERSION - The version of GPU-SD which was found
#
#    GPUSD_INCLUDE_DIRS - Where to find the headers
#
#    GPUSD_LIBRARIES - The GPU-SD libraries
#
# Components may be: core, cgl, glx, wgl, dns_sd
#   For each component, the following variables are set. In addition, the
#   relevent libraries are added to GPUSD_LIBRARIES. The core component is
#   implicit and always searched.
#
#   GPUSD_${COMPONENT}_FOUND - Was the component found?
#   GPUSD_${COMPONENT}_LIBRARY - The component librarys
#
#==================================
# Example Usage:
#
#  find_package(GPUSD 1.0.0 dns_sd REQUIRED)
#  include_directories(${GPUSD_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${GPUSD_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _gpusd_foo
#  Input variables of the form GPUSD_FOO
#  Output variables of the form GPUSD_FOO
#

#
# find and parse gpusd/version.h
find_path(_gpusd_INCLUDE_DIR gpusd/version.h
  HINTS ${CMAKE_SOURCE_DIR}/gpu-sd/${CMAKE_BUILD_TYPE}/install
    $ENV{GPUSD_ROOT} ${GPUSD_ROOT}
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt/local /opt
  )

if(GPUSD_FIND_REQUIRED)
    set(_gpusd_version_output_type FATAL_ERROR)
else()
    set(_gpusd_version_output_type STATUS)
endif()

# Try to ascertain the version...
if(_gpusd_INCLUDE_DIR)
  set(_gpusd_Version_file "${_gpusd_INCLUDE_DIR}/gpusd/version.h")
  if("${_gpusd_INCLUDE_DIR}" MATCHES "\\.framework$" AND
      NOT EXISTS "${_gpusd_Version_file}")
    set(_gpusd_Version_file "${_gpusd_INCLUDE_DIR}/Headers/version.h")
  endif()

  if(EXISTS "${_gpusd_Version_file}")
    file(READ "${_gpusd_Version_file}" _gpusd_Version_contents)
  else()
    set(_gpusd_Version_contents "unknown")
  endif()

  if(_gpusd_Version_contents MATCHES ".*define GPUSD_VERSION_MAJOR[ \t]+([0-9]+).*")
    string(REGEX REPLACE ".*define GPUSD_VERSION_MAJOR[ \t]+([0-9]+).*"
      "\\1" GPUSD_VERSION_MAJOR ${_gpusd_Version_contents})
    string(REGEX REPLACE ".*define GPUSD_VERSION_MINOR[ \t]+([0-9]+).*"
      "\\1" GPUSD_VERSION_MINOR ${_gpusd_Version_contents})
    string(REGEX REPLACE ".*define GPUSD_VERSION_PATCH[ \t]+([0-9]+).*"
      "\\1" GPUSD_VERSION_PATCH ${_gpusd_Version_contents})
    set(GPUSD_VERSION "${GPUSD_VERSION_MAJOR}.${GPUSD_VERSION_MINOR}.${GPUSD_VERSION_PATCH}"
      CACHE INTERNAL "The version of GPU-SD which was detected")
  else()
    set(_gpusd_EPIC_FAIL TRUE)
    message(${_gpusd_version_output_type} "Can't parse gpusd/version.h.")
  endif()
else()
  set(_gpusd_EPIC_FAIL TRUE)
  message(${_gpusd_version_output_type} "Can't find gpusd/version.h.")
endif()

# Version checking
if(GPUSD_FIND_VERSION AND GPUSD_VERSION)
  if(GPUSD_FIND_VERSION_EXACT)
    if(NOT GPUSD_VERSION VERSION_EQUAL ${GPUSD_FIND_VERSION})
      set(_gpusd_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT GPUSD_VERSION VERSION_EQUAL ${GPUSD_FIND_VERSION} AND 
        NOT GPUSD_VERSION VERSION_GREATER ${GPUSD_FIND_VERSION})
      set(_gpusd_version_not_high_enough TRUE)
    endif()
  endif()
endif()

# components
#  core
find_library(_gpusd_core_LIBRARY gpusd
  HINTS ${CMAKE_SOURCE_DIR}/gpusd/${CMAKE_BUILD_TYPE}/install
    $ENV{GPUSD_ROOT} ${GPUSD_ROOT}
  PATH_SUFFIXES lib PATHS /usr /usr/local /opt/local /opt)
set(GPUSD_LIBRARIES ${_gpusd_core_LIBRARY})

#  others
set(_gpusd_COMPONENTS cgl glx wgl dns_sd)
foreach(_gpusd_COMPONENT ${_gpusd_COMPONENTS})
  set(_gpusd_lib GPUSD_${_gpusd_COMPONENT}_LIBRARY)
  find_library(${_gpusd_lib} gpusd_${_gpusd_COMPONENT}
    HINTS ${CMAKE_SOURCE_DIR}/gpusd/${CMAKE_BUILD_TYPE}/install
    $ENV{GPUSD_ROOT} ${GPUSD_ROOT}
    PATH_SUFFIXES lib PATHS /usr /usr/local /opt/local /opt)

  if(${_gpusd_lib} MATCHES "${_gpusd_lib}-NOTFOUND")
    if(GPUSD_FIND_COMPONENTS MATCHES ${_gpusd_COMPONENT})
      message(${_gpusd_version_output_type} "${_gpusd_lib} not found")
    endif()
  else()
    set(GPUSD_${_gpusd_COMPONENT}_FOUND TRUE)
    list(APPEND GPUSD_LIBRARIES ${_gpusd_lib})
  endif()
endforeach()

# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_gpusd_version_not_high_enough)
  set(_gpusd_EPIC_FAIL TRUE)
  message(${_gpusd_version_output_type}
    "Version ${GPUSD_FIND_VERSION} or higher of GPU-SD is required. "
    "Version ${GPUSD_VERSION} was found in ${_gpusd_INCLUDE_DIR}.")
elseif(_gpusd_version_not_exact)
  set(_gpusd_EPIC_FAIL TRUE)
  message(${_gpusd_version_output_type}
    "Version ${GPUSD_FIND_VERSION} of GPU-SD is required exactly. "
    "Version ${GPUSD_VERSION} was found.")
else()
  if(GPUSD_FIND_REQUIRED)
    if(_gpusd_core_LIBRARY MATCHES "_gpusd_core_LIBRARY-NOTFOUND")
      message(${_gpusd_version_output_type}
        "ERROR: Missing the GPU-SD core library.\n"
        "Consider using CMAKE_PREFIX_PATH or the GPUSD_ROOT variable. "
        "See ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(GPUSD DEFAULT_MSG
                                    _gpusd_core_LIBRARY _gpusd_INCLUDE_DIR)
endif()

if(_gpusd_EPIC_FAIL)
  # Zero out everything, we didn't meet version requirements
  set(GPUSD_FOUND FALSE)
  set(GPUSD_LIBRARIES)
  set(_gpusd_INCLUDE_DIR)
endif()

set(GPUSD_INCLUDE_DIRS ${_gpusd_INCLUDE_DIR})
get_filename_component(GPUSD_LIBRARY_DIR ${_gpusd_core_LIBRARY} PATH)

if(GPUSD_FOUND)
  message(STATUS "Found GPU-SD ${GPUSD_VERSION} in ${GPUSD_INCLUDE_DIRS};"
    "${GPUSD_LIBRARIES}")
endif()

