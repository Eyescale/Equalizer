#
# Copyright 2012 Daniel Nachbaur <danielnachbaur@gmail.com>
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
# Find HW-SD. This module searches for the HW-SD library
#    See http://https://github.com/Eyescale/hwsd
#
#
#==================================
#
# The following environment variables are respected for finding HW-SD.
# CMAKE_PREFIX_PATH can also be used for this (see find_library() CMake
# documentation).
#
#    HWSD_ROOT
#
# This module defines the following output variables:
#
#    HWSD_FOUND - Was HW-SD and all of the specified components found?
#
#    HWSD_VERSION - The version of HW-SD which was found
#
#    HWSD_INCLUDE_DIRS - Where to find the headers
#
#    HWSD_LIBRARIES - The HW-SD libraries
#
#    HWSD_COMPONENTS - A list of components found
#
#    HWSD_DEB_DEPENDENCIES - A list of dependencies for the CPack deb generator
#
#==================================
# Example Usage:
#
#  find_package(HWSD 1.0.0 dns_sd REQUIRED)
#  include_directories(${HWSD_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${HWSD_LIBRARIES})
#
#==================================
# Naming convention:
#  Local variables of the form _hwsd_foo
#  Input variables of the form HWSD_FOO
#  Output variables of the form HWSD_FOO
#

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Findhwsd)
include(FindLibraryPackage)
include(FindPackageHandleStandardArgs)

find_library_package(hwsd)

# core
set(HWSD_core_LIBRARY ${HWSD_LIBRARY})
set(HWSD_COMPONENTS core) # reset in !FOUND

# components
set(_hwsd_COMPONENTS gpu_cgl gpu_glx gpu_wgl gpu_dns_sd net_hwloc net_sys net_dns_sd)
foreach(_hwsd_COMPONENT ${_hwsd_COMPONENTS})
  set(_hwsd_lib HWSD_${_hwsd_COMPONENT}_LIBRARY)
  find_library(${_hwsd_lib} hwsd_${_hwsd_COMPONENT}
    PATHS ${HWSD_INCLUDE_DIR}/.. PATH_SUFFIXES lib NO_DEFAULT_PATH)

  if(${_hwsd_lib} MATCHES "${_hwsd_lib}-NOTFOUND")
    if(hwsd_FIND_COMPONENTS MATCHES ${_hwsd_COMPONENT} AND _hwsd_output)
      message(${_hwsd_version_output_type}
        "hwsd_${_hwsd_COMPONENT} not found in ${HWSD_INCLUDE_DIR}/../lib")
    endif()
  else()
    set(HWSD_${_hwsd_COMPONENT}_FOUND TRUE)
    list(APPEND HWSD_LIBRARIES ${${_hwsd_lib}})
    list(APPEND HWSD_COMPONENTS ${_hwsd_COMPONENT})
  endif()
endforeach()

find_package_handle_standard_args(hwsd DEFAULT_MSG
                                  HWSD_LIBRARIES HWSD_INCLUDE_DIRS)

if(HWSD_FOUND)
  set(HWSD_DEB_DEPENDENCIES "hwsd-sd${HWSD_VERSION_MAJOR}-runtime")
  set(HWSD_DEB_BUILD_DEPENDENCIES "hwsd-sd${HWSD_VERSION_MAJOR}-dev")
  if(_flp_output)
    message(STATUS "Found hwsd modules ${HWSD_COMPONENTS}")
  endif()
else()
  set(HWSD_COMPONENTS)
endif()
