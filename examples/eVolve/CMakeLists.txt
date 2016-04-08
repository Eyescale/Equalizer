# Copyright (c) 2010-2016 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(CPPCHECK_VERSION VERSION_LESS 1.68)
  list(APPEND CPPCHECK_EXTRA_ARGS
    --suppress=invalidscanf:${CMAKE_CURRENT_SOURCE_DIR}/rawVolModel.cpp)
else()
  list(APPEND CPPCHECK_EXTRA_ARGS
    --suppress=invalidscanf_libc:${CMAKE_CURRENT_SOURCE_DIR}/rawVolModel.cpp)
endif()

set(EVOLVE_HEADERS
  channel.h
  config.h
  eVolve.h
  error.h
  frameData.h
  glslShaders.h
  hlp.h
  imageOrderer.h
  initData.h
  localInitData.h
  node.h
  pipe.h
  rawVolModel.h
  rawVolModelRenderer.h
  sliceClipping.h
  window.h)

stringify_shaders( vertexShader.glsl fragmentShader.glsl)
set(EVOLVE_SOURCES
  channel.cpp
  config.cpp
  error.cpp
  eVolve.cpp
  frameData.cpp
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
  ${SHADER_SOURCES})

set(EVOLVE_DATA Bucky32x32x32_d.raw Bucky32x32x32_d.raw.vhf)
set(EVOLVE_LINK_LIBRARIES Equalizer ${Boost_PROGRAM_OPTIONS_LIBRARY})

common_application(eVolve GUI EXAMPLE)
