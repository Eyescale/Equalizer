# Copyright (c) 2013 Stefan.Eilemann@epfl.ch

# Use pkg-config to fetch the contents of the .pc file
# After that, use the directories refer to the libraries and
# also the headers

find_package(PkgConfig)

if(VNCSERVER_ROOT)
  set(ENV{PKG_CONFIG_PATH} "${VNCSERVER_ROOT}/lib/pkgconfig")
else()
  foreach(PREFIX ${CMAKE_PREFIX_PATH})
    set(PKG_CONFIG_PATH "${PKG_CONFIG_PATH}:${PREFIX}/lib/pkgconfig")
  endforeach()
  set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif()

if(vncserver_FIND_REQUIRED)
  set(_vncserver_OPTS "REQUIRED")
elseif(vncserver_FIND_QUIETLY)
  set(_vncserver_OPTS "QUIET")
else()
  set(_vncserver_output 1)
endif()

if(vncserver_FIND_VERSION)
  if(vncserver_FIND_VERSION_EXACT)
    pkg_check_modules(VNCSERVER ${_vncserver_OPTS} libvncserver=${vncserver_FIND_VERSION})
  else()
    pkg_check_modules(VNCSERVER ${_vncserver_OPTS} libvncserver>=${vncserver_FIND_VERSION})
  endif()
else()
  pkg_check_modules(VNCSERVER ${_vncserver_OPTS} libvncserver)
endif()

if(VNCSERVER_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(VNCSERVER DEFAULT_MSG VNCSERVER_LIBRARIES)

  if(_vncserver_output)
    message(STATUS
      "Found vncserver ${VNCSERVER_VERSION} in ${VNCSERVER_INCLUDE_DIRS}:${VNCSERVER_LIBRARIES}")
  endif()
endif()
