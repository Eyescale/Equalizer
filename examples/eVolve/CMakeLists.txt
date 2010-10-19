# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

set(HEADERS eVolve.h)

set(SOURCES
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
  window.cpp)

add_executable(eVolve MACOSX_BUNDLE ${HEADERS} ${SOURCES})

target_link_libraries(eVolve Equalizer)

install(TARGETS eVolve DESTINATION bin COMPONENT apps)

install(FILES Bucky32x32x32_d.raw Bucky32x32x32_d.raw.vhf
  DESTINATION share/Equalizer/data COMPONENT data)
install(FILES ${HEADERS} ${SOURCES} DESTINATION share/Equalizer/examples/eVolve COMPONENT examples)
