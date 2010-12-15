##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

# TODO: resolve two-way dependancy between client and util!

set(UTIL_HEADERS
  ../util.h
  ../util/accum.h
  ../util/accumBufferObject.h
  ../util/bitmapFont.h
  ../util/frameBufferObject.h
  ../util/objectManager.h
  ../util/texture.h
  ../util/types.h
  )

set(UTIL_SOURCES
  ../util/accum.cpp
  ../util/accumBufferObject.cpp
  ../util/bitmapFont.cpp
  ../util/gpuCompressor.cpp
  ../util/frameBufferObject.cpp
  ../util/objectManager.cpp
  ../util/texture.cpp
  )

set(CLIENT_HEADERS
  ../client.h
  aglEventHandler.h
  aglMessagePump.h
  aglPipe.h
  aglWindow.h
  aglWindowEvent.h
  base.h
  canvas.h
  channel.h
  channelPackets.h
  channelStatistics.h
  client.h
  clientPackets.h
  commandQueue.h
  compositor.h
  computeContext.h
  config.h
  configEvent.h
  configPackets.h
  configParams.h
  configStatistics.h
  cudaContext.h
  error.h
  event.h
  eventHandler.h
  eye.h
  frame.h
  frameData.h
  glWindow.h
  glXEventHandler.h
  glXMessagePump.h
  glXPipe.h
  glXWindow.h
  glXWindowEvent.h
  global.h
  image.h
  init.h
  layout.h
  log.h
  messagePump.h
  node.h
  nodePackets.h
  nodeFactory.h
  observer.h
  os.h
  packets.h
  pipe.h
  pipePackets.h
  pixelData.h
  segment.h
  server.h
  serverPackets.h
  statistic.h
  statisticSampler.h
  systemPipe.h
  systemWindow.h
  types.h
  version.h
  view.h
  visitorResult.h
  wglEventHandler.h
  wglMessagePump.h
  wglPipe.h
  wglWindow.h
  wglWindowEvent.h
  window.h
  windowPackets.h
  windowStatistics.h
  windowSystem.h
  )

set(CLIENT_SOURCES
  canvas.cpp
  channel.cpp
  channelStatistics.cpp
  client.cpp
  commandQueue.cpp
  compositor.cpp
  computeContext.cpp
  config.cpp
  configEvent.cpp
  configParams.cpp
  configStatistics.cpp
  cudaContext.cpp
  error.cpp
  event.cpp
  eventHandler.cpp
  frame.cpp
  frameData.cpp
  glWindow.cpp
  global.cpp
  image.cpp
  init.cpp
  jitter.cpp
  layout.cpp
  node.cpp
  nodeFactory.cpp
  nodeStatistics.cpp
  observer.cpp
  os.cpp
  pipe.cpp
  pipeStatistics.cpp
  pixelData.cpp
  roiEmptySpaceFinder.cpp
  roiFinder.cpp
  roiTracker.cpp
  segment.cpp
  server.cpp
  statistic.cpp
  systemPipe.cpp
  systemWindow.cpp
  version.cpp
  view.cpp
  window.cpp
  windowStatistics.cpp
  windowSystem.cpp
  )

if(EQ_AGL_USED)
  set(CLIENT_SOURCES ${CLIENT_SOURCES}
    aglEventHandler.cpp
    aglMessagePump.cpp
    aglWindow.cpp
    aglPipe.cpp
    )
endif(EQ_AGL_USED)

if(WIN32)
  set(CLIENT_SOURCES ${CLIENT_SOURCES}
    wglEventHandler.cpp
    wglMessagePump.cpp
    wglWindow.cpp
    wglPipe.cpp
    )
endif(WIN32)

if(EQ_GLX_USED)
  set(CLIENT_SOURCES ${CLIENT_SOURCES}
    glXEventHandler.cpp
    glXMessagePump.cpp
    glXWindow.cpp
    glXPipe.cpp
    )
endif(EQ_GLX_USED)


purple_forward_headers("eq/util/" ${UTIL_HEADERS})

purple_add_library(eq_client ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/client/"
  FORWARD HEADERS
    ${CLIENT_HEADERS}
    ${UTIL_HEADERS}
  SOURCES
    ${CLIENT_SOURCES}
    ${UTIL_SOURCES}
  LINK_LIBRARIES
    shared eq_base
    shared eq_net
    shared eq_fabric
    ${GLEW_LIBRARY}
    ${CARBON_LIBRARY}
    ${X11_LIBRARIES}
    ${GL_LIBRARIES}
    ${CUDA_LIBRARIES}
  )
