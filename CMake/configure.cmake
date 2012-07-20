
# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

find_package(OpenGL REQUIRED)
find_package(Lunchbox 1.4.0 REQUIRED)
find_package(GLEW_MX)

include_directories(${LUNCHBOX_INCLUDE_DIRS})

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
endif()
