##
# Path : libs/client/CMakeLists.txt
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>
#               2011 Cedric Stalder <cedric.stalder@gmail.ch>
##

# TODO: resolve two-way dependancy between client and util!
include(configure.cmake)
include(files.cmake)
include(util/files.cmake)
include(compressor/files.cmake)
include(../fabric/files.cmake)

option(EQUALIZER_BUILD_STATIC_LIBS "Build static Equalizer libraries" OFF)
if(NOT EQUALIZER_BUILD_STATIC_LIBS)
  set(EQ_LIB_VARIANT SHARED)
endif()

option(EQUALIZER_BUILD_MODULARIZED "Build intermediate Equalizer libraries" OFF)
if(NOT EQUALIZER_BUILD_MODULARIZED)
  list(APPEND EQ_LIB_VARIANT EXCLUDE_FROM_ALL)
endif(NOT EQUALIZER_BUILD_MODULARIZED)

add_definitions(-DEQ_EXPORTS -DEQ_PLUGIN_BUILD)

if(APPLE)
  if(EQ_AGL_USED)
    find_library(CARBON_LIBRARY Carbon)
    set(GL_LIBRARIES "${OPENGL_LIBRARIES}")
  endif(EQ_AGL_USED)
  if(EQ_GLX_USED)
    set(GL_LIBRARIES ${GL_LIBRARIES} GL)
  endif(EQ_GLX_USED)
else(APPLE)
  set(GL_LIBRARIES "${OPENGL_LIBRARIES}")
endif(APPLE)
if(NOT CUDA_FOUND)
  set(CUDA_LIBRARIES)
endif(NOT CUDA_FOUND)

set(WINDOWING_LIBRARIES)
if(EQ_AGL_USED) 
  set(WINDOWING_LIBRARIES ${WINDOWING_LIBRARIES} ${CARBON_LIBRARY}) 
endif()
if(EQ_GLX_USED) 
  set(WINDOWING_LIBRARIES ${WINDOWING_LIBRARIES} ${X11_LIBRARIES}) 
endif()

purple_forward_headers("eq/" ${EQ_UTIL_FORWARD_HEADERS})

purple_add_library(eq_fabric ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/fabric/"
  FORWARD HEADERS ${EQ_FABRIC_FORWARD_HEADERS}
  SOURCES ${EQ_FABRIC_SOURCES}
  LINK_LIBRARIES Collage
  )

source_group(fabric REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt)
source_group(plugin FILES ${PLUGIN_HEADERS} )
source_group(util FILES ${EQ_UTIL_FORWARD_HEADERS} ${EQ_UTIL_HEADERS} ${EQ_UTIL_SOURCES} )
source_group(downloader FILES ${EQ_COMPRESSOR_HEADERS} ${EQ_COMPRESSOR_SOURCES})
source_group(client FILES eq.h ${CLIENT_HEADERS} ${CLIENT_SOURCES})

purple_add_library(eq_client ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/"
  FORWARD HEADERS
    eq.h
    ${CLIENT_HEADERS}
    ${EQ_UTIL_FORWARD_HEADERS}
  SOURCES
    ${CLIENT_SOURCES}
    ${EQ_UTIL_SOURCES}
    ${EQ_COMPRESSOR_SOURCES}
  LINK_LIBRARIES
    Collage
    shared eq_fabric
    ${GLEW_LIBRARY}
    ${WINDOWING_LIBRARIES}
    ${GL_LIBRARIES}
    ${CUDA_LIBRARIES}
  )

purple_add_amalgamation(Equalizer eq ${SHORT_VERSION} ${API_VERSION}
  shared eq_fabric
  shared eq_client
  )

# TODO: this will be incorporated into PURPLE_ADD_LIBRARY
configure_file(Equalizer.pc.in Equalizer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Equalizer.pc DESTINATION lib/pkgconfig COMPONENT eqlib)
