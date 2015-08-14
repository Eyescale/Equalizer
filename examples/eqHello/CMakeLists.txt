# Copyright (c) 2010-2015 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>
#                         Daniel Nachbaur <danielnachbaur@gmail.com>

include_directories(${CMAKE_CURRENT_BINARY_DIR})

eq_add_example(eqHello
  SOURCES hello.cpp
  SHADERS
    vertexShader.glsl
    fragmentShader.glsl
  LINK_LIBRARIES Sequel
  )
