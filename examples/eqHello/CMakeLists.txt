# Copyright (c) 2010-2016 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>
#                         Daniel Nachbaur <danielnachbaur@gmail.com>

set(EQHELLO_SOURCES hello.cpp)
set(EQHELLO_SHADERS vertexShader.glsl fragmentShader.glsl)
set(EQHELLO_LINK_LIBRARIES Sequel)

common_application(eqHello GUI EXAMPLE)
