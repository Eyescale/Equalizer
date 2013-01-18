
# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

if(APPLE)
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  LIST(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local)
  link_directories(/usr/X11R6/lib)
  include_directories(SYSTEM /usr/X11R6/include)
endif()

include(FindPackages)

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

find_package(GLEW_MX)

include_directories(${COLLAGE_INCLUDE_DIRS})

# CUDA
option(EQUALIZER_USE_CUDA "Test for CUDA support" ON)
if(EQUALIZER_USE_CUDA AND CMAKE_VERSION VERSION_GREATER 2.7)
  find_package(CUDA)
  if(APPLE)
    set(CUDA_64_BIT_DEVICE_CODE OFF)
  endif()
endif()
if(CUDA_FOUND)
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
endif(CUDA_FOUND)

# OpenSceneGraph
option(EQUALIZER_USE_OSG "Test for OpenSceneGraph support" ON)
if(EQUALIZER_USE_OSG)
  find_package(OpenSceneGraph 2.9.8 COMPONENTS osgDB osgUtil)
  if(OSG_LIBRARY MATCHES "OSG_LIBRARY-NOTFOUND")
    set(OSG_FOUND 0)
  endif()
  if("${OPENSCENEGRAPH_VERSION}" VERSION_LESS "2.9.8")
    set(OSG_FOUND 0)
  endif()
else()
  set(OSG_FOUND)
endif()
