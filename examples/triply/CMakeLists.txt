
# Copyright (c) 2011-2015 Stefan Eilemann <eile@eyescale.ch>

set(TRIPLY_PUBLIC_HEADERS
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
  Collage vmmlib ${EQ_GL_LIBRARIES} ${GLEW_LIBRARY})
set(TRIPLY_INCLUDE_NAME triply)

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_SOURCE_DIR}/plyfile.cpp)

set(TRIPLY_OMIT_LIBRARY_HEADER ON)
set(TRIPLY_OMIT_EXPORT ON)
set(TRIPLY_NAMESPACE triply)
common_library(triply)
if(CMAKE_COMPILER_IS_CLANG)
  target_compile_options(triply PUBLIC -Wno-overloaded-virtual)
endif()
