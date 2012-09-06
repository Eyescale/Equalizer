# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

set(AGL_HEADERS
  agl/eventHandler.h
  agl/messagePump.h
  agl/pipe.h
  agl/window.h
  agl/windowEvent.h
  agl/types.h
)

set(GLX_HEADERS
  glx/eventHandler.h
  glx/messagePump.h
  glx/pipe.h
  glx/window.h
  glx/windowEvent.h
  glx/types.h
)

set(WGL_HEADERS
  wgl/eventHandler.h
  wgl/messagePump.h
  wgl/pipe.h
  wgl/window.h
  wgl/windowEvent.h
  wgl/types.h
)

set(CLIENT_HEADERS
  ${AGL_HEADERS} ${GLX_HEADERS} ${WGL_HEADERS}
  aglTypes.h
  api.h
  base.h
  canvas.h
  channel.h
  channelStatistics.h
  client.h
  commandQueue.h
  compositor.h
  computeContext.h
  config.h
  configParams.h
  configStatistics.h
  cudaContext.h
  defines.h
  error.h
  event.h
  eventCommand.h
  eventHandler.h
  exception.h
  eye.h
  frame.h
  frameData.h
  gl.h
  glWindow.h
  glXTypes.h
  global.h
  image.h
  init.h
  layout.h
  log.h
  messagePump.h
  node.h
  nodeFactory.h
  observer.h
  os.h
  pipe.h
  pixelData.h
  segment.h
  server.h
  statistic.h
  statisticSampler.h
  system.h
  systemPipe.h
  systemWindow.h
  types.h
  view.h
  visitorResult.h
  wglTypes.h
  window.h
  windowStatistics.h
  windowSystem.h
  zoomFilter.h
  )

set(CLIENT_SOURCES
  detail/channel.ipp
  canvas.cpp
  channel.cpp
  channelStatistics.cpp
  client.cpp
  commandQueue.cpp
  compositor.cpp
  computeContext.cpp
  config.cpp
  configParams.cpp
  configStatistics.cpp
  cudaContext.cpp
  event.cpp
  eventCommand.cpp
  eventHandler.cpp
  frame.cpp
  frameData.cpp
  gl.cpp
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
  worker.cpp
  )

if(NOT EQUALIZER_BUILD_2_0_API)
  list(APPEND CLIENT_HEADERS configEvent.h)
  list(APPEND CLIENT_SOURCES configEvent.cpp)
endif()

if(EQ_AGL_USED)
  set(AGL_SOURCES
    agl/eventHandler.cpp
    agl/messagePump.cpp
    agl/window.cpp
    agl/pipe.cpp
    agl/windowSystem.cpp
  )
  list(APPEND CLIENT_SOURCES ${AGL_SOURCES})
endif()

if(EQ_GLX_USED)
  set(GLX_SOURCES
    glx/eventHandler.cpp
    glx/messagePump.cpp
    glx/pipe.cpp
    glx/window.cpp
    glx/windowSystem.cpp
    glXTypes.cpp
  )
  list(APPEND CLIENT_SOURCES ${GLX_SOURCES})
endif()

if(WIN32)
  set(WGL_SOURCES
    wgl/eventHandler.cpp
    wgl/messagePump.cpp
    wgl/window.cpp
    wgl/pipe.cpp
    wgl/windowSystem.cpp
  )
  list(APPEND CLIENT_SOURCES ${WGL_SOURCES})
endif()

