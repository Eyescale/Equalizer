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
# - Find Equalizer
# This module searches for the Equalizer and Collage library
#    See http://www.equalizergraphics.com
#
#==================================
#
# The following environment variables are also respected for finding Equalizer
# and Collage.  CMAKE_PREFIX_PATH can also be used for this
# (see find_library() CMake documentation).
#
#    EQ_ROOT
#
# This module defines the following output variables:
#
#    EQUALIZER_FOUND - Was Equalizer and all of the specified components found?
#
#    EQUALIZER_VERSION - The version of Equalizer which was found
#
#    EQUALIZER_VERSION_ABI - The DSO version of Equalizer which was found
#
#    EQUALIZER_INCLUDE_DIRS - Where to find the headers
#
#    EQUALIZER_LIBRARIES - The Equalizer libraries
#
#    EQUALIZER_DEB_DEPENDENCIES - A list of dependencies for CPack deb generator
#
#==================================
# Example Usage:
#
#  find_package(Equalizer 1.0.0 REQUIRED)
#  include_directories(${EQUALIZER_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${EQUALIZER_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _eq_foo
#  Input variables of the form Equalizer_FOO
#  Output variables of the form EQUALIZER_FOO
#

# Find Collage
set(_eq_required)
if(Equalizer_FIND_REQUIRED)
  set(_eq_required REQUIRED)
endif()
if(Equalizer_FIND_QUIETLY)
  set(_eq_quiet QUIET)
endif()

# find and parse eq/client/version.h [new location]
find_path(_eq_INCLUDE_DIR eq/client/version.h
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{EQ_ROOT} ${EQ_ROOT}
  PATH_SUFFIXES include
  PATHS /usr /usr/local /opt/local /opt
  )

if(_eq_INCLUDE_DIR AND EXISTS "${_eq_INCLUDE_DIR}/eq/client/version.h")
  set(_eq_Version_file "${_eq_INCLUDE_DIR}/eq/client/version.h")
else() # find old one
  find_path(_eq_INCLUDE_DIR eq/version.h
    HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{EQ_ROOT} ${EQ_ROOT}
    PATH_SUFFIXES include
    PATHS /usr /usr/local /opt/local /opt
    )
  if(_eq_INCLUDE_DIR AND EXISTS "${_eq_INCLUDE_DIR}/eq/version.h")
    set(_eq_Version_file "${_eq_INCLUDE_DIR}/eq/version.h")
  endif()
endif()

if(Equalizer_FIND_REQUIRED)
  set(_eq_version_output_type FATAL_ERROR)
  set(_eq_output 1)
else()
  set(_eq_version_output_type STATUS)
  if(NOT Equalizer_FIND_QUIETLY)
    set(_eq_output 1)
  endif()
endif()

if(_eq_Version_file)
# Try to ascertain the version...
  if("${_eq_INCLUDE_DIR}" MATCHES "\\.framework$" AND
      NOT EXISTS "${_eq_Version_file}")
    set(_eq_Version_file "${_eq_INCLUDE_DIR}/Headers/version.h")
  endif()
    
  file(READ "${_eq_Version_file}" _eq_Version_contents)
  string(REGEX REPLACE ".*define EQ_VERSION_MAJOR[ \t]+([0-9]+).*"
    "\\1" EQUALIZER_VERSION_MAJOR ${_eq_Version_contents})
  string(REGEX REPLACE ".*define EQ_VERSION_MINOR[ \t]+([0-9]+).*"
    "\\1" EQUALIZER_VERSION_MINOR ${_eq_Version_contents})
  string(REGEX REPLACE ".*define EQ_VERSION_PATCH[ \t]+([0-9]+).*"
    "\\1" EQUALIZER_VERSION_PATCH ${_eq_Version_contents})
  string(REGEX REPLACE ".*define EQ_VERSION_ABI[ \t]+([0-9]+).*"
    "\\1" EQUALIZER_VERSION_ABI ${_eq_Version_contents})
  if(NOT EQUALIZER_VERSION_ABI GREATER 1)
    set(EQUALIZER_VERSION_ABI)
  endif()

  set(EQUALIZER_VERSION "${EQUALIZER_VERSION_MAJOR}.${EQUALIZER_VERSION_MINOR}.${EQUALIZER_VERSION_PATCH}"
    CACHE INTERNAL "The version of Equalizer which was detected")
else()
  set(_eq_EPIC_FAIL TRUE)
  if(_eq_output)
    message(${_eq_version_output_type}
      "Can't find Equalizer header file version.h.")
  endif()
endif()

# Version checking
if(Equalizer_FIND_VERSION AND EQUALIZER_VERSION)
  if(Equalizer_FIND_VERSION_EXACT)
    if(NOT EQUALIZER_VERSION VERSION_EQUAL ${Equalizer_FIND_VERSION})
      set(_eq_version_not_exact TRUE)
    endif()
  else()
    # version is too low
    if(NOT EQUALIZER_VERSION VERSION_EQUAL ${Equalizer_FIND_VERSION} AND 
        NOT EQUALIZER_VERSION VERSION_GREATER ${Equalizer_FIND_VERSION})
      set(_eq_version_not_high_enough TRUE)
    endif()
  endif()
endif()

find_library(_eq_LIBRARY Equalizer PATH_SUFFIXES lib
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{EQ_ROOT} ${EQ_ROOT}
  PATHS /usr /usr/local /opt/local /opt
)
find_library(_eq_fabric_LIBRARY EqualizerFabric PATH_SUFFIXES lib
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{EQ_ROOT} ${EQ_ROOT}
  PATHS /usr /usr/local /opt/local /opt
)
find_library(EQUALIZER_SERVER_LIBRARY EqualizerServer PATH_SUFFIXES lib
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{EQ_ROOT} ${EQ_ROOT}
  PATHS /usr /usr/local /opt/local /opt
)
find_library(EQUALIZER_ADMIN_LIBRARY EqualizerAdmin PATH_SUFFIXES lib
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{EQ_ROOT} ${EQ_ROOT}
  PATHS /usr /usr/local /opt/local /opt
)
find_library(EQUALIZER_SEQUEL_LIBRARY Sequel PATH_SUFFIXES lib
  HINTS ${CMAKE_SOURCE_DIR}/../../.. $ENV{EQ_ROOT} ${EQ_ROOT}
  PATHS /usr /usr/local /opt/local /opt
)

# Inform the users with an error message based on what version they
# have vs. what version was required.
if(_eq_version_not_high_enough)
  set(_eq_EPIC_FAIL TRUE)
  message(${_eq_version_output_type}
    "Version ${Equalizer_FIND_VERSION} or higher of Equalizer is required. "
    "Version ${EQUALIZER_VERSION} was found in ${_eq_Version_file}.")
elseif(_eq_version_not_exact)
  set(_eq_EPIC_FAIL TRUE)
  message(${_eq_version_output_type}
    "Version ${Equalizer_FIND_VERSION} of Equalizer is required exactly. "
    "Version ${EQUALIZER_VERSION} was found.")
else()
  if(Equalizer_FIND_REQUIRED)
    if(_eq_LIBRARY MATCHES "_eq_LIBRARY-NOTFOUND")
      message(FATAL_ERROR "Missing the Equalizer library.\n"
        "Consider using CMAKE_PREFIX_PATH or the EQ_ROOT environment variable. "
        "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
    endif()
  endif()
  find_package_handle_standard_args(Equalizer DEFAULT_MSG
                                    _eq_LIBRARY _eq_INCLUDE_DIR)
  # Matching Collage versions
  set(_eq_coVersion_1.3.1 "0.5.1")
  set(_eq_coVersion_1.3.0 "0.5.0")
  set(_eq_coVersion_1.2.1 "0.4.8")
  set(_eq_coVersion_1.2.0 "0.4.8")
  set(_eq_coVersion_1.1.7 "0.4.7")
  set(_eq_coVersion_1.1.6 "0.4.1")
  set(_eq_coVersion_1.1.5 "0.4.1")
  set(_eq_coVersion_1.1.4 "0.4.1")
  set(_eq_coVersion_1.1.3 "0.4.0")
  set(_eq_coVersion_1.1.2 "0.4.0")
  set(_eq_coVersion_1.1.1 "0.4.0")
  set(_eq_coVersion_1.1.0 "0.4.0")
  set(_eq_coVersion_1.0.2 "0.3.1")
  set(_eq_coVersion_1.0.1 "0.3.1")
  set(_eq_coVersion_1.0.0 "0.3.0")
  find_package(Collage "${_eq_coVersion_${EQUALIZER_VERSION}}" EXACT
               ${_eq_required} ${_eq_quiet})
  if(NOT COLLAGE_FOUND)
    set(_eq_EPIC_FAIL 1)
  endif()
endif()

if(_eq_EPIC_FAIL OR NOT COLLAGE_FOUND)
    # Zero out everything, we didn't meet version requirements
    set(EQUALIZER_FOUND FALSE)
    set(_eq_LIBRARY)
    set(_eq_fabric_LIBRARY)
    set(_eq_INCLUDE_DIR)
else()
  if(EQUALIZER_VERSION_ABI LESS 110)
    set(EQUALIZER_DEB_DEPENDENCIES "equalizer (>=${EQUALIZER_VERSION})")
  else()
    set(EQUALIZER_DEB_DEPENDENCIES "equalizer${EQUALIZER_VERSION_ABI}-eqlib")
  endif()
endif()

set(EQUALIZER_INCLUDE_DIRS ${_eq_INCLUDE_DIR})
set(EQUALIZER_LIBRARIES ${_eq_LIBRARY} ${COLLAGE_LIBRARIES})
if(EQUALIZER_VERSION VERSION_GREATER 1.0)
  set(EQUALIZER_LIBRARIES ${EQUALIZER_LIBRARIES} ${_eq_fabric_LIBRARY})
endif()
get_filename_component(EQUALIZER_LIBRARY_DIR ${_eq_LIBRARY} PATH)

if(EQUALIZER_FOUND)
  message("-- Found Equalizer ${EQUALIZER_VERSION}/${EQUALIZER_VERSION_ABI} in ${EQUALIZER_INCLUDE_DIRS}"
    ";${EQUALIZER_LIBRARIES}")
endif()
