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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/FindDASH)
include(FindLibraryPackage)
include(FindPackageHandleStandardArgs)

find_library_package(dash TRANSIENT Lunchbox)
find_package_handle_standard_args(DASH DEFAULT_MSG
                                  DASH_LIBRARIES DASH_INCLUDE_DIRS)
