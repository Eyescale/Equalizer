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
#    GPUSD_COMPONENTS - A list of components found
#
#    GPUSD_DEB_DEPENDENCIES - A list of dependencies for the CPack deb generator
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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Findgpusd)
include(FindLibraryPackage)
include(FindPackageHandleStandardArgs)

find_library_package(gpusd)

# core
set(GPUSD_core_LIBRARY ${GPUSD_LIBRARY})
set(GPUSD_COMPONENTS core) # reset in !FOUND

# components
set(_gpusd_COMPONENTS cgl glx wgl dns_sd)
foreach(_gpusd_COMPONENT ${_gpusd_COMPONENTS})
  set(_gpusd_lib GPUSD_${_gpusd_COMPONENT}_LIBRARY)
  find_library(${_gpusd_lib} gpusd_${_gpusd_COMPONENT}
    PATHS ${GPUSD_INCLUDE_DIR}/.. PATH_SUFFIXES lib NO_DEFAULT_PATH)

  if(${_gpusd_lib} MATCHES "${_gpusd_lib}-NOTFOUND")
    if(gpusd_FIND_COMPONENTS MATCHES ${_gpusd_COMPONENT} AND _gpusd_output)
      message(${_gpusd_version_output_type}
        "gpusd_${_gpusd_COMPONENT} not found in ${GPUSD_INCLUDE_DIR}/../lib")
    endif()
  else()
    set(GPUSD_${_gpusd_COMPONENT}_FOUND TRUE)
    list(APPEND GPUSD_LIBRARIES ${${_gpusd_lib}})
    list(APPEND GPUSD_COMPONENTS ${_gpusd_COMPONENT})
  endif()
endforeach()

find_package_handle_standard_args(gpusd DEFAULT_MSG
                                  GPUSD_core_LIBRARY GPUSD_INCLUDE_DIRS)
if(GPUSD_FOUND)
  set(GPUSD_DEB_DEPENDENCIES "gpu-sd${GPUSD_VERSION_MAJOR}-runtime")
  set(GPUSD_DEB_BUILD_DEPENDENCIES "gpu-sd${GPUSD_VERSION_MAJOR}-dev")
  if(_flp_output)
    message(STATUS "Found gpusd modules ${GPUSD_COMPONENTS}")
  endif()
else()
  set(GPUSD_COMPONENTS)
endif()
