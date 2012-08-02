# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

include(FindPackageHandleStandardArgs)

if(MSVC)
  set(MAGELLAN_FOUND 1) # always on
else()
  if(EQ_AGL_USED)
    find_path(MAGELLAN_INCLUDE_DIR 3DconnexionClient/ConnexionClientAPI.h
      PATHS /usr/include /usr/local/include /opt/local/include)
    find_library(MAGELLAN_LIBRARY NAMES 3DconnexionClient)
  elseif(EQ_GLX_USED)
    find_path(MAGELLAN_INCLUDE_DIR spnav.h PATHS /usr/include /usr/local/include)
    find_library(MAGELLAN_LIBRARY spnav PATHS /usr/lib /usr/local/lib)
  endif()

  find_package_handle_standard_args(MAGELLAN DEFAULT_MSG MAGELLAN_INCLUDE_DIR
    MAGELLAN_LIBRARY)
  if(MAGELLAN_FOUND)
    message(STATUS
      "Found SpaceMouse API in ${MAGELLAN_INCLUDE_DIR};${MAGELLAN_LIBRARY}")
  endif()
endif()
