#
# Copyright 2012 Stefan Eilemann <eile@eyescale.ch>
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
# - Find Servus
# This module searches for the Servus library
#    See https://github.com/Eyescale/servus
#
#==================================
#
# The following environment variables are respected for finding Servus.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    SERVUS_ROOT
#    EQ_ROOT
#
# This module defines the following output variables:
#
#    SERVUS_FOUND - Was Servus and all of the specified components found?
#
#    SERVUS_VERSION - The version of Servus which was found
#
#    SERVUS_VERSION_ABI - The DSO version of Servus which was found
#
#    SERVUS_INCLUDE_DIRS - Where to find the headers
#
#    SERVUS_LIBRARIES - The Servus libraries
#
#==================================
# Example Usage:
#
#  find_package(Servus 0.3.0 REQUIRED)
#  include_directories(${SERVUS_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${SERVUS_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _servus_foo
#  Input variables of the form Servus_FOO
#  Output variables of the form SERVUS_FOO
#

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/FindServus)
include(FindLibraryPackage)
include(FindPackageHandleStandardArgs)

find_library_package(servus)
find_package_handle_standard_args(Servus DEFAULT_MSG
                                  SERVUS_LIBRARIES SERVUS_INCLUDE_DIRS)
