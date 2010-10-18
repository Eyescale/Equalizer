# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

set(HEADER 
  cameraAnimation.h
  channel.h
  config.h
  configEvent.h
  eqPly.h
  fragmentShader_glsl.h
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
  vertexShader_glsl.h
  view.h
  window.h)
  
add_executable(eqPly MACOSX_BUNDLE
  ${HEADER}
  cameraAnimation.cpp
  channel.cpp
  config.cpp
  configEvent.cpp
  eqPly.cpp
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
  )

if(MSVC)
  SOURCE_GROUP("CMake Files" FILES CMakeLists.txt)
endif(MSVC)
  
target_link_libraries(eqPly Equalizer EqualizerAdmin)

install(TARGETS eqPly DESTINATION bin)

install(FILES rockerArm.ply screwdriver.ply
  DESTINATION share/Equalizer/data COMPONENT data)
