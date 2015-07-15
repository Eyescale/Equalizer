
# Copyright (c) 2013-2015 Stefan.Eilemann@epfl.ch

set(EQUALIZERADMIN_PUBLIC_HEADERS
  admin.h
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

set(EQUALIZERADMIN_SOURCES
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

set(EQUALIZERADMIN_LINK_LIBRARIES EqualizerFabric EqualizerServer)
set(EQUALIZERADMIN_INCLUDE_NAME eq/admin)
set(EQUALIZERADMIN_NAMESPACE eqadmin)
set(EQUALIZERADMIN_OMIT_LIBRARY_HEADER ON)

common_library(EqualizerAdmin)
target_compile_definitions(EqualizerAdmin PRIVATE EQUALIZERFABRIC_SHARED_INL)
