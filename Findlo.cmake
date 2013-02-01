# Copyright (c) 2013 Stefan.Eilemann@epfl.ch

# Use pkg-config to fetch the contents of the .pc file
# After that, use the directories refer to the libraries and
# also the headers

find_package(PkgConfig)

if(LO_ROOT)
  set(ENV{PKG_CONFIG_PATH} "${LO_ROOT}/lib/pkgconfig")
else()
  foreach(PREFIX ${CMAKE_PREFIX_PATH})
    set(PKG_CONFIG_PATH "${PKG_CONFIG_PATH}:${PREFIX}/lib/pkgconfig")
  endforeach()
  set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif()

if(lo_FIND_REQUIRED)
  set(_lo_OPTS "REQUIRED")
elseif(lo_FIND_QUIETLY)
  set(_lo_OPTS "QUIET")
else()
  set(_lo_output 1)
endif()

if(lo_FIND_VERSION)
  if(lo_FIND_VERSION_EXACT)
    pkg_check_modules(LO ${_lo_OPTS} liblo=${lo_FIND_VERSION})
  else()
    pkg_check_modules(LO ${_lo_OPTS} liblo>=${lo_FIND_VERSION})
  endif()
else()
  pkg_check_modules(LO ${_lo_OPTS} liblo)
endif()

if(LO_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LO DEFAULT_MSG LO_LIBRARIES)

  if(_lo_output)
    message(STATUS
      "Found lo ${LO_VERSION} in ${LO_INCLUDE_DIRS}:${LO_LIBRARIES}")
  endif()
endif()
