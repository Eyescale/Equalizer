# Copyright (c) 2010-2016 Stefan Eilemann <eile@eyescale.ch>

if(NOT EQ_GLX_USED)
  message(STATUS "GLX window system not supported: Skipping example eqCPU")
  return()
endif()

set(EQCPU_HEADERS channel.h pipe.h window.h)
set(EQCPU_SOURCES channel.cpp main.cpp window.cpp)
set(EQCPU_LINK_LIBRARIES ${X11_LIBRARIES} Equalizer)
common_application(eqCPU GUI EXAMPLE)
