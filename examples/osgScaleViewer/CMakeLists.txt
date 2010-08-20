# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

find_package(OpenSceneGraph)

if(NOT OPENSCENEGRAPH_FOUND)
  message(WARNING "OSG not found: Example 'osgScaleViewer' will be skipped!")
  return()
endif(NOT OPENSCENEGRAPH_FOUND)

# Ubuntu 10.04 comes with OSG version 2.8.1
# this version does not compile
if(${OPENSCENEGRAPH_VERSION} MATCHES "2.8.1")
  message(WARNING "OSG v2.8.1 found: Example 'osgScaleViewer' will be skipped!")
  return()
endif(${OPENSCENEGRAPH_VERSION} MATCHES "2.8.1")

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

add_executable(osgScaleViewer
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
  quad.cpp
  )

target_link_libraries(osgScaleViewer Equalizer ${OPENSCENEGRAPH_LIBRARIES})

install(TARGETS osgScaleViewer DESTINATION bin)
