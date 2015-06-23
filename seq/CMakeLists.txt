# Copyright (c) 2011-2015 Stefan Eilemann <eile@eyescale.ch>

set(SEQUEL_PUBLIC_HEADERS
    application.h
    error.h
    objectType.h
    renderer.h
    sequel.h
    types.h
    viewData.h
  )

set(SEQUEL_HEADERS
    detail/channel.h
    detail/config.h
    detail/masterConfig.h
    detail/node.h
    detail/objectMap.h
    detail/pipe.h
    detail/renderer.h
    detail/slaveConfig.h
    detail/view.h
    detail/window.h
  )

set(SEQUEL_SOURCES
    application.cpp
    detail/application.cpp
    detail/channel.cpp
    detail/config.cpp
    detail/masterConfig.cpp
    detail/node.cpp
    detail/objectMap.cpp
    detail/pipe.cpp
    detail/renderer.cpp
    detail/slaveConfig.cpp
    detail/view.cpp
    detail/window.cpp
    error.cpp
    renderer.cpp
    viewData.cpp
  )

set(SEQUEL_LINK_LIBRARIES Equalizer)
set(SEQUEL_NAMESPACE seq)
set(SEQUEL_INCLUDE_NAME seq)
common_library(Sequel)
