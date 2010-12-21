# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)
include(PurpleAddAmalgamation)
include(${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/files.cmake)

option(EQUALIZER_BUILD_STATIC_LIBS "Build static libraries" OFF)
if(NOT EQUALIZER_BUILD_STATIC_LIBS)
  set(EQ_LIB_VARIANT SHARED)
endif()

option(EQUALIZER_BUILD_MODULARIZED "Set this to OFF to build amalgamations only" OFF)
if(NOT EQUALIZER_BUILD_MODULARIZED)
  list(APPEND EQ_LIB_VARIANT EXCLUDE_FROM_ALL)
endif(NOT EQUALIZER_BUILD_MODULARIZED)

source_group(client FILES ${CLIENT_HEADERS} ${CLIENT_SOURCES})
source_group(util FILES ${UTIL_HEADERS} ${UTIL_SOURCES})
source_group(compressor FILES ${COMPRESSOR_SOURCES})

add_definitions(-DEQ_EXPORTS -DEQ_PLUGIN_BUILD -DGLEW_STATIC)

if(WIN32)
  list(APPEND ADD_LINKLIB Ws2_32 mswsock)
endif(WIN32)

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

add_subdirectory(collage/base)
add_subdirectory(collage)
add_subdirectory(client)
add_subdirectory(admin)
add_subdirectory(server)

# TODO: split this library
purple_add_library(eq ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/"
  HEADERS
    client/eq.h
    ${CLIENT_HEADERS}
  SOURCES
    ${COMPRESSOR_SOURCES}
  LINK_LIBRARIES
    shared eq_base
    shared eq_client
    ${GLEW_LIBRARY}
    ${CARBON_LIBRARY}
    ${X11_LIBRARIES}
    ${GL_LIBRARIES}
    ${CUDA_LIBRARIES}
  )

purple_add_amalgamation(Equalizer
  shared eq_base
  shared eq_net
  shared eq_fabric
  shared eq_client
  shared eq
  )

# TODO: this will be incorporated into PURPLE_ADD_LIBRARY
configure_file(Equalizer.pc.in Equalizer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Equalizer.pc DESTINATION lib/pkgconfig COMPONENT lib)
