# Copyright (c) 2011-2016 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${PROJECT_SOURCE_DIR}/examples/eqPly)

set(SEQPLY_HEADERS application.h renderer.h)
set(SEQPLY_SOURCES
  application.cpp
  main.cpp
  renderer.cpp
  ../eqPly/frameData.h
  ../eqPly/frameData.cpp)

set(SEQPLY_LINK_LIBRARIES Sequel triply ${Boost_PROGRAM_OPTIONS_LIBRARY})
common_application(seqPly GUI EXAMPLE)
