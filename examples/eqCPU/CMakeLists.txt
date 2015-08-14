# Copyright (c) 2010-2015 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  return()
endif()
if(NOT EQ_GLX_USED)
  message(STATUS "GLX window system not supported: Skipping example eqCPU")
  return()
endif()

eq_add_example(eqCPU
  HEADERS channel.h pipe.h window.h
  SOURCES channel.cpp main.cpp window.cpp
  LINK_LIBRARIES ${X11_LIBRARIES})
