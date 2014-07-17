
# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

if(APPLE)
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  list(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  list(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local)
  link_directories(/opt/X11/lib /usr/X11R6/lib)
  include_directories(SYSTEM /opt/X11/include /usr/X11R6/include)
endif()

include(FindPackages)

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

if(CUDA_FOUND)
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
endif(CUDA_FOUND)
