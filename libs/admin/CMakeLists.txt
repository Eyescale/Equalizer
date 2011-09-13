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
    packets.h
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

purple_add_library(EqualizerAdmin SHARED
  HEADERS_PREFIX "eq/admin/"
  FORWARD HEADERS ${ADMIN_HEADERS}
  SOURCES ${ADMIN_SOURCES}
  LINK_LIBRARIES Equalizer
  )
set_target_properties(lib_EqualizerAdmin_shared PROPERTIES
  FOLDER "Libraries" PROJECT_LABEL "Admin")
