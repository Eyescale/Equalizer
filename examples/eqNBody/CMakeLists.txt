# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

if(NOT CUDA_FOUND)
  message(STATUS "CUDA not found: Example 'eqNBody' will be skipped!")
  return()
endif(NOT CUDA_FOUND)

cuda_compile(NBODY_FILES nbody.cu)

set(HEADERS channel.h)

set(SOURCES
  channel.cpp
  client.cpp
  config.cpp
  configEvent.cpp
  controller.cpp
  frameData.cpp
  initData.cpp
  main.cpp
  node.cpp
  pipe.cpp
  render_particles.cpp
  sharedData.cpp
  sharedDataProxy.cpp
  window.cpp)

add_executable(eqNBody MACOSX_BUNDLE ${NBODY_FILES} ${HEADERS} ${SOURCES})

target_link_libraries(eqNBody Equalizer ${CUDA_LIBRARIES})

install(TARGETS eqNBody DESTINATION bin COMPONENT apps)
install(FILES ${NBODY_FILES} ${HEADERS} ${SOURCES} DESTINATION share/Equalizer/examples/eqNBody COMPONENT examples)
