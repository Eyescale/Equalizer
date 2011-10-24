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
# - Find ZeroConf headers
#
#==================================
#
# The following environment variables are respected for finding Dnssd.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    DNSSD_ROOT
#
# This module defines the following output variables:
#
#    DNSSD_FOUND - Was Dnssd and all of the specified components found?
#
#    DNSSD_INCLUDE_DIRS - Where to find the headers
#
#    DNSSD_LIBRARIES - The Dnssd libraries
#
#==================================
# Example Usage:
#
#  find_package(DNSSD REQUIRED)
#  include_directories(${DNSSD_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${DNSSD_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _dnssd_foo
#  Output variables of the form DNSSD_FOO
#

find_path(_dnssd_INCLUDE_DIR dns_sd.h
  HINTS $ENV{DNSSD_ROOT} ${DNSSD_ROOT}
  PATH_SUFFIXES include PATHS /usr /usr/local /opt/local /opt
  )

if(DNSSD_FIND_REQUIRED)
    set(_dnssd_output_type FATAL_ERROR)
else()
    set(_dnssd_output_type STATUS)
endif()

if(NOT _dnssd_INCLUDE_DIR)
  set(_dnssd_EPIC_FAIL TRUE)
  message(${_dnssd_output_type} "Can't find dns_sd.h header file.")
endif()

if(APPLE)
  find_library(_dnssd_LIBRARY System
    HINTS $ENV{DNSSD_ROOT} ${DNSSD_ROOT}
    PATH_SUFFIXES lib PATHS /usr /usr/local /opt/local /opt
    )
else()
  find_library(_dnssd_LIBRARY dns_sd
    HINTS $ENV{DNSSD_ROOT} ${DNSSD_ROOT}
    PATH_SUFFIXES lib PATHS /usr /usr/local /opt/local /opt
    )
endif()

if(DNSSD_FIND_REQUIRED)
  if(_dnssd_LIBRARY MATCHES "_dnssd_LIBRARY-NOTFOUND")
    set(_dnssd_EPIC_FAIL TRUE)
    message(FATAL_ERROR "Missing the ZeroConf library.\n"
      "Consider using CMAKE_PREFIX_PATH or the DNSSD_ROOT environment variable. "
      "See the ${CMAKE_CURRENT_LIST_FILE} for more details.")
  endif()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DNSSD DEFAULT_MSG
                                  _dnssd_LIBRARY _dnssd_INCLUDE_DIR)

if(_dnssd_EPIC_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(DNSSD_FOUND FALSE)
    set(_dnssd_LIBRARY)
    set(_dnssd_INCLUDE_DIR)
endif()

set(DNSSD_INCLUDE_DIRS ${_dnssd_INCLUDE_DIR})
set(DNSSD_LIBRARIES ${_dnssd_LIBRARY})

if(DNSSD_FOUND)
  message("-- Found ZeroConf in ${DNSSD_INCLUDE_DIRS};${DNSSD_LIBRARIES}")
endif()

