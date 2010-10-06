# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfig.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles.txt)

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
  find_library(CARBON_LIBRARY Carbon)
  if(X11_FOUND)
    set(GLX_LIBRARY GL)
  endif(X11_FOUND)
endif(APPLE)

add_library(Equalizer SHARED ${SOURCES})
target_link_libraries(Equalizer
  ${PTHREAD_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARY}
  ${CARBON_LIBRARY}
  ${X11_LIBRARIES}
  ${GLX_LIBRARY}
  )

install(TARGETS Equalizer
  ARCHIVE DESTINATION lib COMPONENT dev
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  )

foreach(HEADER ${HEADERS})
  string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
  install(FILES ${HEADER} DESTINATION include/eq/${DIR} COMPONENT dev)
endforeach(HEADER ${HEADERS})

configure_file(Equalizer.pc.in Equalizer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Equalizer.pc DESTINATION lib/pkgconfig COMPONENT dev)
