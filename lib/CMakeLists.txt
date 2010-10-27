# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

include(${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/files.cmake)

set(HEADERS
  ${BASE_HEADERS}
  ${CLIENT_HEADERS}
  ${FABRIC_HEADERS}
  ${NET_HEADERS}
  ${PLUGIN_HEADERS}
  ${UTIL_HEADERS}
  base.h
  client.h
  eq.h
  fabric.h
  net.h
  util.h
  admin.h
  )

set(SOURCES
  ${BASE_SOURCES}
  ${CLIENT_SOURCES}
  ${FABRIC_SOURCES}
  ${NET_SOURCES}
  ${UTIL_SOURCES}
  ${COMPRESSOR_SOURCES}
  )

if(MSVC)
  source_group(base\\headers FILES ${BASE_HEADERS})
  source_group(base\\sources FILES ${BASE_SOURCES})
  source_group(client\\headers FILES ${CLIENT_HEADERS})
  source_group(client\\sources FILES ${CLIENT_SOURCES})
  source_group(fabric\\headers FILES ${FABRIC_HEADERS})
  source_group(fabric\\sources FILES ${FABRIC_SOURCES})
  source_group(net\\headers FILES ${NET_HEADERS})
  source_group(net\\sources FILES ${NET_SOURCES})
  source_group(plugin\\headers FILES ${PLUGIN_HEADERS})
  source_group(util\\headers FILES ${UTIL_HEADERS})
  source_group(util\\sources FILES ${UTIL_SOURCES})
  source_group(compressor\\sources FILES ${COMPRESSOR_SOURCES})
  set(SOURCES ${SOURCES} ${HEADERS})
endif(MSVC)

forward_headers(eq ${HEADERS})

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

add_library(Equalizer SHARED ${SOURCES})
target_link_libraries(Equalizer
  ${PTHREAD_LIBRARIES}
  ${Boost_LIBRARIES}
  ${GLEW_LIBRARY}
  ${CARBON_LIBRARY}
  ${X11_LIBRARIES}
  ${GL_LIBRARIES}
  ${CUDA_LIBRARIES}
  )

install(TARGETS Equalizer
  ARCHIVE DESTINATION lib COMPONENT lib
  LIBRARY DESTINATION lib COMPONENT lib
  RUNTIME DESTINATION bin COMPONENT lib
  )

foreach(HEADER ${HEADERS})
  string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
  install(FILES ${HEADER} DESTINATION include/eq/${DIR} COMPONENT dev)
endforeach(HEADER ${HEADERS})

configure_file(Equalizer.pc.in Equalizer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Equalizer.pc DESTINATION lib/pkgconfig COMPONENT lib)
if(MSVC)
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/Equalizer.pdb DESTINATION bin COMPONENT dev)
endif(MSVC)
