# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>
#                    2012 Daniel Nachbaur <danielnachbaur@gmail.com>

set(EQ_FABRIC_PUBLIC_HEADERS
  api.h
  base.h
  canvas.h
  channel.h
  client.h
  colorMask.h
  commands.h
  config.h
  configParams.h
  configVisitor.h
  drawableConfig.h
  elementVisitor.h
  equalizer.h
  equalizerTypes.h
  error.h
  errorRegistry.h
  eye.h
  focusMode.h
  frame.h
  frustum.h
  global.h
  gpuInfo.h
  iAttribute.h
  init.h
  layout.h
  leafVisitor.h
  log.h
  node.h
  nodeType.h
  object.h
  observer.h
  paths.h
  pipe.h
  pixel.h
  pixelViewport.h
  projection.h
  range.h
  renderContext.h
  segment.h
  server.h
  subPixel.h
  swapBarrier.h
  task.h
  tile.h
  types.h
  view.h
  viewport.h
  vmmlib.h
  wall.h
  window.h
  zoom.h
  )

set(EQ_FABRIC_HEADERS
  nameFinder.h
  canvas.ipp
  channel.ipp
  config.ipp
  layout.ipp
  node.ipp
  observer.ipp
  pipe.ipp
  segment.ipp
  server.ipp
  view.ipp
  window.ipp
  )

set(EQ_FABRIC_SOURCES
  client.cpp
  colorMask.cpp
  configParams.cpp
  equalizer.cpp
  error.cpp
  errorRegistry.cpp
  eye.cpp
  frame.cpp
  frustum.cpp
  global.cpp
  iAttribute.cpp
  init.cpp
  object.cpp
  pixel.cpp
  projection.cpp
  range.cpp
  renderContext.cpp
  subPixel.cpp
  swapBarrier.cpp
  viewport.cpp
  wall.cpp
  zoom.cpp
  )

list(SORT EQ_FABRIC_HEADERS)
list(SORT EQ_FABRIC_PUBLIC_HEADERS)
list(SORT EQ_FABRIC_SOURCES)
