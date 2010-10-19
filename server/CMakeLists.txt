# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

find_package(BISON REQUIRED)
find_package(FLEX  REQUIRED) 
 
bison_target(PARSER loader.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp 
     COMPILE_FLAGS "-l -p eqLoader_")

flex_target(LEXER loader.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(LEXER PARSER)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DEQSERVER_EXPORTS -DYY_NEVER_INTERACTIVE)

add_library(EqualizerServer SHARED
  ${BISON_PARSER_OUTPUTS}
  ${FLEX_LEXER_OUTPUTS}
  eqServer.cpp
  canvas.cpp
  channel.cpp
  channelUpdateVisitor.cpp
  compound.cpp
  compoundInitVisitor.cpp
  compoundUpdateDataVisitor.cpp
  compoundUpdateInputVisitor.cpp
  compoundUpdateOutputVisitor.cpp
  config.cpp
  configUpdateDataVisitor.cpp
  connectionDescription.cpp
  equalizers/dfrEqualizer.cpp
  equalizers/framerateEqualizer.cpp
  equalizers/equalizer.cpp
  equalizers/loadEqualizer.cpp
  equalizers/monitorEqualizer.cpp
  equalizers/viewEqualizer.cpp
  frame.cpp
  frameData.cpp
  frustum.cpp
  frustumData.cpp
  global.cpp
  init.cpp
  layout.cpp
  loader.cpp
  node.cpp
  nodeFactory.cpp
  observer.cpp
  parser.cpp
  pipe.cpp
  segment.cpp
  server.cpp
  startLocalServer.cpp
  swapBarrier.cpp
  view.cpp
  window.cpp
  )

target_link_libraries(EqualizerServer Equalizer)

install(TARGETS EqualizerServer
  ARCHIVE DESTINATION lib COMPONENT lib
  LIBRARY DESTINATION lib COMPONENT lib
  RUNTIME DESTINATION bin COMPONENT lib
  )
