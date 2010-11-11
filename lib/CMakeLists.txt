# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)
include(PurpleAddAmalgamation)
include(${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/files.cmake)

source_group(client FILES ${CLIENT_HEADERS} ${CLIENT_SOURCES})
source_group(plugin FILES ${PLUGIN_HEADERS})
source_group(util FILES ${UTIL_HEADERS} ${UTIL_SOURCES})
source_group(compressor FILES ${COMPRESSOR_SOURCES})

add_definitions(-DEQ_EXPORTS -DEQ_PLUGIN_BUILD -DGLEW_STATIC)

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

add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(fabric)

# TODO: split this library
purple_add_library(eq_transitional SHARED
  FORWARD HEADERS
    #${BASE_HEADERS}
    ${CLIENT_HEADERS}
    #${FABRIC_HEADERS}
    #${NET_HEADERS}
    ${PLUGIN_HEADERS}
    ${UTIL_HEADERS}
    #base.h
    client.h
    eq.h
    #fabric.h
    #net.h
    util.h
    admin.h
  SOURCES
    #${BASE_SOURCES}
    ${CLIENT_SOURCES}
    #${FABRIC_SOURCES}
    #${NET_SOURCES}
    ${UTIL_SOURCES}
    ${COMPRESSOR_SOURCES}
  LINK_LIBRARIES
    shared eq_base
    shared eq_net
    shared eq_fabric
    ${PTHREAD_LIBRARIES}
    ${Boost_LIBRARIES}
    ${GLEW_LIBRARY}
    ${CARBON_LIBRARY}
    ${X11_LIBRARIES}
    ${GL_LIBRARIES}
    ${CUDA_LIBRARIES}
  HEADERS_PREFIX "eq/"
  )

purple_add_amalgamation(Equalizer
    shared eq_base
    shared eq_net
    shared eq_fabric
    shared eq_transitional
  LINK_LIBRARIES
    ${PTHREAD_LIBRARIES}
    ${Boost_LIBRARIES}
    ${GLEW_LIBRARY}
    ${CARBON_LIBRARY}
    ${X11_LIBRARIES}
    ${GL_LIBRARIES}
    ${CUDA_LIBRARIES}
  )

# TODO: this will be incorporated into PURPLE_ADD_LIBRARY
configure_file(Equalizer.pc.in Equalizer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Equalizer.pc DESTINATION lib/pkgconfig COMPONENT lib)
if(MSVC_IDE)
  install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/Equalizer.pdb DESTINATION bin COMPONENT dev)
endif(MSVC_IDE)
