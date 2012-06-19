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
# - Find Collage
# This module searches for the Collage library
#    See http://www.libcollage.net
#
#==================================
#
# The following environment variables are respected for finding Collage.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    CO_ROOT
#    EQ_ROOT
#
# This module defines the following output variables:
#
#    COLLAGE_FOUND - Was Collage and all of the specified components found?
#
#    COLLAGE_VERSION - The version of Collage which was found
#
#    COLLAGE_VERSION_ABI - The DSO version of Collage which was found
#
#    COLLAGE_INCLUDE_DIRS - Where to find the headers
#
#    COLLAGE_LIBRARIES - The Collage libraries
#
#==================================
# Example Usage:
#
#  find_package(Collage 0.3.0 REQUIRED)
#  include_directories(${COLLAGE_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${COLLAGE_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _co_foo
#  Input variables of the form Collage_FOO
#  Output variables of the form COLLAGE_FOO
#

#
# find and parse co/version.h
find_path(_co_INCLUDE_DIR co/version.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{CO_ROOT} $ENV{EQ_ROOT} ${CO_ROOT} ${EQ_ROOT}
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt/local /opt
  )

if(Collage_FIND_REQUIRED)
  set(_co_version_output_type FATAL_ERROR)
  set(_co_output 1)
  set(_co_required REQUIRED)
else()
  set(_co_version_output_type STATUS)
  if(NOT Collage_FIND_QUIETLY)
    set(_co_output 1)
  endif()
endif()
if(Collage_FIND_QUIETLY)
  set(_co_quiet QUIET)
endif()

# Try to ascertain the version...
if(_co_INCLUDE_DIR)
  set(_co_Version_file "${_co_INCLUDE_DIR}/co/version.h")
  if("${_co_INCLUDE_DIR}" MATCHES "\\.framework$" AND
      NOT EXISTS "${_co_Version_file}")
    set(_co_Version_file "${_co_INCLUDE_DIR}/Headers/version.h")
  endif()

  if(EXISTS "${_co_Version_file}")
    file(READ "${_co_Version_file}" _co_Version_contents)
  else()
    set(_co_Version_contents "unknown")
  endif()

  if(_co_Version_contents MATCHES ".*define CO_VERSION_MAJOR[ \t]+([0-9]+).*")
    string(REGEX REPLACE ".*define CO_VERSION_MAJOR[ \t]+([0-9]+).*"
      "\\1" COLLAGE_VERSION_MAJOR ${_co_Version_contents})
    string(REGEX REPLACE ".*define CO_VERSION_MINOR[ \t]+([0-9]+).*"
      "\\1" COLLAGE_VERSION_MINOR ${_co_Version_contents})
    string(REGEX REPLACE ".*define CO_VERSION_PATCH[ \t]+([0-9]+).*"
      "\\1" COLLAGE_VERSION_PATCH ${_co_Version_contents})
    string(REGEX REPLACE ".*define CO_VERSION_ABI[ \t]+([0-9]+).*"
      "\\1" COLLAGE_VERSION_ABI ${_co_Version_contents})
    set(COLLAGE_VERSION "${COLLAGE_VERSION_MAJOR}.${COLLAGE_VERSION_MINOR}.${COLLAGE_VERSION_PATCH}"
      CACHE INTERNAL "The version of Collage which was detected")
  else()
    set(COLLAGE_VERSION "0.3.0"
      CACHE INTERNAL "The version of Collage which was detected")
  endif()
else()
  set(_co_EPIC_FAIL TRUE)
  if(_co_output)
    message(${_co_version_output_type}
      "Can't find Collage header file version.h.")
  endif()
endif()

#
# Version checking
#
if(Collage_FIND_VERSION AND COLLAGE_VERSION)
  if(Collage_FIND_VERSION_EXACT)
    if(NOT COLLAGE_VERSION VERSION_EQUAL ${Collage_FIND_VERSION})
      set(_co_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT COLLAGE_VERSION VERSION_EQUAL ${Collage_FIND_VERSION} AND 
        NOT COLLAGE_VERSION VERSION_GREATER ${Collage_FIND_VERSION})
      set(_co_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(_co_LIBRARY Collage
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{CO_ROOT} $ENV{EQ_ROOT} ${CO_ROOT} ${EQ_ROOT}
  PATH_SUFFIXES lib
  PATHS /usr /usr/local /opt/local /opt
)
        
# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_co_version_not_high_enough)
  set(_co_EPIC_FAIL TRUE)
  if(_co_output)
    message(${_co_version_output_type}
      "Version ${Collage_FIND_VERSION} or higher of Collage is required. "
      "Version ${COLLAGE_VERSION} was found in ${_co_INCLUDE_DIR}.")
  endif()
elseif(_co_version_not_exact)
  set(_co_EPIC_FAIL TRUE)
  if(_co_output)
    message(${_co_version_output_type}
      "Version ${Collage_FIND_VERSION} of Collage is required exactly. "
      "Version ${COLLAGE_VERSION} was found.")
  endif()
else()
  if(Collage_FIND_REQUIRED)
    if(_co_LIBRARY MATCHES "_co_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the Collage library.\n"
        "Consider using CMAKE_PREFIX_PATH or the CO_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Collage DEFAULT_MSG
                                    _co_LIBRARY _co_INCLUDE_DIR)

  if(COLLAGE_VERSION VERSION_GREATER 0.5) # need Lunchbox
    set(_co_lbVersion_0.5.1 "0.9.0")
    set(_co_lbVersion_0.5.2 "1.3.5")
    set(_co_lbVersion_0.5.5 "1.3.5")
    find_package(Lunchbox ${_co_lbVersion_${COLLAGE_VERSION}} EXACT
      ${_co_required} ${_co_quiet})
    if(NOT LUNCHBOX_FOUND)
      set(_co_EPIC_FAIL 1)
    endif()
  endif()
endif()

if(_co_EPIC_FAIL)
  # Zero out everything, we didn't meet version requirements
  set(COLLAGE_FOUND FALSE)
  set(_co_LIBRARY)
  set(_co_INCLUDE_DIR)
endif()

set(COLLAGE_INCLUDE_DIRS ${_co_INCLUDE_DIR} ${LUNCHBOX_INCLUDE_DIRS})
set(COLLAGE_LIBRARIES ${_co_LIBRARY} ${LUNCHBOX_LIBRARIES})
get_filename_component(COLLAGE_LIBRARY_DIR ${_co_LIBRARY} PATH)

if(LUNCHBOX_FOUND)
  list(APPEND COLLAGE_INCLUDE_DIRS ${LUNCHBOX_INCLUDE_DIRS})
  list(APPEND COLLAGE_LIBRARIES  ${LUNCHBOX_LIBRARIES})
endif()

if(COLLAGE_FOUND AND _co_output)
  message(STATUS "Found Collage ${COLLAGE_VERSION}/${COLLAGE_VERSION_ABI} in "
    "${COLLAGE_INCLUDE_DIRS};${COLLAGE_LIBRARIES}")
endif()

