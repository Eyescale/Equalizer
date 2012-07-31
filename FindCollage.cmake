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

include(FindLibraryPackage)
include(FindPackageHandleStandardArgs)

set(_Collage_Lunchbox_version_0.5.1 "0.9.0")
set(_Collage_Lunchbox_version_0.5.2 "1.3.5")
set(_Collage_Lunchbox_version_0.5.5 "1.3.5")

# needed if called directly w/o called from FindEqualizer
if(NOT COLLAGE_ROOT)
  set(COLLAGE_ROOT ${CO_ROOT})
endif()
if(NOT COLLAGE_ROOT)
  set(COLLAGE_ROOT ${EQ_ROOT})
endif()

find_library_package(Collage INCLUDE co TRANSIENT Lunchbox)
find_package_handle_standard_args(Collage DEFAULT_MSG
                                  COLLAGE_LIBRARIES COLLAGE_INCLUDE_DIRS)
