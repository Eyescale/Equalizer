# Copyright (c) 2011-2012 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../eqPly/)

set(KD_HEADERS
    ../eqPly/ply.h
    ../eqPly/vertexBufferBase.h
    ../eqPly/vertexBufferData.h
    ../eqPly/vertexBufferDist.h
    ../eqPly/vertexBufferLeaf.h
    ../eqPly/vertexBufferNode.h
    ../eqPly/vertexBufferRoot.h
    ../eqPly/vertexBufferState.h
    ../eqPly/vertexData.h
)
set(KD_SOURCES
    ../eqPly/plyfile.cpp
    ../eqPly/vertexBufferBase.cpp
    ../eqPly/vertexBufferDist.cpp
    ../eqPly/vertexBufferLeaf.cpp
    ../eqPly/vertexBufferNode.cpp
    ../eqPly/vertexBufferRoot.cpp
    ../eqPly/vertexBufferState.cpp
    ../eqPly/vertexData.cpp
)

eq_add_example(seqPly
  HEADERS
    ${KD_HEADERS}
  SOURCES
    main.cpp
    renderer.h
    renderer.cpp
    seqPly.h
    seqPly.cpp
    ../eqPly/frameData.h
    ../eqPly/frameData.cpp
    ${KD_SOURCES}
  LINK_LIBRARIES
    ${EQUALIZER_SEQUEL_LIBRARY}
  )

source_group(kdTree FILES ${KD_HEADERS} ${KD_SOURCES})
