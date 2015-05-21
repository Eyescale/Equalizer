
# Copyright (c) 2011-2015 Stefan Eilemann <eile@eyescale.ch>

set(TRIPLY_PUBLIC_HEADERS
  api.h
  ply.h
  typedefs.h
  vertexBufferBase.h
  vertexBufferData.h
  vertexBufferDist.h
  vertexBufferLeaf.h
  vertexBufferNode.h
  vertexBufferRoot.h
  vertexBufferState.h
  vertexData.h)

set(TRIPLY_SOURCES
  plyfile.cpp
  vertexBufferBase.cpp
  vertexBufferDist.cpp
  vertexBufferLeaf.cpp
  vertexBufferNode.cpp
  vertexBufferRoot.cpp
  vertexBufferState.cpp
  vertexData.cpp)

set(TRIPLY_LINK_LIBRARIES
  ${COLLAGE_LIBRARIES} ${EQ_GL_LIBRARIES} ${GLEW_LIBRARY})
set(PROJECT_INCLUDE_NAME triply)

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_SOURCE_DIR}/plyfile.cpp)

add_definitions(-DPLYLIB_SHARED)
common_library(triply)
