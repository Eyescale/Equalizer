
# Copyright (c) 2011 Maxim Makhinya <maxmah@gmail.com>
#               2012 David Steiner  <steiner@ifi.uzh.ch>

set(VOLVIS_ASYNC_SYSTEM_SOURCES)
set(VOLVIS_ASYNC_SYSTEM_HEADERS)

if(EQ_AGL_USED)
  set(VOLVIS_ASYNC_SYSTEM_HEADERS asyncFetcher/aglWindowShared.h)
  set(VOLVIS_ASYNC_SYSTEM_SOURCES asyncFetcher/aglWindowShared.cpp)
endif()

if(EQ_GLX_USED)
  set(VOLVIS_ASYNC_SYSTEM_HEADERS asyncFetcher/glXWindowShared.h)
endif()

# set(CUDA_COMP_FILES)

set(VOLVIS_ASYNC_FETCHER_HEADERS
  ./asyncFetcher/compression/compression.h
  ./asyncFetcher/gpuAsyncLoader.h
  ./asyncFetcher/gpuAsyncLoaderBase.h
  ./asyncFetcher/gpuCacheIndex.h
  ./asyncFetcher/gpuCacheManager.h
  ./asyncFetcher/gpuCommands.h
  ./asyncFetcher/ramAsyncLoader.h
  ./asyncFetcher/ramCommands.h
  ./asyncFetcher/ramDataElement.h
  ./asyncFetcher/ramPool.h
  ./asyncFetcher/timeStamp.h
  ./asyncFetcher/timeStamper.h
  ${VOLVIS_ASYNC_SYSTEM_HEADERS}
)

set(VOLVIS_ASYNC_FETCHER_SOURCES
  ./asyncFetcher/compression/compression.cpp
#  ${CUDA_COMP_FILES}
  ./asyncFetcher/gpuAsyncLoader.cpp
  ./asyncFetcher/gpuAsyncLoaderBase.cpp
  ./asyncFetcher/gpuCacheIndex.cpp
  ./asyncFetcher/gpuCacheManager.cpp
  ./asyncFetcher/ramAsyncLoader.cpp
  ./asyncFetcher/ramPool.cpp
  ./asyncFetcher/timeStamper.cpp
  ${VOLVIS_ASYNC_SYSTEM_SOURCES}
)

set(VOLVIS_EQ_HEADERS
  ./EQ/cameraAnimation.h
  ./EQ/channel.h
  ./EQ/guiConnectionDefs.h
  ./EQ/guiDock.h
  ./EQ/guiPackets.h
  ./EQ/config.h
  ./EQ/error.h
  ./EQ/frameData.h
  ./EQ/initData.h
  ./EQ/localInitData.h
  ./EQ/node.h
  ./EQ/pipe.h
  ./EQ/screenGrabber.h
  ./EQ/volumeInfo.h
  ./EQ/volVis.h
  ./EQ/window.h
)

set(VOLVIS_EQ_SOURCES
  ./EQ/cameraAnimation.cpp
  ./EQ/channel.cpp
  ./EQ/guiDock.cpp
  ./EQ/config.cpp
  ./EQ/error.cpp
  ./EQ/frameData.cpp
  ./EQ/initData.cpp
  ./EQ/localInitData.cpp
  ./EQ/node.cpp
  ./EQ/pipe.cpp
  ./EQ/screenGrabber.cpp
  ./EQ/volumeInfo.cpp
  ./EQ/volVis.cpp
  ./EQ/window.cpp
)

set(VOLVIS_LB_HEADERS
  ./LB/boxN.h
  ./LB/cameraParameters.h
  ./LB/orderEstimator.h
  ./LB/renderNode.h
)

set(VOLVIS_LB_SOURCES
  ./LB/cameraParameters.cpp
  ./LB/orderEstimator.cpp
)

set(VOLVIS_RENDERER_HEADERS
  ./renderer/glslShaders.h
  ./renderer/model.h
  ./renderer/rendererBase.h
  ./renderer/transferFunction.h
  ./renderer/raycasting/nearPlaneClipper.h
  ./renderer/raycasting/rendererRaycast.h
  ./renderer/raycasting/rendererRaycast2.h
  ./renderer/slice/rendererSlice.h
  ./renderer/slice/sliceClipping.h
)

set(VOLVIS_RENDERER_SOURCES
  ./renderer/glslShaders.cpp
  ./renderer/model.cpp
  ./renderer/rendererBase.cpp
  ./renderer/transferFunction.cpp
  ./renderer/raycasting/nearPlaneClipper.cpp
  ./renderer/raycasting/rendererRaycast.cpp
  ./renderer/raycasting/rendererRaycast2.cpp
  ./renderer/slice/rendererSlice.cpp
  ./renderer/slice/sliceClipping.cpp
)

set(VOLVIS_MAIN_SOURCES
  ./main.cpp
)

set(VOLVIS_MAIN_TESTS_SOURCES
  ./mainTests.cpp
)

set(VOLVIS_UTIL_HEADERS
  ./util/unlocker.h
)

set(VOLVIS_SHADERS
  ./renderer/raycasting/vertexShader_raycast.glsl
  ./renderer/raycasting/fragmentShader_raycast.glsl
  ./renderer/raycasting/fragmentShader_raycast_ext.glsl
  ./renderer/slice/vertexShader_slice.glsl
  ./renderer/slice/fragmentShader_slice.glsl
)

