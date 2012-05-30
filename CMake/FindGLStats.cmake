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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/FindGLStats)
include(FindLibraryPackage)
include(FindPackageHandleStandardArgs)

find_library_package(GLStats)
find_package_handle_standard_args(GLStats DEFAULT_MSG
                                  GLSTATS_LIBRARIES GLSTATS_INCLUDE_DIRS)
