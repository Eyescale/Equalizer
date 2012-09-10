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
# find_library_package(name [INCLUDE include_prefix] [TRANSIENT package])
#
# The default INCLUDE prefix is the package name, which may be
# overwritten if the package name differs from the include prefix,
# e.g., due to capitalization.
#
# TRANSIENT packages are packages which are exposed through the API of
# the package, that is, packages which a user of this package needs to
# include and link. It does not concern packages which are only used
# internally. The version of the transient package is parsed from
# version.h, e.g., "DASH_LUNCHBOX_VERSION M.m.p"
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
  set(_flp_${name}_FAIL)
  set(_flp_${name}_REQ)
  set(_flp_${name}_QUIET)
  set(_flp_${name}_out)
  string(TOUPPER ${name} _flp_${name}_UC)

  # options
  set(oneValueArgs INCLUDE)
  set(multiValueArgs TRANSIENT)
  cmake_parse_arguments(_flp_${name} "" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN})
  if(NOT _flp_${name}_INCLUDE)
    set(_flp_${name}_INCLUDE ${name})
  endif()
  string(TOUPPER ${_flp_${name}_INCLUDE} _flp_${name}_INCLUDE_UC)

  # might already be set from a previous run with a different/wrong version
  unset(${_flp_NAME}_INCLUDE_DIR CACHE)

  find_path(${_flp_${name}_UC}_INCLUDE_DIR ${_flp_${name}_INCLUDE}/version.h
    HINTS "${${_flp_${name}_UC}_ROOT}/include" "$ENV{${_flp_${name}_UC}_ROOT}/include"
    PATHS /usr/include /usr/local/include /opt/local/include /opt/include)

  if(${name}_FIND_REQUIRED)
    set(_flp_version_output_type FATAL_ERROR)
    set(_flp_${name}_out 1)
    set(_flp_${name}_REQ REQUIRED)
  else()
    set(_flp_version_output_type STATUS)
    if(NOT ${name}_FIND_QUIETLY)
      set(_flp_${name}_out 1)
    endif()
  endif()
  if(${name}_FIND_QUIETLY)
    set(_flp_${name}_QUIET QUIET)
  endif()

  # Try to ascertain the version...
  if(${_flp_${name}_UC}_INCLUDE_DIR)
    set(_flp_Version_file "${${_flp_${name}_UC}_INCLUDE_DIR}/${_flp_${name}_INCLUDE}/version.h")
    if("${${_flp_${name}_UC}_INCLUDE_DIR}" MATCHES "\\.framework$" AND
        NOT EXISTS "${_flp_Version_file}")
      set(_flp_Version_file "${${_flp_${name}_UC}_INCLUDE_DIR}/Headers/version.h")
    endif()

    if(EXISTS "${_flp_Version_file}")
      file(READ "${_flp_Version_file}" _flp_${name}_Version_contents)
    else()
      set(_flp_${name}_Version_contents "unknown")
    endif()

    # parse version out of version.h
    string(REGEX MATCH "define[ \t]+${_flp_${name}_UC}_VERSION_MAJOR[ \t]+[0-9]+"
      ${_flp_${name}_UC}_VERSION_MAJOR ${_flp_${name}_Version_contents})
    string(REGEX MATCH "define[ \t]+${_flp_${name}_UC}_VERSION_MINOR[ \t]+[0-9]+"
      ${_flp_${name}_UC}_VERSION_MINOR ${_flp_${name}_Version_contents})
    string(REGEX MATCH "define[ \t]+${_flp_${name}_UC}_VERSION_PATCH[ \t]+[0-9]+"
      ${_flp_${name}_UC}_VERSION_PATCH ${_flp_${name}_Version_contents})
    string(REGEX MATCH "define[ \t]+${_flp_${name}_UC}_VERSION_ABI[ \t]+[0-9]+"
      ${_flp_${name}_UC}_VERSION_ABI ${_flp_${name}_Version_contents})

    if("${${_flp_${name}_UC}_VERSION_MAJOR}" STREQUAL "") # Try 'include' naming
      string(REGEX MATCH
        "define[ \t]+${_flp_${name}_INCLUDE_UC}_VERSION_MAJOR[ \t]+[0-9]+"
        ${_flp_${name}_UC}_VERSION_MAJOR ${_flp_${name}_Version_contents})
      string(REGEX MATCH
        "define[ \t]+${_flp_${name}_INCLUDE_UC}_VERSION_MINOR[ \t]+[0-9]+"
        ${_flp_${name}_UC}_VERSION_MINOR ${_flp_${name}_Version_contents})
      string(REGEX MATCH
        "define[ \t]+${_flp_${name}_INCLUDE_UC}_VERSION_PATCH[ \t]+[0-9]+"
        ${_flp_${name}_UC}_VERSION_PATCH ${_flp_${name}_Version_contents})
      string(REGEX MATCH
        "define[ \t]+${_flp_${name}_INCLUDE_UC}_VERSION_ABI[ \t]+[0-9]+"
        ${_flp_${name}_UC}_VERSION_ABI ${_flp_${name}_Version_contents})
    endif()

    if("${${_flp_${name}_UC}_VERSION_MAJOR}" STREQUAL "")
      set(_flp_${name}_FAIL TRUE)
      if(_flp_${name}_out)
        message(${_flp_version_output_type} "Can't parse ${_flp_Version_file}.")
      endif()
    else()
      string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_${name}_UC}_VERSION_MAJOR
        ${${_flp_${name}_UC}_VERSION_MAJOR})

      if("${${_flp_${name}_UC}_VERSION_MINOR}" STREQUAL "")
        set(${_flp_${name}_UC}_VERSION_MINOR 0)
      else()
        string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_${name}_UC}_VERSION_MINOR
          ${${_flp_${name}_UC}_VERSION_MINOR})
      endif()
      if("${${_flp_${name}_UC}_VERSION_PATCH}" STREQUAL "")
        set(${_flp_${name}_UC}_VERSION_PATCH 0)
      else()
        string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_${name}_UC}_VERSION_PATCH
          ${${_flp_${name}_UC}_VERSION_PATCH})
      endif()
      if("${${_flp_${name}_UC}_VERSION_ABI}" STREQUAL "")
        set(${_flp_${name}_UC}_VERSION_ABI 0)
      else()
        string(REGEX REPLACE ".*([0-9]+)" "\\1" ${_flp_${name}_UC}_VERSION_ABI
          ${${_flp_${name}_UC}_VERSION_ABI})
      endif()

      set(${_flp_${name}_UC}_VERSION "${${_flp_${name}_UC}_VERSION_MAJOR}.${${_flp_${name}_UC}_VERSION_MINOR}.${${_flp_${name}_UC}_VERSION_PATCH}")
    endif()

    # Find transient packages
    foreach(_flp_trans ${_flp_${name}_TRANSIENT})
      string(TOUPPER ${_flp_trans} _flp_${name}_TRANS)

      # search 'COLLAGE_LUNCHBOX_VERSION'
      string(REGEX MATCH
        "define[ \t]+${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION[ \t]+[0-9.]+"
        ${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION
        ${_flp_${name}_Version_contents})

      # not found -> search 'CO_LUNCHBOX_VERSION'
      if("${${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION}" STREQUAL "")
        string(REGEX MATCH
          "define[ \t]+${_flp_${name}_INCLUDE_UC}_${_flp_${name}_TRANS}_VERSION[ \t]+[0-9.]+"
          ${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION
          ${_flp_${name}_Version_contents})
      endif()

      # not found -> use _Collage_Lunchbox_version_${CO_VERSION}
      if("${${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION}" STREQUAL "")
        set(${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION
          ${_${name}_${_flp_trans}_version_${${_flp_${name}_UC}_VERSION}})
      endif()

      if(${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION)
        string(REGEX REPLACE ".*[ \t]([0-9.]+)" "\\1"
          ${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION
          ${${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION})
        find_package(${_flp_trans}
          ${${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION}
          EXACT ${_flp_${name}_REQ} ${_flp_${name}_QUIET})
        if(NOT ${_flp_${name}_TRANS}_FOUND)
          set(_flp_${name}_FAIL TRUE)
        endif()
      else()
        message(STATUS
          "Can't figure out ${_flp_trans} version for "
          "${name} ${${_flp_${name}_UC}_VERSION}, use "
          "${_flp_${name}_UC}_${_flp_${name}_TRANS}_VERSION in "
          "${_flp_${name}_INCLUDE}/version.h.")
      endif()
    endforeach()
  else()
    set(_flp_${name}_FAIL TRUE)
    if(_flp_${name}_out)
      message(${_flp_version_output_type}
        "Can't find ${_flp_${name}_INCLUDE}/version.h.")
    endif()
  endif()

  # Version checking
  if(${_flp_${name}_UC}_VERSION AND ${name}_FIND_VERSION)
    if(${name}_FIND_VERSION_EXACT)
      if(NOT ${name}_FIND_VERSION VERSION_EQUAL ${_flp_${name}_UC}_VERSION)
        set(_flp_${name}_FAIL TRUE)
        if(_flp_${name}_out)
          message(${_flp_version_output_type}
            "Version ${${name}_FIND_VERSION} of ${name} is required exactly. "
            "Version ${${_flp_${name}_UC}_VERSION} was found.")
        endif()
      endif()
    else()
      if( ${name}_FIND_VERSION VERSION_GREATER ${_flp_${name}_UC}_VERSION )
        set(_flp_${name}_FAIL TRUE)
        if(_flp_${name}_out)
          message(${_flp_version_output_type}
            "Version ${${name}_FIND_VERSION} or higher of ${name} is required. "
            "Version ${${_flp_${name}_UC}_VERSION} was found in ${${_flp_${name}_UC}_INCLUDE_DIR}.")
        endif()
      endif()
    endif()
  endif()

  # include
  set(${_flp_${name}_UC}_INCLUDE_DIRS ${${_flp_${name}_UC}_INCLUDE_DIR})

  # library
  find_library(${_flp_${name}_UC}_LIBRARY ${name}
    PATHS ${${_flp_${name}_UC}_INCLUDE_DIR}/.. PATH_SUFFIXES lib NO_DEFAULT_PATH)
  set(${_flp_${name}_UC}_LIBRARIES ${${_flp_${name}_UC}_LIBRARY})

  if(${name}_FIND_REQUIRED)
    if(${_flp_${name}_UC}_LIBRARY MATCHES "${_flp_${name}_UC}_LIBRARY-NOTFOUND")
      set(_flp_${name}_FAIL TRUE)
      if(_flp_${name}_out)
        message(${_flp_version_output_type}
          "Missing the ${name} library in ${${_flp_${name}_UC}_INCLUDE_DIR}/../lib.")
      endif()
    endif()
  endif()

  if(_flp_${name}_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(${_flp_${name}_UC}_FOUND)
    set(${_flp_${name}_UC}_LIBRARIES)
    set(${_flp_${name}_UC}_INCLUDE_DIRS)
  else()
    string(TOLOWER ${name} _flp_name)

    set(${_flp_${name}_UC}_FOUND TRUE)
    set(${_flp_${name}_UC}_DEB_DEPENDENCIES
      "${_flp_name}${${_flp_${name}_UC}_VERSION_ABI}-lib")
    set(${_flp_${name}_UC}_DEB_BUILD_DEPENDENCIES
      "${_flp_name}${${_flp_${name}_UC}_VERSION_ABI}-dev")
    get_filename_component(${_flp_${name}_UC}_LIBRARY_DIRS
      ${${_flp_${name}_UC}_LIBRARY} PATH)

    # Add transient package information to self
    foreach(_flp_trans ${_flp_${name}_TRANSIENT})
      string(TOUPPER ${_flp_trans} _flp_TRANS)
      if(${_flp_TRANS}_FOUND)
        list(APPEND
          ${_flp_${name}_UC}_INCLUDE_DIRS ${${_flp_TRANS}_INCLUDE_DIRS})
        list(APPEND
          ${_flp_${name}_UC}_LIBRARY_DIRS ${${_flp_TRANS}_LIBRARY_DIRS})
        list(APPEND ${_flp_${name}_UC}_LIBRARIES ${${_flp_TRANS}_LIBRARIES})
      endif()
    endforeach()

    if(_flp_${name}_out)
      message(STATUS "Found ${name} ${${_flp_${name}_UC}_VERSION} in "
        "${${_flp_${name}_UC}_INCLUDE_DIRS}:${${_flp_${name}_UC}_LIBRARIES}")
    endif()
  endif()
endmacro()
