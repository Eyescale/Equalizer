
# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

if(APPLE)
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  list(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  list(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local)
  link_directories(/usr/X11R6/lib)
  include_directories(SYSTEM /usr/X11R6/include)
endif()

include(FindPackages)

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

find_package(GLEW_MX)

if(CUDA_FOUND)
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
endif(CUDA_FOUND)

# OpenSceneGraph
option(EQUALIZER_USE_OSG "Test for OpenSceneGraph support" ON)
if(NOT EQUALIZER_USE_OSG)
  set(OSG_FOUND)
endif()
