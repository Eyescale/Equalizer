# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

eq_add_example(eqPixelBench
  HEADERS
    channel.h
    config.h
    configEvent.h
    window.h
  SOURCES
    channel.cpp
    config.cpp
    configEvent.cpp
    main.cpp
    window.cpp
  LINK_LIBRARIES
    Equalizer
  )
