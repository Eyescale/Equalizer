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
# Module to find a standard library and includes. Intended to be used by
# FindFoo.cmake scripts. Assumes the following layout:
#   (root)/include/(name)/version.h
#      parsed for defines: NAME_VERSION_MAJOR, -_MINOR, -_PATCH, -_ABI
#
#
#==================================
#
# Invocation:
#
# find_library_package(name [INCLUDE include_prefix])
#
# The following CMAKE variables are respected for finding the package.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation):
#
#    NAME_ROOT, $ENV{NAME_ROOT}
#    name_FIND_REQUIRED
#    name_FIND_QUIETLY
#    name_FIND_VERSION
#    name_FIND_VERSION_EXACT
#
# This macro defines the following output variables:
#
#    NAME_FOUND - Was library and headers found?
#
#    NAME_VERSION - The version found
#
#    NAME_INCLUDE_DIRS - Where to find the headers
#
#    NAME_LIBRARIES - The libraries
#
#    NAME_LIBRARY_DIRS - Where to find the libraries
#
#==================================
# Example Usage:
#
#  find_library_package(Lunchbox VERSION 1.0.0 REQUIRED)
#  find_package_handle_standard_args(Lunchbox DEFAULT_MSG
#                                    LUNCHBOX_LIBRARIES LUNCHBOX_INCLUDE_DIRS)
#
#==================================
# Naming convention:
#  Local variables of the form _flp_foo

#
# find and parse name/version.h
include(CMakeParseArguments)

macro(FIND_LIBRARY_PACKAGE name)
  # reset internal variables
  set(_flp_EPIC_FAIL)
  set(_flp_REQUIRED)
  set(_flp_QUIET)
  set(_flp_output)
  string(TOUPPER ${name} _flp_NAME)

  # options
  set(oneValueArgs INCLUDE)
  cmake_parse_arguments(_flp "" "${oneValueArgs}" "" ${ARGN} )
  if(NOT _flp_INCLUDE)
    set(_flp_INCLUDE ${name})
  endif()

  find_path(${_flp_NAME}_INCLUDE_DIR ${_flp_INCLUDE}/version.h
    HINTS "${${_flp_NAME}_ROOT}/include" "$ENV{${_flp_NAME}_ROOT}/include"
    PATHS /usr/include /usr/local/include /opt/local/include /opt/include)

  if(${name}_FIND_REQUIRED)
    set(_flp_version_output_type FATAL_ERROR)
    set(_flp_output 1)
  else()
    set(_flp_version_output_type STATUS)
    if(NOT ${name}_FIND_QUIETLY)
      set(_flp_output 1)
    endif()
  endif()

  # Try to ascertain the version...
  if(${_flp_NAME}_INCLUDE_DIR)
    set(_flp_Version_file "${${_flp_NAME}_INCLUDE_DIR}/${_flp_INCLUDE}/version.h")
    if("${${_flp_NAME}_INCLUDE_DIR}" MATCHES "\\.framework$" AND
        NOT EXISTS "${_flp_Version_file}")
      set(_flp_Version_file "${${_flp_NAME}_INCLUDE_DIR}/Headers/version.h")
    endif()

    if(EXISTS "${_flp_Version_file}")
      file(READ "${_flp_Version_file}" _flp_Version_contents)
    else()
      set(_flp_Version_contents "unknown")
    endif()

    string(REGEX MATCH "define[ \t]+${_flp_NAME}_VERSION_MAJOR[ \t]+[0-9]+"
      ${_flp_NAME}_VERSION_MAJOR ${_flp_Version_contents})
    string(REGEX MATCH "define[ \t]+${_flp_NAME}_VERSION_MINOR[ \t]+[0-9]+"
      ${_flp_NAME}_VERSION_MINOR ${_flp_Version_contents})
    string(REGEX MATCH "define[ \t]+${_flp_NAME}_VERSION_PATCH[ \t]+[0-9]+"
      ${_flp_NAME}_VERSION_PATCH ${_flp_Version_contents})
    string(REGEX MATCH "define[ \t]+${_flp_NAME}_VERSION_ABI[ \t]+[0-9]+"
      ${_flp_NAME}_VERSION_ABI ${_flp_Version_contents})

    if("${${_flp_NAME}_VERSION_MAJOR}" STREQUAL "")
      set(_flp_EPIC_FAIL TRUE)
      if(_flp_output)
        message(${_flp_version_output_type} "Can't parse ${_flp_Version_file}.")
      endif()
    else()
      string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_NAME}_VERSION_MAJOR
        ${${_flp_NAME}_VERSION_MAJOR})

      if("${${_flp_NAME}_VERSION_MINOR}" STREQUAL "")
        set(${_flp_NAME}_VERSION_MINOR 0)
      else()
        string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_NAME}_VERSION_MINOR
          ${${_flp_NAME}_VERSION_MINOR})
      endif()
      if("${${_flp_NAME}_VERSION_PATCH}" STREQUAL "")
        set(${_flp_NAME}_VERSION_PATCH 0)
      else()
        string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_NAME}_VERSION_PATCH
          ${${_flp_NAME}_VERSION_PATCH})
      endif()
      if("${${_flp_NAME}_VERSION_ABI}" STREQUAL "")
        set(${_flp_NAME}_VERSION_ABI 0)
      else()
        string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_NAME}_VERSION_ABI
          ${${_flp_NAME}_VERSION_ABI})
      endif()

      set(${_flp_NAME}_VERSION "${${_flp_NAME}_VERSION_MAJOR}.${${_flp_NAME}_VERSION_MINOR}.${${_flp_NAME}_VERSION_PATCH}")
    endif()
  else()
    set(_flp_EPIC_FAIL TRUE)
    if(_flp_output)
      message(${_flp_version_output_type}
        "Can't find ${_flp_INCLUDE}/version.h.")
    endif()
  endif()

  # Version checking
  if(${_flp_NAME}_VERSION AND ${name}_FIND_VERSION)
    if(${name}_FIND_VERSION_EXACT)
      if(NOT ${name}_FIND_VERSION VERSION_EQUAL ${_flp_NAME}_VERSION)
        set(_flp_EPIC_FAIL TRUE)
        if(_flp_output)
          message(${_flp_version_output_type}
            "Version ${${name}_FIND_VERSION} of ${name} is required exactly. "
            "Version ${${_flp_NAME}_VERSION} was found.")
        endif()
      endif()
    else()
      if( ${name}_FIND_VERSION VERSION_GREATER ${_flp_NAME}_VERSION )
        set(_flp_EPIC_FAIL TRUE)
        if(_flp_output)
          message(${_flp_version_output_type}
            "Version ${${name}_FIND_VERSION} or higher of ${name} is required. "
            "Version ${${_flp_NAME}_VERSION} was found in ${${_flp_NAME}_INCLUDE_DIR}.")
        endif()
      endif()
    endif()
  endif()

  # include
  set(${_flp_NAME}_INCLUDE_DIRS ${${_flp_NAME}_INCLUDE_DIR})

  # library
  find_library(${_flp_NAME}_LIBRARY ${name}
    PATHS ${${_flp_NAME}_INCLUDE_DIR}/.. PATH_SUFFIXES lib NO_DEFAULT_PATH)
  set(${_flp_NAME}_LIBRARIES ${${_flp_NAME}_LIBRARY})

  if(${name}_FIND_REQUIRED)
    if(${_flp_NAME}_LIBRARY MATCHES "${_flp_NAME}_LIBRARY-NOTFOUND")
      set(_flp_EPIC_FAIL TRUE)
      if(_flp_output)
        message(${_flp_version_output_type}
          "Missing the ${name} library in ${${_flp_NAME}_INCLUDE_DIR}/../lib.")
      endif()
    endif()
  endif()

  if(_flp_EPIC_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(${_flp_NAME}_FOUND)
    set(${_flp_NAME}_LIBRARIES)
    set(${_flp_NAME}_INCLUDE_DIRS)
  else()
    set(${_flp_NAME}_FOUND TRUE)
    set(${_flp_NAME}_DEB_DEPENDENCIES "${name}${${_flp_NAME}_VERSION_ABI}-lib")
    set(${_flp_NAME}_DEB_BUILD_DEPENDENCIES
      "${name}${${_flp_NAME}_VERSION_ABI}-dev")
    get_filename_component(${_flp_NAME}_LIBRARY_DIRS ${${_flp_NAME}_LIBRARY}
      PATH)

    if(_flp_output)
      message(STATUS "Found ${name} ${${_flp_NAME}_VERSION} in "
        "${${_flp_NAME}_INCLUDE_DIRS}:${${_flp_NAME}_LIBRARIES}")
    endif()
  endif()
endmacro()
