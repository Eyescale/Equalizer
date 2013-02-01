# Copyright (c) 2013 Daniel.Nachbaur@epfl.ch

# Use pkg-config to fetch the contents of the .pc file
# After that, use the directories refer to the libraries and
# also the headers

find_package(PkgConfig)

if(V4L2_ROOT)
  set(ENV{PKG_CONFIG_PATH} "${V4L2_ROOT}/lib/pkgconfig")
else()
  foreach(PREFIX ${CMAKE_PREFIX_PATH})
    set(PKG_CONFIG_PATH "${PKG_CONFIG_PATH}:${PREFIX}/lib/pkgconfig")
  endforeach()
  set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif()

if(v4l2_FIND_REQUIRED)
  set(_v4l2_OPTS "REQUIRED")
elseif(v4l2_FIND_QUIETLY)
  set(_v4l2_OPTS "QUIET")
else()
  set(_v4l2_output 1)
endif()

if(v4l2_FIND_VERSION)
  if(v4l2_FIND_VERSION_EXACT)
    pkg_check_modules(V4L2 ${_v4l2_OPTS} libv4l2=${v4l2_FIND_VERSION})
  else()
    pkg_check_modules(V4L2 ${_v4l2_OPTS} libv4l2>=${v4l2_FIND_VERSION})
  endif()
else()
  pkg_check_modules(V4L2 ${_v4l2_OPTS} libv4l2)
endif()

if(V4L2_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(V4L2 DEFAULT_MSG V4L2_LIBRARIES)

  if(_v4l2_output)
    message(STATUS
      "Found v4l2 ${V4L2_VERSION} in ${V4L2_INCLUDE_DIRS}:${V4L2_LIBRARIES}")
  endif()
endif()
