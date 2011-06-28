# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.com>

include(PurpleForwardHeaders)
include(PurpleAddLibrary)
include(PurpleAddAmalgamation)

option(EQUALIZER_BUILD_STATIC_LIBS "Build static Equalizer libraries" OFF)
mark_as_advanced(EQUALIZER_BUILD_STATIC_LIBS)
if(NOT EQUALIZER_BUILD_STATIC_LIBS)
  set(EQ_LIB_VARIANT SHARED)
endif()

option(EQUALIZER_BUILD_MODULARIZED "Build intermediate Equalizer libraries" OFF)
mark_as_advanced(EQUALIZER_BUILD_MODULARIZED)
if(NOT EQUALIZER_BUILD_MODULARIZED)
  list(APPEND EQ_LIB_VARIANT EXCLUDE_FROM_ALL)
endif(NOT EQUALIZER_BUILD_MODULARIZED)

add_subdirectory(collage)
add_subdirectory(fabric)
add_subdirectory(client)
add_subdirectory(admin)
add_subdirectory(server)

