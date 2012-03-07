# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

if(EQ_AGL_USED)
  find_path(MAGELLAN_INCLUDE_DIR 3DconnexionClient/ConnexionClientAPI.h
    PATHS /usr/include /usr/local/include /opt/local/include)
  find_library(MAGELLAN_LIBRARY NAMES 3DconnexionClient)
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(MAGELLAN DEFAULT_MSG
    MAGELLAN_INCLUDE_DIR MAGELLAN_LIBRARY)
  if(MAGELLAN_FOUND)
    message(STATUS "Found SpaceMouse API in ${MAGELLAN_INCLUDE_DIR};${MAGELLAN_LIBRARY}")
  endif()
endif()

if(MSVC)
  set(MAGELLAN_FOUND 1) # always on
endif()
