
# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

# CUDA
option(EQUALIZER_USE_CUDA "Test for CUDA support" ON)
if(EQUALIZER_USE_CUDA)
  find_package(CUDA)
  if(APPLE)
    set(CUDA_64_BIT_DEVICE_CODE OFF) 
  endif()
endif()
if(CUDA_FOUND)
  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
  set(EQ_FEATURES "${EQ_FEATURES} CUDA")
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
