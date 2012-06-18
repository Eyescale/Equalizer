# Copyright (c) 2012 Marwan Abdellah <marwan.abdellah@epfl.ch>

# Use pkg-config to fetch the contents of the .pc file 
# After that, use the directories refer to the libraries and 
# also the headers

find_package(PkgConfig)

# To point to the hwloc.pc in the installation directory 
# For details, http://www.mail-archive.com/cmake@cmake.org/msg14754.html
if(HWLOC_ROOT)
  set(ENV{PKG_CONFIG_PATH} "${HWLOC_ROOT}/lib/pkgconfig")
endif()

if(NOT HWLOC_FIND_QUIETLY)
  set(_hwloc_output 1)
endif()

pkg_check_modules(HWLOC hwloc)

if(HWLOC_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(HWLOC DEFAULT_MSG HWLOC_LIBRARIES
    HWLOC_INCLUDE_DIRS)

  find_library(HWLOC_LIBRARIES hwloc
    PATHS ${HWLOC_ROOT} PATH_SUFFIXES lib NO_DEFAULT_PATH)

  if(${HWLOC_VERSION} VERSION_LESS 1.5.0)
    set(HWLOC_GL_FOUND)
  else() 
    set(HWLOC_GL_FOUND 1)
  endif()

  if(HWLOC_FOUND AND _hwloc_output)
    message(STATUS
      "Found HWLOC ${HWLOC_VERSION} in ${HWLOC_INCLUDE_DIRS};${HWLOC_LIBRARIES}")
  endif()
endif(HWLOC_FOUND)
