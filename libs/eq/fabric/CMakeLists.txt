
# Copyright (c) 2011-2012 Stefan Eilemann <eile@eyescale.ch>

include(files.cmake)

purple_add_library(EqualizerFabric SHARED
  HEADERS_PREFIX "eq/fabric/"
  HEADERS ${EQ_FABRIC_PUBLIC_HEADERS}
  SOURCES ${EQ_FABRIC_SOURCES} ${EQ_FABRIC_HEADERS}
  LINK_LIBRARIES lib_Collage_shared
  )

source_group(\\ REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt)
source_group(fabric FILES ${EQ_FABRIC_PUBLIC_HEADERS} ${EQ_FABRIC_HEADERS}
             ${EQ_FABRIC_SOURCES})
