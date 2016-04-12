# Copyright (c) 2010-2016 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>

include_directories(SYSTEM ${OPENSCENEGRAPH_INCLUDE_DIRS})

add_definitions(-DEQ_IGNORE_GLEW)

set(OSGSCALEVIEWER_HEADERS
  channel.h
  config.h
  frameData.h
  initData.h
  node.h
  nodeFactory.h
  osgScaleViewer.h
  pipe.h
  quad.h
  sceneReader.h
  sceneView.h
  tracker.h
  util.h
  window.h)

set(OSGSCALEVIEWER_SOURCES
  main.cpp
  osgScaleViewer.cpp
  channel.cpp
  config.cpp
  frameData.cpp
  initData.cpp
  node.cpp
  pipe.cpp
  sceneReader.cpp
  sceneView.cpp
  tracker.cpp
  window.cpp
  quad.cpp)

set(OSGSCALEVIEWER_LINK_LIBRARIES ${OPENSCENEGRAPH_LIBRARIES} Equalizer)
common_application(osgScaleViewer GUI EXAMPLE)
