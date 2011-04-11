# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)

purple_add_library(Sequel SHARED
  HEADERS_PREFIX "eq/sequel/"
  FORWARD HEADERS
    api.h
    application.h
    sequel.h
  SOURCES
    application.cpp
  LINK_LIBRARIES Equalizer
  )
set_target_properties(lib_Sequel_shared PROPERTIES
  FOLDER "Libraries" PROJECT_LABEL "Sequel")
source_group(\\ REGULAR_EXPRESSION .*)
