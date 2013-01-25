# Copyright (c) 2012 Marwan Abdellah <marwan.abdellah@epfl.ch>
#                    Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# Use pkg-config to fetch the contents of the .pc file
# After that, use the directories refer to the libraries and
# also the headers

find_package(PkgConfig)

if(HWLOC_ROOT)
  set(ENV{PKG_CONFIG_PATH} "${HWLOC_ROOT}/lib/pkgconfig")
endif()

if(HWLOC_FIND_QUIETLY)
  set(_hwloc_QUIET QUIET)
else()
  set(_hwloc_output 1)
endif()

if(HWLOC_FIND_VERSION)
  if(HWLOC_FIND_VERSION_EXACT)
    pkg_check_modules(HWLOC ${_hwloc_QUIET} hwloc=${HWLOC_FIND_VERSION})
  else()
    pkg_check_modules(HWLOC ${_hwloc_QUIET} hwloc>=${HWLOC_FIND_VERSION})
  endif()
else()
  pkg_check_modules(HWLOC ${_hwloc_QUIET} hwloc)
endif()

if(HWLOC_FOUND)
  find_library(HWLOC_LIBRARY hwloc
    HINTS $ENV{HWLOC_ROOT} ${HWLOC_ROOT}
    PATHS ${HWLOC_LIBRARY_DIRS}
    PATH_SUFFIXES lib)
  set(HWLOC_LIBRARIES ${HWLOC_LIBRARY})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(HWLOC DEFAULT_MSG HWLOC_LIBRARIES
    HWLOC_INCLUDEDIR)

  if(${HWLOC_VERSION} VERSION_LESS 1.5.0)
    set(HWLOC_GL_FOUND)
  else()
    set(HWLOC_GL_FOUND 1)
  endif()

  set(HWLOC_INCLUDE_DIRS ${HWLOC_INCLUDEDIR})

  if(HWLOC_FOUND AND _hwloc_output)
    message(STATUS
      "Found HWLOC ${HWLOC_VERSION} in ${HWLOC_INCLUDE_DIRS};${HWLOC_LIBRARIES}")
  endif()
endif()
