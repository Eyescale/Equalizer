# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

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
    ${OPENSCENEGRAPH_LIBRARIES}
  )
