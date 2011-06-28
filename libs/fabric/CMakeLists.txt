# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)
include(files.cmake)

purple_add_library(eq_fabric ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/fabric/"
  FORWARD HEADERS ${EQ_FABRIC_FORWARD_HEADERS}
  SOURCES ${EQ_FABRIC_SOURCES} ${EQ_FABRIC_HEADERS}
  )
source_group(\\ REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt)
source_group(fabric FILES ${EQ_FABRIC_FORWARD_HEADERS} ${EQ_FABRIC_HEADERS}
             ${EQ_FABRIC_SOURCES})
