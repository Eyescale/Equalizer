# Copyright (c) 2011-2012 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
					${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}/examples
                    ${CMAKE_SOURCE_DIR}/examples/eqPly)

eq_add_example(seqPly
  HEADERS
  SOURCES
    application.cpp
    application.h
    main.cpp
    renderer.cpp
    renderer.h
    ../eqPly/frameData.h
    ../eqPly/frameData.cpp
  LINK_LIBRARIES
    ${EQUALIZER_SEQUEL_LIBRARY}
    plylib
  )
