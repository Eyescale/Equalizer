# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)
include(files.cmake)

purple_add_library(EqualizerFabric STATIC EXCLUDE_FROM_ALL
  HEADERS_PREFIX "eq/fabric/"
  FORWARD HEADERS ${EQ_FABRIC_FORWARD_HEADERS}
  SOURCES ${EQ_FABRIC_SOURCES} ${EQ_FABRIC_HEADERS}
  )
add_dependencies(lib_EqualizerFabric_static Collage) # need forwards, static lib
