# Copyright (c) 2012

find_path(HWLOC_INCLUDE_DIR "hwloc.h"
  /usr/include
  /usr/local/include
  /opt/local/include 
)

# ---------------------------------
# http://www.vtk.org/Wiki/CMake_FAQ
# ---------------------------------
file(TO_CMAKE_PATH "$ENV{HWLOC_LIB_DIR}" HWLOC_LIB_DIR)
find_library(HWLOC_LIB NAMES hwloc HINTS ${HWLOC_LIB_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HWLOC DEFAULT_MSG HWLOC_LIB HWLOC_INCLUDE_DIR)

if(HWLOC_FOUND)
  message("-- Found HWLOC in ${HWLOC_INCLUDE_DIR}, ${HWLOC_LIB}")
else()
   message("HWLOC NOT FOUND")
endif(HWLOC_FOUND)
