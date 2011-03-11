# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

eq_add_example(eqPly
  HEADERS
    cameraAnimation.h
    channel.h
    config.h
    configEvent.h
    eqPly.h
    error.h
    frameData.h
    initData.h
    localInitData.h
    modelAssigner.h
    node.h
    node.h
    pipe.h
    ply.h
    tracker.h
    typedefs.h
    vertexBufferBase.h
    vertexBufferData.h
    vertexBufferDist.h
    vertexBufferLeaf.h
    vertexBufferNode.h
    vertexBufferRoot.h
    vertexBufferState.h
    vertexData.h
    view.h
    window.h
  SOURCES
    cameraAnimation.cpp
    channel.cpp
    config.cpp
    configEvent.cpp
    eqPly.cpp
    error.cpp
    frameData.cpp
    initData.cpp
    localInitData.cpp
    main.cpp
    node.cpp
    pipe.cpp
    plyfile.cpp
    tracker.cpp
    vertexBufferBase.cpp
    vertexBufferDist.cpp
    vertexBufferLeaf.cpp
    vertexBufferNode.cpp
    vertexBufferRoot.cpp
    vertexData.cpp
    view.cpp
    window.cpp
  SHADERS
    vertexShader.glsl
    fragmentShader.glsl
  LINK_LIBRARIES
    Equalizer
    shared EqualizerAdmin
  )

install(FILES rockerArm.ply screwdriver.ply DESTINATION share/Equalizer/data COMPONENT data)
