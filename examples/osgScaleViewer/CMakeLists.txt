# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

find_package(OpenSceneGraph)

if(NOT OPENSCENEGRAPH_FOUND)

  message(STATUS "  Example 'osgScaleViewer' will be skipped!")

else(NOT OPENSCENEGRAPH_FOUND)

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

endif(NOT OPENSCENEGRAPH_FOUND)
