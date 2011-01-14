##
# Path : libs/collage/CMakeLists.txt
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.ch>
##

include(files.cmake)
include(configure.cmake)

option(COLLAGE_BUILD_STATIC_LIBS "Build static Collage libraries" OFF)
if(NOT COLLAGE_BUILD_STATIC_LIBS)
  set(CO_LIB_VARIANT SHARED)
endif()

option(COLLAGE_BUILD_MODULARIZED "Build intermediate Collage libraries" OFF)
if(NOT COLLAGE_BUILD_MODULARIZED)
  list(APPEND CO_LIB_VARIANT EXCLUDE_FROM_ALL)
endif(NOT COLLAGE_BUILD_MODULARIZED)

set(CO_ADD_LINKLIB)

if(WIN32)
  list(APPEND CO_ADD_LINKLIB Ws2_32 mswsock )
endif(WIN32)

add_definitions(-DCO_EXPORTS -DEQ_PLUGIN_BUILD )

add_subdirectory( base )

if(Boost_FOUND)
  list(APPEND CO_SOURCES rspConnection.cpp)
  list(APPEND CO_ADD_LINKLIB ${Boost_SYSTEM_LIBRARY})
endif(Boost_FOUND)

if(MSVC)
  list(APPEND CO_SOURCES pgmConnection.cpp)
  list(APPEND CO_ADD_LINKLIB Ws2_32) 
endif(MSVC)

if(WIN32)
  list(APPEND CO_ADD_LINKLIB mswsock) 
endif(WIN32)

#TO-DO Add base directory. The problem is about the path of file here.
#      Maybe remove base\CMakeLists.txt and build base lib here. 
source_group("plugin" FILES ${PLUGIN_HEADERS} )
source_group("collage" FILES ${CO_FORWARD_HEADERS} ${CO_HEADERS} ${CO_SOURCES} )
source_group("compressor" FILES ${CO_COMPRESSOR_HEADERS} ${CO_COMPRESSOR_SOURCES})

purple_add_library(co ${CO_LIB_VARIANT}
  HEADERS_PREFIX "co/"
  FORWARD HEADERS
    ${CO_FORWARD_HEADERS}
    ${PLUGIN_HEADERS}
  PRECOMPILE_HEADERS
    precompile.h
  SOURCES
    ${CO_HEADERS}
    ${CO_SOURCES}
    ${CO_COMPRESSOR_HEADERS} 
    ${CO_COMPRESSOR_SOURCES}
  LINK_LIBRARIES
    shared co_base
    ${CO_ADD_LINKLIB}
  )
  
purple_add_amalgamation(Collage
    shared co_base
    shared co
  )

# TODO: this will be incorporated into PURPLE_ADD_LIBRARY
configure_file(Collage.pc.in Collage.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Collage.pc DESTINATION lib/pkgconfig COMPONENT lib)
