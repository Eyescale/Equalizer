# Copyright (c) 2010-2015 Stefan Eilemann <eile@eyescale.ch>

if(NOT EQ_GLX_USED)
  message(STATUS "GLX window system not supported: Skipping example eqCPU")
  return()
endif(NOT EQ_GLX_USED)

eq_add_example(eqCPU
  HEADERS channel.h pipe.h window.h
  SOURCES channel.cpp main.cpp window.cpp)
