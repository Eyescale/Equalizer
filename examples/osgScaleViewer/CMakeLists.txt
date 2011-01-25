# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

if( NOT EQUALIZER_USE_OSG )
  return()
endif()
find_package(OpenSceneGraph 2.9.8 COMPONENTS osgDB osgUtil)
if(OSG_LIBRARY MATCHES "OSG_LIBRARY-NOTFOUND")
  message(STATUS "Skipping 'osgScaleViewer' example (OSG not found)")	 
  return()
endif()
if(${OPENSCENEGRAPH_VERSION} VERSION_LESS "2.9.8")
  message(STATUS "Skipping 'osgScaleViewer' example (OSG ${OPENSCENEGRAPH_VERSION} found, need >= 2.9.8)")
  return()
endif()


include_directories(SYSTEM ${OPENSCENEGRAPH_INCLUDE_DIRS})

eq_add_example(osgScaleViewer
  HEADERS
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
    window.h
  SOURCES
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
  LINK_LIBRARIES
    Equalizer
    ${OPENSCENEGRAPH_LIBRARIES}
  )
