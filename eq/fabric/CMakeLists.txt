
# Copyright (c) 2011-2016 Stefan Eilemann <eile@eyescale.ch>

set(EQUALIZERFABRIC_PUBLIC_HEADERS
  axisEvent.h
  base.h
  buttonEvent.h
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
  errorCodes.h
  errorRegistry.h
  event.h
  eventEnums.h
  eventType.h
  eye.h
  focusMode.h
  frame.h
  frameData.h
  frustum.h
  global.h
  gpuInfo.h
  iAttribute.h
  init.h
  keyEvent.h
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
  pointerEvent.h
  projection.h
  range.h
  renderContext.h
  segment.h
  server.h
  sizeEvent.h
  statistic.h
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
  windowSettings.h
  zoom.h
  )

set(EQUALIZERFABRIC_HEADERS
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

set(EQUALIZERFABRIC_SOURCES
  client.cpp
  colorMask.cpp
  configParams.cpp
  equalizer.cpp
  error.cpp
  errorRegistry.cpp
  eventType.cpp
  eye.cpp
  frame.cpp
  frameData.cpp
  frustum.cpp
  global.cpp
  iAttribute.cpp
  init.cpp
  object.cpp
  pixel.cpp
  projection.cpp
  range.cpp
  renderContext.cpp
  statistic.cpp
  subPixel.cpp
  swapBarrier.cpp
  viewport.cpp
  wall.cpp
  windowSettings.cpp
  zoom.cpp
  )

set(EQUALIZERFABRIC_LINK_LIBRARIES PUBLIC Lunchbox Collage vmmlib)
set(EQUALIZERFABRIC_INCLUDE_NAME eq/fabric)
set(EQUALIZERFABRIC_NAMESPACE eqfabric)
set(EQUALIZERFABRIC_OMIT_LIBRARY_HEADER ON)

list(APPEND CPPCHECK_EXTRA_ARGS -DEQFABRIC_API=)
common_library(EqualizerFabric)
if(CMAKE_COMPILER_IS_CLANG)
  target_compile_options(EqualizerFabric PUBLIC -Wno-overloaded-virtual)
endif()
