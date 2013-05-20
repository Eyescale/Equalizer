# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

include(FindPackageHandleStandardArgs)

set(MAGELLAN_API "spnav")
if(APPLE)
  foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
    if(ARCH STREQUAL "ppc" OR ARCH STREQUAL "i386")
      set(MAGELLAN_API "Carbon")
    endif()
  endforeach()
endif()
if(MSVC)
  set(MAGELLAN_API "Windows")
endif()
if(MAGELLAN_FIND_COMPONENTS)
  set(MAGELLAN_API ${MAGELLAN_FIND_COMPONENTS})
endif()

if(MAGELLAN_API STREQUAL "Windows")
  set(MAGELLAN_FOUND 1) # always on
  return()
endif()

if(MAGELLAN_API STREQUAL "Carbon")
  find_path(MAGELLAN_INCLUDE_DIR 3DconnexionClient/ConnexionClientAPI.h
    PATHS /usr/include /usr/local/include /opt/local/include)
  set(MAGELLAN_LIBRARY)
elseif(MAGELLAN_API STREQUAL "spnav")
  find_path(MAGELLAN_INCLUDE_DIR spnav.h PATHS /usr/include /usr/local/include)
  find_library(MAGELLAN_LIBRARY spnav PATHS /usr/lib /usr/local/lib)
endif()

find_package_handle_standard_args(MAGELLAN DEFAULT_MSG MAGELLAN_INCLUDE_DIR
  MAGELLAN_LIBRARY)

if(MAGELLAN_FOUND AND NOT MAGELLAN_FIND_QUIETLY)
  message(STATUS
    "Found SpaceMouse API in ${MAGELLAN_INCLUDE_DIR};${MAGELLAN_LIBRARY}")
endif()
