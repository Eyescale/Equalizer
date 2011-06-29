# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(KD_HEADERS
    ply.h
    vertexBufferBase.h
    vertexBufferData.h
    vertexBufferDist.h
    vertexBufferLeaf.h
    vertexBufferNode.h
    vertexBufferRoot.h
    vertexBufferState.h
    vertexData.h
)
set(KD_SOURCES
    plyfile.cpp
    vertexBufferBase.cpp
    vertexBufferDist.cpp
    vertexBufferLeaf.cpp
    vertexBufferNode.cpp
    vertexBufferRoot.cpp
    vertexData.cpp
)

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
    tracker.h
    typedefs.h
    view.h
    window.h
    ${KD_HEADERS}
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
    tracker.cpp
    view.cpp
    window.cpp
    ${KD_SOURCES}
  SHADERS
    vertexShader.glsl
    fragmentShader.glsl
  LINK_LIBRARIES
    ${EQUALIZER_ADMIN_LIBRARY}
  )

source_group(kdTree FILES ${KD_HEADERS} ${KD_SOURCES})
install(FILES rockerArm.ply screwdriver.ply DESTINATION share/Equalizer/data COMPONENT data)
install(FILES logo.rgb DESTINATION share/Equalizer/data COMPONENT data)
