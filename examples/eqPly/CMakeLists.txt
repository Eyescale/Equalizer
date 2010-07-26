# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

add_executable(eqPly
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

target_link_libraries(eqPly Equalizer EqualizerAdmin)

install(TARGETS eqPly DESTINATION bin)

install(FILES rockerArm.ply screwdriver.ply
  DESTINATION shared/Equalizer/data COMPONENT data)
