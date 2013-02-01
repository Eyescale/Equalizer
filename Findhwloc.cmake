# Copyright (c) 2012 Marwan Abdellah <marwan.abdellah@epfl.ch>
#                    Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#               2013 Stefan.Eilemann@epfl.ch

# Use pkg-config to fetch the contents of the .pc file
# After that, use the directories refer to the libraries and
# also the headers

find_package(PkgConfig)

if(HWLOC_ROOT)
  set(ENV{PKG_CONFIG_PATH} "${HWLOC_ROOT}/lib/pkgconfig")
else()
  foreach(PREFIX ${CMAKE_PREFIX_PATH})
    set(PKG_CONFIG_PATH "${PKG_CONFIG_PATH}:${PREFIX}/lib/pkgconfig")
  endforeach()
  set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif()

if(hwloc_FIND_REQUIRED)
  set(_hwloc_OPTS "REQUIRED")
elseif(hwloc_FIND_QUIETLY)
  set(_hwloc_OPTS "QUIET")
else()
  set(_hwloc_output 1)
endif()

if(hwloc_FIND_VERSION)
  if(hwloc_FIND_VERSION_EXACT)
    pkg_check_modules(HWLOC ${_hwloc_OPTS} hwloc=${hwloc_FIND_VERSION})
  else()
    pkg_check_modules(HWLOC ${_hwloc_OPTS} hwloc>=${hwloc_FIND_VERSION})
  endif()
else()
  pkg_check_modules(HWLOC ${_hwloc_OPTS} hwloc)
endif()

if(HWLOC_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(HWLOC DEFAULT_MSG HWLOC_LIBRARIES
    HWLOC_INCLUDE_DIRS)

  if(NOT ${HWLOC_VERSION} VERSION_LESS 1.5.0)
    set(HWLOC_GL_FOUND 1)
  endif()

  if(_hwloc_output)
    message(STATUS
      "Found hwloc ${HWLOC_VERSION} in ${HWLOC_INCLUDE_DIRS}:${HWLOC_LIBRARIES}")
  endif()
endif()
