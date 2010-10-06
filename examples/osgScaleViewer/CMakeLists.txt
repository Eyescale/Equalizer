# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

find_package(OpenSceneGraph 2.9.8 COMPONENTS osgDB osgUtil)

if(NOT OPENSCENEGRAPH_FOUND)
  message(WARNING "OSG not found: Example 'osgScaleViewer' will be skipped!")
  return()
endif(NOT OPENSCENEGRAPH_FOUND)

include_directories(SYSTEM ${OPENSCENEGRAPH_INCLUDE_DIRS})

add_executable(osgScaleViewer MACOSX_BUNDLE
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
