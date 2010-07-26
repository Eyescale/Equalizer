# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

find_package(OpenSceneGraph REQUIRED)

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

target_link_libraries(osgScaleViewer
  ${EQ_LIBRARIES}
  ${OPENSCENEGRAPH_LIBRARIES}
  )

install(TARGETS osgScaleViewer DESTINATION bin)
