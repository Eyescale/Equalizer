##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
# Copyright (c) 2010 Cedric Stalder <cedric.stalder@gmail.ch>
##
include(PurpleAddLibrary)
include(${CMAKE_CURRENT_SOURCE_DIR}/files.cmake)

source_group( "plugin" FILES ${PLUGIN_HEADERS} )
source_group( "base" FILES ${COBASE_FORWARD_HEADERS} ${COBASE_HEADERS} ${COBASE_SOURCES})

purple_add_library(co_base ${CO_LIB_VARIANT}
  HEADERS_PREFIX "co/base/"
  FORWARD HEADERS
    ${COBASE_FORWARD_HEADERS}
  PRECOMPILE_HEADERS
    <pthread.h>
    precompile.h
  SOURCES
    ${COBASE_SOURCES}
    ${COBASE_HEADERS}
  LINK_LIBRARIES
    ${PTHREAD_LIBRARIES}
  )
