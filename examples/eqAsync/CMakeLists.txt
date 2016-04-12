# Copyright (c) 2010-2016 Maxim Makhinya <maxmah@gmail.com>
#                         Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan.Eilemann@epfl.ch

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

set(EQASYNC_HEADERS asyncFetcher.h eqAsync.h)
set(EQASYNC_SOURCES asyncFetcher.cpp eqAsync.cpp main.cpp)
set(EQASYNC_LINK_LIBRARIES ${CARBON_LIBRARY} Equalizer)
common_application(eqAsync GUI EXAMPLE)
