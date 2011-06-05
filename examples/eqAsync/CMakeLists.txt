# Copyright (c)      2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Maxim Makhinya <maxmah@gmail.com>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(EQASYNC_SYSTEM_SOURCES)
set(EQASYNC_SYSTEM_HEADERS)
if(EQ_AGL_USED)
  set(EQASYNC_SYSTEM_HEADERS aglWindowShared.h)
  set(EQASYNC_SYSTEM_SOURCES aglWindowShared.cpp)
endif()

eq_add_example(eqAsync
  HEADERS
    asyncFetcher.h
    eqAsync.h
    ${EQASYNC_SYSTEM_HEADERS}
  SOURCES
    asyncFetcher.cpp
    eqAsync.cpp
    main.cpp
    ${EQASYNC_SYSTEM_SOURCES}
  LINK_LIBRARIES
    Equalizer
  )
