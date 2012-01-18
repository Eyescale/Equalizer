# Copyright (c) 2012 Marwan Abdellah <marwan.abdellah@epfl.ch>

find_path(HWLOC_INCLUDE_DIR "hwloc.h" 
  HINTS ${HWLOC_ROOT}/include
  /usr/include
  /usr/local/include
  /opt/local/include 
)

find_library(HWLOC_LIB NAMES hwloc 
	HINTS ${HWLOC_ROOT}
	PATHS 
	/usr/lib 
	/usr/local/lib 
	/opt/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HWLOC DEFAULT_MSG HWLOC_LIB HWLOC_INCLUDE_DIR)

if(HWLOC_FOUND)
  message(STATUS "Found HWLOC in ${HWLOC_INCLUDE_DIR};${HWLOC_LIB}")
endif(HWLOC_FOUND)
