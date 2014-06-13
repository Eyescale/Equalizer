# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2013 Stefan Eilemann <eile@eyescale.ch>
#               2012 Daniel Nachbaur <danielnachbaur@gmail.com>

set(AGL_HEADERS
  agl/eventHandler.h
  agl/messagePump.h
  agl/pipe.h
  agl/window.h
  agl/windowEvent.h
  agl/types.h
)

set(AGL_SOURCES
  agl/eventHandler.cpp
  agl/messagePump.cpp
  agl/window.cpp
  agl/pipe.cpp
  agl/windowSystem.cpp
)

set(GLX_HEADERS
  glx/eventHandler.h
  glx/messagePump.h
  glx/pipe.h
  glx/window.h
  glx/windowEvent.h
  glx/types.h
)

set(GLX_SOURCES
  glx/eventHandler.cpp
  glx/messagePump.cpp
  glx/pipe.cpp
  glx/window.cpp
  glx/windowSystem.cpp
  glx/X11Connection.h
  glx/types.cpp
)

if(DISPLAYCLUSTER_FOUND)
  set(DISPLAYCLUSTER_SOURCES
    dc/connection.h
    dc/eventHandler.cpp
    dc/eventHandler.h
    dc/proxy.cpp
    dc/proxy.h
  )
endif()

set(QT_HEADERS
  qt/eventHandler.h
  qt/glWidget.h
  qt/messagePump.h
  qt/types.h
  qt/window.h
  qt/windowEvent.h
)

set(QT_SOURCES
  qt/eventHandler.cpp
  qt/glWidget.cpp
  qt/messagePump.cpp
  qt/window.cpp
  qt/windowSystem.cpp
)

set(WGL_HEADERS
  wgl/eventHandler.h
  wgl/messagePump.h
  wgl/pipe.h
  wgl/window.h
  wgl/windowEvent.h
  wgl/types.h
)

set(WGL_SOURCES
  wgl/eventHandler.cpp
  wgl/messagePump.cpp
  wgl/window.cpp
  wgl/pipe.cpp
  wgl/windowSystem.cpp
)

set(CLIENT_PUBLIC_HEADERS
  ${AGL_HEADERS} ${GLX_HEADERS} ${QT_HEADERS} ${WGL_HEADERS}
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
  configEvent.h
  configStatistics.h
  cudaContext.h
  error.h
  event.h
  eventICommand.h
  eventHandler.h
  exception.h
  eye.h
  frame.h
  frameData.h
  gl.h
  glException.h
  glWindow.h
  global.h
  image.h
  init.h
  layout.h
  log.h
  messagePump.h
  node.h
  nodeFactory.h
  notifierInterface.h
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
  window.h
  windowSettings.h
  windowStatistics.h
  windowSystem.h
  zoomFilter.h
  )

set(CLIENT_HEADERS
  detail/fileFrameWriter.h
  exitVisitor.h
  half.h
  initVisitor.h
  transferFinder.h
  )

set(CLIENT_SOURCES
  ${DISPLAYCLUSTER_SOURCES}
  canvas.cpp
  channel.cpp
  channelStatistics.cpp
  client.cpp
  commandQueue.cpp
  compositor.cpp
  computeContext.cpp
  config.cpp
  configStatistics.cpp
  cudaContext.cpp
  detail/channel.ipp
  detail/fileFrameWriter.cpp
  event.cpp
  eventHandler.cpp
  eventICommand.cpp
  frame.cpp
  frameData.cpp
  gl.cpp
  glException.cpp
  glWindow.cpp
  global.cpp
  half.cpp
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
  windowSettings.cpp
  windowStatistics.cpp
  windowSystem.cpp
  worker.cpp
  )

if(NOT EQUALIZER_BUILD_2_0_API)
  list(APPEND CLIENT_SOURCES configEvent.cpp)
endif()
if(GLSTATS_FOUND)
  list(APPEND CLIENT_HEADERS detail/statsRenderer.h)
endif()

if(EQ_AGL_USED)
  list(APPEND CLIENT_SOURCES ${AGL_SOURCES})
endif()
if(EQ_GLX_USED)
  list(APPEND CLIENT_SOURCES ${GLX_SOURCES})
endif()
if(EQ_QT_USED)
  list(APPEND CLIENT_SOURCES ${QT_SOURCES})
endif()
if(WIN32)
  list(APPEND CLIENT_SOURCES ${WGL_SOURCES})
endif()
if(OpenCV_FOUND)
  list(APPEND CLIENT_SOURCES detail/cvTracker.h detail/cvTracker.cpp)
endif()

list(SORT CLIENT_SOURCES)
list(SORT CLIENT_HEADERS)
list(SORT CLIENT_PUBLIC_HEADERS)
