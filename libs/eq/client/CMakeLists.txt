##
# Path : libs/client/CMakeLists.txt
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>
#               2011 Cedric Stalder <cedric.stalder@gmail.ch>
##

# TODO: resolve two-way dependancy between client and util!
include(configure.cmake)
include(files.cmake)
include(../util/files.cmake)
include(compressor/files.cmake)

add_definitions(-DEQ_PLUGIN_BUILD)

if(APPLE)
  if(EQ_AGL_USED)
    find_library(CARBON_LIBRARY Carbon)
    set(GL_LIBRARIES "${OPENGL_gl_LIBRARY}")
  endif(EQ_AGL_USED)
  if(EQ_GLX_USED)
    set(GL_LIBRARIES ${GL_LIBRARIES} GL)
  endif(EQ_GLX_USED)
else(APPLE)
  set(GL_LIBRARIES "${OPENGL_gl_LIBRARY}")
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

source_group(fabric REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt)
source_group(plugin FILES ${PLUGIN_HEADERS} )
source_group(util FILES ${EQ_UTIL_PUBLIC_HEADERS} ${EQ_UTIL_HEADERS} ${EQ_UTIL_SOURCES} )
source_group(downloader FILES ${EQ_COMPRESSOR_HEADERS} ${EQ_COMPRESSOR_SOURCES})
source_group(client FILES eq.h ${CLIENT_HEADERS} ${CLIENT_SOURCES})
source_group(agl FILES ${AGL_HEADERS} ${AGL_SOURCES})
source_group(glx FILES ${GLX_HEADERS} ${GLX_SOURCES})
source_group(wgl FILES ${WGL_HEADERS} ${WGL_SOURCES})

purple_add_library(Equalizer SHARED
  HEADERS_PREFIX "eq/client"
  HEADERS ${CLIENT_HEADERS} ${EQ_UTIL_PUBLIC_HEADERS}
  SOURCES ${CLIENT_SOURCES} ${EQ_UTIL_SOURCES} ${EQ_COMPRESSOR_SOURCES}
  LINK_LIBRARIES
    shared EqualizerFabric
    ${GLEW_LIBRARY}
    ${WINDOWING_LIBRARIES}
    ${GL_LIBRARIES}
    ${CUDA_LIBRARIES}
  )

option(EQUALIZER_OLD_CLIENT_LAYOUT "Install header files as in v1.0 directory layout" OFF)
if(EQUALIZER_OLD_CLIENT_LAYOUT)
  install(FILES ${CLIENT_HEADERS} DESTINATION include/eq/ COMPONENT eqdev)
endif()

