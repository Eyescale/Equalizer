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

source_group(fabric REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt ${ADMIN_HEADERS} ${ADMIN_SOURCES})

purple_add_library(eq_admin ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/admin/"
  FORWARD HEADERS ${ADMIN_HEADERS}
  SOURCES ${ADMIN_SOURCES}
  LINK_LIBRARIES Collage shared eq_fabric
  )

purple_add_amalgamation(EqualizerAdmin eq ${SHORT_VERSION} ${VERSION_ABI}
  shared eq_admin
  shared eq_fabric
  )
