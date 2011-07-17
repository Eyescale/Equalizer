# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)
include(files.cmake)

purple_add_library(EqualizerFabric SHARED
  HEADERS_PREFIX "eq/fabric/"
  HEADERS ${EQ_FABRIC_FORWARD_HEADERS}
  SOURCES ${EQ_FABRIC_SOURCES} ${EQ_FABRIC_HEADERS}
  LINK_LIBRARIES Collage
  )
source_group(\\ REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt)
source_group(fabric FILES ${EQ_FABRIC_FORWARD_HEADERS} ${EQ_FABRIC_HEADERS}
             ${EQ_FABRIC_SOURCES})
