# Copyright (c) 2012 Marwan Abdellah <marwan.abdellah@epfl.ch>

# Use pkg-config to fetch the contents of the .pc file 
# After that, use the directories refer to the libraries and 
# also the headers
  
find_package(PkgConfig)

# To point to the hwloc.pc in the installation directory 
# For details, http://www.mail-archive.com/cmake@cmake.org/msg14754.html
set(ENV{PKG_CONFIG_PATH} "${CMAKE_BINARY_DIR}/../install/lib/pkgconfig")

pkg_check_modules(HWLOC hwloc)

if(HWLOC_FOUND)
  message(STATUS "Found HWLOC version ${HWLOC_VERSION} with pkg-config")
  set(HWLOC_FOUND 0)
  set(HWLOC_ROOT ${HWLOC_PREFIX})
  
  find_path(HWLOC_INCLUDE_DIR "hwloc.h" 
    HINTS ${HWLOC_ROOT}/include
    /usr/include
    /usr/local/include
    /opt/local/include 
	)

  find_library(HWLOC_LIB NAMES hwloc 
	HINTS ${HWLOC_ROOT}/lib
	PATHS /usr/lib /usr/local/lib /opt/local/lib
	)
  
  set (HWLOC_LIBRARIES ${HWLOC_LIB})
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(HWLOC DEFAULT_MSG HWLOC_LIBRARIES
    HWLOC_INCLUDE_DIR)

  # Version checker 
  if (${HWLOC_VERSION} VERSION_LESS 1.5.0)
	message("Version is less than 1.5.0")
	set(HWLOC_GL_FOUND 0)
  else() 
	message("Version is greater than or equal 1.5.0")
	set(HWLOC_GL_FOUND 1)
  endif()

  if(HWLOC_FOUND)
	message(STATUS "Found HWLOC in ${HWLOC_INCLUDE_DIR};${HWLOC_LIBRARIES}")
  endif(HWLOC_FOUND)
else(HWLOC_FOUND)
  message(STATUS "HWLOC NOT Found")
endif(HWLOC_FOUND)
