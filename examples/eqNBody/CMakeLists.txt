# Copyright (c) 2010-2016 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>

include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
remove_definitions(${EQ_DEFINITIONS}) # WAR bug in FindCUDA.cmake

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:LIBC;LIBCMT;MSVCRT)
endif()

cuda_compile(NBODY_FILES nbody.cu)

set(EQNBODY_HEADERS
  channel.h
  client.h
  config.h
  configEvent.h
  controller.h
  frameData.h
  initData.h
  nbody.h
  node.h
  pipe.h
  render_particles.h
  sharedData.h
  sharedDataProxy.h
  window.h)

set(EQNBODY_SOURCES
  ${NBODY_FILES}
  channel.cpp
  client.cpp
  config.cpp
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

set(EQNBODY_DATA nbody_kernel.cu)

set(EQNBODY_LINK_LIBRARIES ${CUDA_LIBRARIES} Equalizer)

common_application(eqNBody GUI EXAMPLE)
