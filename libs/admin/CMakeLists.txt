# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

include(PurpleAddLibrary)

purple_add_library(EqualizerAdmin SHARED
  HEADERS_PREFIX "eq/admin/"
  FORWARD HEADERS
    admin.h
    api.h
    base.h
    canvas.h
    channel.h
    client.h
    config.h
    init.h
    layout.h
    node.h
    observer.h
    packets.h
    pipe.h
    segment.h
    server.h
    types.h
    view.h
    visitorResult.h
    window.h
  SOURCES
    canvas.cpp
    channel.cpp
    client.cpp
    config.cpp
    init.cpp
    layout.cpp
    node.cpp
    nodeFactory.cpp
    observer.cpp
    pipe.cpp
    segment.cpp
    server.cpp
    view.cpp
    window.cpp
  LINK_LIBRARIES
    Equalizer
  )
