# Copyright (c) 2013 Stefan.Eilemann@epfl.ch

# Use pkg-config to fetch the contents of the .pc file
# After that, use the directories refer to the libraries and
# also the headers

find_package(PkgConfig)

if(JACK_ROOT)
  set(ENV{PKG_CONFIG_PATH} "${JACK_ROOT}/lib/pkgconfig")
else()
  foreach(PREFIX ${CMAKE_PREFIX_PATH})
    set(PKG_CONFIG_PATH "${PKG_CONFIG_PATH}:${PREFIX}/lib/pkgconfig")
  endforeach()
  set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif()

if(jack_FIND_REQUIRED)
  set(_jack_OPTS "REQUIRED")
elseif(jack_FIND_QUIETLY)
  set(_jack_OPTS "QUIET")
else()
  set(_jack_output 1)
endif()

if(jack_FIND_VERSION)
  if(jack_FIND_VERSION_EXACT)
    pkg_check_modules(JACK ${_jack_OPTS} jack=${jack_FIND_VERSION})
  else()
    pkg_check_modules(JACK ${_jack_OPTS} jack>=${jack_FIND_VERSION})
  endif()
else()
  pkg_check_modules(JACK ${_jack_OPTS} jack)
endif()

if(JACK_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(JACK DEFAULT_MSG JACK_LIBRARIES)

  if(_jack_output)
    message(STATUS
      "Found jack ${JACK_VERSION} in ${JACK_INCLUDE_DIRS}:${JACK_LIBRARIES}")
  endif()
endif()
