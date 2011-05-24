# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)

purple_add_library(Sequel SHARED
  HEADERS_PREFIX "eq/sequel/"
  FORWARD HEADERS
    api.h
    application.h
    error.h
    objectFactory.h
    objectType.h
    renderer.h
    sequel.h
    types.h
  SOURCES
    application.cpp
    error.cpp
    renderer.cpp
    detail/application.cpp
    detail/channel.h
    detail/channel.cpp
    detail/config.h
    detail/config.cpp
    detail/masterConfig.h
    detail/masterConfig.cpp
    detail/node.h
    detail/node.cpp
    detail/objectMap.h
    detail/objectMap.cpp
    detail/pipe.h
    detail/pipe.cpp
    detail/slaveConfig.h
    detail/slaveConfig.cpp
  LINK_LIBRARIES Equalizer
  )
set_target_properties(lib_Sequel_shared PROPERTIES
  FOLDER "Libraries" PROJECT_LABEL "Sequel")
source_group(\\ REGULAR_EXPRESSION .*)
