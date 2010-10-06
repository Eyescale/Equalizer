# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

add_executable(eVolve MACOSX_BUNDLE
  channel.cpp
  config.cpp
  eVolve.cpp
  frameData.cpp
  framesOrderer.cpp
  glslShaders.cpp
  initData.cpp
  localInitData.cpp
  main.cpp
  node.cpp
  pipe.cpp
  rawVolModel.cpp
  rawVolModelRenderer.cpp
  sliceClipping.cpp
  window.cpp
  )

target_link_libraries(eVolve Equalizer)

install(TARGETS eVolve DESTINATION bin)

install(FILES Bucky32x32x32_d.raw Bucky32x32x32_d.raw.vhf
  DESTINATION share/Equalizer/data COMPONENT data)
