
# Copyright (c) 2010      Daniel Pfeifer  <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>
#               2010-2011 Cedric Stalder  <cedric.stalder@gmail.com>
#                    2011 Maxim  Makhinya <maxmah@gmail.com>
#                    2012, David Steiner  <steiner@ifi.uzh.ch>

include(PurpleAddLibrary)

option(MASSVOL_BUILD_STATIC_LIBS "Build static MassVolVis libraries" OFF)
mark_as_advanced(MASSVOL_BUILD_STATIC_LIBS)
if(NOT MASSVOL_BUILD_STATIC_LIBS)
  set(MSV_LIB_VARIANT SHARED)
endif()

option(MASSVOL_BUILD_MODULARIZED "Build intermediate MassVolVis libraries" OFF)
mark_as_advanced(MASSVOL_BUILD_MODULARIZED)
if(NOT MASSVOL_BUILD_MODULARIZED)
  list(APPEND MSV_LIB_VARIANT EXCLUDE_FROM_ALL)
endif(NOT MASSVOL_BUILD_MODULARIZED)

include(pthreads/CMakeLists.txt)

add_subdirectory(msv)
add_subdirectory(tclap)

