
set(HEADERS
  admin.h
  base.h
  canvas.h
  channel.h
  client.h
  config.h
  init.h
  layout.h
  node.h
  observer.h
  packets.h
  pipe.h
  segment.h
  server.h
  types.h
  view.h
  visitorResult.h
  window.h
  )

set(SOURCES
  canvas.cpp
  channel.cpp
  client.cpp
  config.cpp
  init.cpp
  layout.cpp
  node.cpp
  nodeFactory.cpp
  observer.cpp
  pipe.cpp
  segment.cpp
  server.cpp
  view.cpp
  window.cpp
  )

if(MSVC)
  set(SOURCES ${SOURCES} ${HEADERS})
endif(MSVC)

forward_headers(eq/admin ${HEADERS})

add_definitions(-DEQADMIN_EXPORTS)

add_library(EqualizerAdmin SHARED ${SOURCES})
target_link_libraries(EqualizerAdmin Equalizer)

install(TARGETS EqualizerAdmin
  ARCHIVE DESTINATION lib COMPONENT lib
  LIBRARY DESTINATION lib COMPONENT lib
  RUNTIME DESTINATION bin COMPONENT lib
  )

install(FILES ${HEADERS} DESTINATION include/eq/admin COMPONENT dev)
if(MSVC)
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/EqualizerAdmin.pdb DESTINATION bin COMPONENT dev)
endif(MSVC)
