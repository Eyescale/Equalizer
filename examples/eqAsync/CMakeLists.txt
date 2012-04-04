# Copyright (c)      2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Maxim Makhinya <maxmah@gmail.com>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

eq_add_example(eqAsync
  HEADERS
    asyncFetcher.h
    eqAsync.h
  SOURCES
    asyncFetcher.cpp
    eqAsync.cpp
    main.cpp
  LINK_LIBRARIES
    ${CARBON_LIBRARY}
  )
