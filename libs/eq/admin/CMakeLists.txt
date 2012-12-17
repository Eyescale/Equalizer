# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

include(PurpleAddLibrary)

set(ADMIN_HEADERS
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
    pipe.h
    segment.h
    server.h
    types.h
    view.h
    visitorResult.h
    window.h
)

set(ADMIN_SOURCES
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
)

source_group(fabric REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt ${ADMIN_HEADERS} ${ADMIN_SOURCES})

purple_add_library(EqualizerAdmin SHARED
  HEADERS_PREFIX "eq/admin/"
  HEADERS ${ADMIN_HEADERS}
  SOURCES ${ADMIN_SOURCES}
  LINK_LIBRARIES shared EqualizerFabric
  )
