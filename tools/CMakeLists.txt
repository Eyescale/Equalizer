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

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/examples/include)

if(WIN32)
  eq_add_tool(affinityCheck
    SOURCES affinityCheck/affinityCheck.cpp
    LINK_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_LIBRARIES}
    )
endif(WIN32)

eq_add_tool(configTool
  HEADERS configTool/configTool.h configTool/frame.h
  SOURCES configTool/configTool.cpp configTool/writeFromFile.cpp
  LINK_LIBRARIES Equalizer shared EqualizerServer
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
  LINK_LIBRARIES Equalizer
  )

eq_add_tool(netproxy
  SOURCES netproxy/netproxy.cpp
  LINK_LIBRARIES Equalizer
  )

eq_add_tool(windowAdmin
  SOURCES windowAdmin/main.cpp
  LINK_LIBRARIES Equalizer EqualizerAdmin
  )

eq_add_tool(eqServer
  SOURCES server/eqServer.cpp
  LINK_LIBRARIES shared EqualizerServer
  )
