# Copyright (c) 2011-2014 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/examples/eqPly)

eq_add_example(seqPly
  HEADERS
    application.h
    renderer.h
  SOURCES
    application.cpp
    main.cpp
    renderer.cpp
    ../eqPly/frameData.h
    ../eqPly/frameData.cpp
  LINK_LIBRARIES
    Sequel triply
  )
