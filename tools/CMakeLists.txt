# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

include(ParseArguments)
include(PurpleInstallPDB)
include(PurpleExpandLibraries)

macro(EQ_ADD_TOOL NAME)
  parse_arguments(THIS "SOURCES;HEADERS;LINK_LIBRARIES" "" ${ARGN})
  set(THIS_TARGET ${NAME})
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS})
  add_executable(${THIS_TARGET} ${THIS_SOURCES} ${THIS_HEADERS})
  purple_expand_libraries(LINK_LIBRARIES ${THIS_LINK_LIBRARIES})
  target_link_libraries(${THIS_TARGET} ${LINK_LIBRARIES})
  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME} FOLDER "Tools")
  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT tools)
  purple_install_pdb(${THIS_TARGET} DESTINATION bin COMPONENT tools)
endmacro(EQ_ADD_TOOL NAME)

include_directories(${CMAKE_SOURCE_DIR}/examples/include
  ${CMAKE_SOURCE_DIR}/examples/eqPly)

if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIRS})
  set(GLEW_LIBRARY ${GLEW_MX_LIBRARIES})
else()
  set(GLEW_LIBRARY "GLEW_MX-Equalizer")
  add_definitions(-DGLEW_STATIC=1 -DGLEW_MX=1)
endif()

if(GPUSD_FOUND)
  add_definitions(-DEQ_USE_GPUSD)
endif()

if(WIN32)
  eq_add_tool(affinityCheck
    SOURCES affinityCheck/affinityCheck.cpp
    LINK_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_LIBRARIES}
    )
endif(WIN32)

eq_add_tool(configTool
  HEADERS configTool/configTool.h configTool/frame.h
  SOURCES configTool/configTool.cpp configTool/writeFromFile.cpp
  LINK_LIBRARIES shared Equalizer shared EqualizerServer
  )

eq_add_tool(eqPlyConverter
  HEADERS 
    ../examples/eqPly/ply.h
    ../examples/eqPly/vertexBufferBase.h
    ../examples/eqPly/vertexBufferData.h
    ../examples/eqPly/vertexBufferLeaf.h
    ../examples/eqPly/vertexBufferNode.h
    ../examples/eqPly/vertexBufferRoot.h
    ../examples/eqPly/vertexBufferState.h
    ../examples/eqPly/vertexData.h
  SOURCES eqPlyConverter/main.cpp
    ../examples/eqPly/plyfile.cpp
    ../examples/eqPly/vertexBufferBase.cpp
    ../examples/eqPly/vertexBufferLeaf.cpp
    ../examples/eqPly/vertexBufferNode.cpp
    ../examples/eqPly/vertexBufferRoot.cpp
    ../examples/eqPly/vertexBufferState.cpp
    ../examples/eqPly/vertexData.cpp
    LINK_LIBRARIES shared Equalizer
  )

eq_add_tool(eVolveConverter
  HEADERS
    eVolveConverter/codebase.h
    eVolveConverter/ddsbase.h
    eVolveConverter/eVolveConverter.h
    eVolveConverter/hlp.h
  SOURCES
    eVolveConverter/eVolveConverter.cpp
    eVolveConverter/ddsbase.cpp
  )

eq_add_tool(netperf
  SOURCES netperf/netperf.cpp
  LINK_LIBRARIES Collage
  )

eq_add_tool(netproxy
  SOURCES netproxy/netproxy.cpp
  LINK_LIBRARIES Collage
  )

eq_add_tool(windowAdmin
  SOURCES windowAdmin/main.cpp
  LINK_LIBRARIES shared EqualizerAdmin
  )

eq_add_tool(eqServer
  SOURCES server/eqServer.cpp
  LINK_LIBRARIES shared EqualizerServer
  )
